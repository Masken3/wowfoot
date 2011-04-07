/*********

our colors are referred to as "RGB" triples, but are actually typically YUV

------

we palettize ("inverse colormap") using an octree lookup system

**********/

#include <crblib/inc.h>
#include <crblib/tsc.h>
#include "colorconv.h"

/*******/

typedef struct palInfo palInfo;

int closestPalInlineBGR(int B,int G,int R,palInfo *pi);
int closestPal(int R,int G,int B,palInfo *pi);
palInfo * closestPalInit(ubyte * palette);
void closestPalFree(palInfo *info);

/******/

bool palettizePlane24to8bit(ubyte *rgbPlane,ubyte *palPlane,int planeLen,ubyte *palette)
{
palInfo *palInfo;

	pushTSC();

	// rgbPlane is (planeLen*3) bytes
	// palette is 768 bytes

	palInfo = closestPalInit(palette);
	if ( ! palInfo ) return false;

	while(planeLen--) 
	{ 
		*palPlane++ = closestPalInlineBGR(rgbPlane[2],rgbPlane[1],rgbPlane[0],palInfo);
		rgbPlane += 3;
	}

	closestPalFree(palInfo);
	
	showPopTSC("palettize",stderr);

return true;
}

void paletteRGBtoYUV(ubyte *from,ubyte *to)
{
ubyte *fmpal,*topal;
int r;

	fmpal = from;
	topal = to;
	for(r=0;r<256;r++) 
	{
		RGB3_to_YUV3(fmpal,topal);
		fmpal += 3; topal += 3;
	}
}

void paletteYUVtoRGB(ubyte *from,ubyte *to)
{
ubyte *fmpal,*topal;
int r;

	fmpal = from;
	topal = to;
	for(r=0;r<256;r++) 
	{
		YUV3_to_RGB3(fmpal,topal);
		fmpal += 3; topal += 3;
	}
}
/***************************/

/***************
**
*

Build an OctTree containing all the palette entries; the RGB value
is the index into the tree, the value at the leaf is a palette index.  All null children are then set
to point to their closest neighbor.  It has a maximum depth of 8.

To find a palette entry, you take your RGB and just keep stepping in; in fact, it's quite trivial.
on an image of B bytes and a palette of P entries, this method is O(B+P)

Find the right neighbor for null children is a very difficult algorithm.  I punt and
leave them null; when we find a null in descent, we do a hash-assisted search to find
the right pal entry, then add this color & pal entry to the octree for future use.

we store palette entries in the octree as (palVal+1) so that we can use 0 to mean "not assigned"

per-pixel time : 5e-7	(found in octree lookup)
per-color time : 7e-6	(not in octree time)

total_seconds = (5e-7)*(num_pels + palettize_size) + 
	(3e-8)*(num_actual_colors - palettize_size)*(palettize_size)

	(coder=bitplane,transform=L97)

stop-rate 4 , PSNR on :
brute-force 
	pal1 ; 33.29
	pal2 : 37.69
	pal3 : 33.69
	pal4 : 44.69
OctTree without "expandNulls"	("fast")
	pal1 ; 25.73
	pal2 : 32.50
	pal3 : 27.84
	pal4 : 28.07
OctTree with brute-force "expandNulls"
	pal1 ; 32.53
	pal2 : 37.09
	pal3 : 33.27
	pal4 : 33.50
OctTree with brute-force on null
	pal1 ; 33.15
	pal2 : 37.71
	pal3 : 33.65
	pal4 : 35.05
*
**
 *****************/

#define QUANT_BITS	(4)
#define QUANT_SHIFT	(8-QUANT_BITS)
#define QUANT_ROUND	(1<<(QUANT_SHIFT-1))
#define HASH_BITS	(QUANT_BITS*3)
#define HASH_SIZE	(1<<HASH_BITS)
#define HASH(R,G,B)	( (((R)>>QUANT_SHIFT)<<(QUANT_BITS+QUANT_BITS)) + (((G)>>QUANT_SHIFT)<<(QUANT_BITS)) + (((B)>>QUANT_SHIFT)))
#define HASHROUNDED(R,G,B)	( (((R+QUANT_ROUND)>>QUANT_SHIFT)<<(QUANT_BITS+QUANT_BITS)) + (((G+QUANT_ROUND)>>QUANT_SHIFT)<<(QUANT_BITS)) + (((B+QUANT_ROUND)>>QUANT_SHIFT)))

typedef struct octNode octNode;
struct octNode 
{
	octNode * kids[8];
	octNode * parent;
};

typedef struct hashNode 
{
	struct hashNode *next;
	int R,G,B,pal;
} hashNode;

struct palInfo 
{
	ubyte *palette;
	octNode *root;
	hashNode * hash[HASH_SIZE+1];
};

// internal protos:

int colorDistance(ubyte *ca,ubyte *cb);
int findClosestPalBrute(int R,int G,int B,palInfo *pi);
void freeOctNodes(octNode *node,int bits);
void addOctNode(octNode *root,int R,int G,int B,int palVal);
void addHash(palInfo *pi,int R,int G,int B,int palVal,int hash);
void freeHash(palInfo *pi);

#define RGBbits(R,G,B,bits) (((((R)>>(bits))&1)<<2) + ((((G)>>(bits))&1)<<1) + (((B)>>((bits)))&1))

/********************/

palInfo * closestPalInit(ubyte * palette)
{
palInfo *pi;
int i;

	i = HASH_SIZE;

	if ( (pi = new(palInfo)) == NULL )
		return NULL;

	pi->palette = palette;

	pi->root = new(octNode);
	assert(pi->root);

	for(i=0;i<256;i++) 
	{
		int R,G,B;
		R = palette[3*i]; G = palette[3*i+1]; B = palette[3*i+2];
		addOctNode(pi->root,R,G,B,i);
		addHash(pi,R,G,B,i,HASH(R,G,B));
	}

return pi;
}

int findClosestPal(int R,int G,int B,palInfo *pi)
{
hashNode *node;
int hash,d,bestD,bestP;

	hash = HASHROUNDED(R,G,B);
	if ( hash > HASH_SIZE ) hash = HASH_SIZE;

	node = pi->hash[ hash ];
	if ( ! node ) 
	{
		bestP = findClosestPalBrute(R,G,B,pi);
#if 1
		// helps speed a little; depends on how common individual RGB values are
		addOctNode(pi->root,R,G,B,bestP);
#endif
#if 0
		//this could help speed, but actually makes this
		// method approximate
		node = new(hashNode);
		assert(node);
		node->next = pi->hash[hash];
		pi->hash[hash] = node;
		node->R = R;
		node->G = G;
		node->B = B;
		node->pal = bestP;
#endif
		return bestP;
	}

	bestD = 99999999;	bestP = node->pal;
	while(node) 
	{
		d = (R - node->R)*(R - node->R) + (G - node->G)*(G - node->G) + (B - node->B)*(B - node->B);
		if ( d < bestD ) 
		{
			bestD = d;
			bestP = node->pal;
		}
		node = node->next;
	}

	return bestP;
}

#define doStep(bits)	do { kid = (node)->kids[ RGBbits(R,G,B,bits) ]; \
				if ( kid ) node = kid; else return findClosestPal(R,G,B,pi); } while(0)

#define doSteps()	do { node = pi->root; doStep(7); doStep(6); doStep(5); doStep(4); doStep(3); doStep(2); doStep(1); doStep(0); } while(0)

int closestPalInlineBGR(int B,int G,int R,palInfo *pi)
{
octNode *node,*kid;

	doSteps();

return ((int)node)-1;
}

int closestPal(int R,int G,int B,palInfo *pi)
{
octNode *node,*kid;

	doSteps();

//	assertdebug( ((int)node) <= 256 && ((int)node) > 0 );

return ((int)node)-1;
}

void closestPalFree(palInfo *pi)
{

	assert(pi);
	
	freeOctNodes(pi->root,7);
	freeHash(pi);

	destroy(pi);
}


int findClosestPalBrute(int R,int G,int B,palInfo *pi)
{
int d,p;
int bestD,bestP;
ubyte * pal;
ubyte color[3];

	// now do a brute-force best-pal search to find the best pal entry

	color[0] = R;	color[1] = G;	color[2] = B;
	pal = pi->palette;
	bestD = colorDistance(color,pal);
	bestP = 0;
	for(p=1;p<256;p++)
	{
		pal += 3;
		d = colorDistance(color,pal);
		if ( d < bestD ) 
		{
			bestD = d;
			bestP = p;
		}
	}
return bestP;
}

int colorDistance(ubyte *ca,ubyte *cb)
{
int d,x;
	d = 0;
	x = ca[0] - cb[0];
	d += x*x;
	x = ca[1] - cb[1];
	d += x*x;
	x = ca[2] - cb[2];
	d += x*x;
return d;
}

void addOctNode(octNode *root,int R,int G,int B,int palVal)
{
int idx;
int bits;
octNode *node = root;

	for(bits=7;bits>0;bits--) 
	{
		idx = RGBbits(R,G,B,bits);
		if ( ! node->kids[idx] ) 
		{
			node->kids[idx] = new(octNode);
			node->kids[idx]->parent = node;
		}
		node = node->kids[idx];
	}
	idx = RGBbits(R,G,B,0);
	node->kids[idx] = (octNode *)(palVal+1);
}

void freeOctNodes(octNode *node,int bits)
{
int i;

	bits--;
	if ( ! bits ) 
	{
		destroy(node);
		return;
	}

	for(i=0;i<8;i++) 
	{
		if ( node->kids[i] )
			freeOctNodes(node->kids[i],bits);
	}

	destroy(node);
}


void addHash(palInfo *pi,int R,int G,int B,int palVal,int hash)
{
hashNode *node;
int i,h;

	for(i=0;i<8;i++) 
	{
		h = hash + (i&1) + (((i>>1)&1)<<QUANT_BITS) + ((i>>2)<<(QUANT_BITS+QUANT_BITS));
		if ( h <= HASH_SIZE ) 
		{
			node = new(hashNode);
			assert(node);
			node->next = pi->hash[h];
			pi->hash[h] = node;
			node->R = R;
			node->G = G;
			node->B = B;
			node->pal = palVal;
		}
	}
}

void freeHash(palInfo *pi)
{
int i;
hashNode *node,*next;

	for(i=0;i<=HASH_SIZE;i++) 
	{
		node = pi->hash[i];
		while(node) {
			next = node->next;
			destroy(node);
			node = next;
		}
	}
}
