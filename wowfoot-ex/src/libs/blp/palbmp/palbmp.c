#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include <crblib/inc.h>
#include <crblib/bmpio.h>
#include <crblib/fileutil.h>
#include <crblib/strutil.h>
#include <crblib/imutil.h>
#include "palcreate.h"
#include "palettize.h"
#include "paloptimize.h"
#include "colorconv.h"

int tune_param;

#ifdef _MSC_VER
#pragma warning( disable : 4018 )	// signed/unsigned mismatch
#endif

#ifdef assert
#undef assert
#endif

#ifdef _DEBUG
#define assert(exp) if ( exp ) ; else { errputs("assertion failed: " #exp); __asm { int 03 }; }
#else
#define assert(exp) if ( exp ) ; else errexit("assertion failed: " #exp);
#endif // _DEBUG

char * getParam(char *arg,char *argv[],int *iPtr);

int main(int argc,char *argv[])
{
ubyte *rawPlane= NULL;
myBMP *mybm= NULL;
char *sourceName,*targetName;
int i,rawPlaneLen,bmType;
int palNColors;
bool palOptimize;

	errputs("palBMP (c) 1998 by Charles Bloom");

	if ( argc < 3 ) {
		errputs("palBMP : palettize a bmp");
		errputs("usage: palBMP <source> <target> <colors>");
		exit(0);
	}

	sourceName = argv[1];
	targetName = argv[2];

	palNColors = atol(argv[3]);
	assert( palNColors > 0 && palNColors <= 256 );

	// defaults:
	palOptimize = false;

	for(i=4;i<argc;i++)
	{
		char *arg,flag;

		arg = getParam(argv[i],argv,&i);

		flag = toupper(*arg);
		arg++;

		switch(flag)
		{
			case 'O':
				palOptimize = true;
				errputs("got options : optimize");
				break;
			case 'T':
				tune_param = atol( getParam(arg,argv,&i) );
				fprintf(stderr,"got option : tune_param = %d\n",tune_param);
				break;
			default:
				fprintf(stderr,"unknown option ignored : %c %s\n",flag,arg);
				break;
		}
	}

	if ( ! (mybm = loadImage(sourceName)) )
		errexit("readImage failed");

	rawPlaneLen = mybm->dataBytes;
	if ( mybm->type != MBTYPE_24BIT_RGB )
		rawPlaneLen *= 3;

	if ( ! (rawPlane = malloc(rawPlaneLen) ) )
		errexit("malloc failed");

	switch(mybm->type) {
		case MBTYPE_8BIT_PAL:	bmType = PT_8B_PAL; break;
		case MBTYPE_8BIT_GRAY:	bmType = PT_8B_GRAY; break;
		case MBTYPE_24BIT_RGB:	bmType = PT_24B_RGB; break;
		default:
			errexit("only does 1 or 3 planes");
			break;
	}

	convertPlanes( mybm->data, bmType, rawPlane, PT_24B_YUV,
					mybm->dataBytes, mybm->palette );

	if (1) 
	{
		ubyte *palPlane;
		ubyte palette[768];
		myBMP palbm;
		int palLen;

		palLen = (mybm->width) * (mybm->height);

		palPlane = malloc(palLen);
		assert(palPlane);	

		createPaletteSlow( rawPlane, palLen, palette, palNColors );
		for(i=palNColors;i<256;i++)
			palette[3*i+0] = palette[3*i+1] = palette[3*i+2] = 0;

		if ( palOptimize )
			palettizePlane24Optimize( rawPlane, palLen,palette,palNColors,0);

		convertPlanes( rawPlane, PT_24B_RGB, palPlane, PT_8B_PAL,
						mybm->dataBytes, palette );

		paletteYUVtoRGB(palette,palette);

		palbm.height = mybm->height;
		palbm.width = mybm->width;
		palbm.type = MBTYPE_8BIT_PAL;
		palbm.dataBytes = palLen;
		palbm.data = palPlane;
		palbm.palette = palette;

		errputs("writing palettized bmp");

		if ( ! saveBMP(&palbm,targetName) )
			errexit("writeImage failed");

		if ( 1 )
		{
		ubyte * raw;
			raw = malloc(rawPlaneLen);
			assert(raw);
			convertPlanes( palPlane, PT_8B_PAL, raw, PT_24B_YUV, palLen, palette );
			ubytesCompare(rawPlane,raw,rawPlaneLen,stdout);
			free(raw);
		}


		free(palPlane);
	}

	free(rawPlane);

return 0;
}

