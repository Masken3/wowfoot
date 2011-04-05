
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <sys/stat.h>
#include <errno.h>

#include <crblib/inc.h>
#include <crblib/fileutil.h>
#include <crblib/chshutil.h>

#include <io.h>

bool chsh_Quiet=false;

bool Skipped=0,MadeDir=0;

bool CheckAndChangeAccess(char * Name);
bool AskContinue(void);
bool AskOverwrite2(char* Target,char*Source);

//-----------------------------------------------------

/***************** delete************/

bool DeleteError=0;
bool DeleteFuncQuiet = false;

bool DeleteFunc(char * Name,bool IsDir)
{
int err;

	Skipped=0;

	if ( ! CheckAndChangeAccess(Name))
	{
		DeleteError =1;
		return( AskContinue());
	}

	/*
	{
	char curdir[1024];
	getcwd(curdir,1024);
	err = chdir("c:\\");
	perror("chdir");
	}
	*/

	if ( IsDir ) err = rmdir(Name);
	else err = remove(Name);

	if ( err )
	{
		DeleteError =1;
		fprintf(stderr," %s: ",Name); fflush(stderr);
		if( IsDir) perror("rmdir");
		else perror("remove");
		return( AskContinue());
	}

	if ( ! DeleteFuncQuiet )
		printf("	%s\n",Name);

return(1);
}

/************ copy ***************/

bool CopyFunc(char * Name,char *ToName);

static bool CopyBufUsed =0; /*semaphore */
static ubyte * CopyBuf = NULL;
static const size_t CopyBufSize =(1<<17);

bool CopyError=0;

void CopyFuncFree(void)
{
if ( CopyBufUsed ) return;
if ( CopyBuf ) { free(CopyBuf);CopyBuf =NULL; }
}

bool CopyFuncInit(void)
{
if ( CopyBuf ) return(1);
if ( (CopyBuf= malloc(CopyBufSize)) ==NULL )
	return(0);
return(1);
}


bool ShowCopyMkDirError(char *Name)
{
CopyError= 1;
fprintf(stderr," %s",Name); fflush(stderr);
perror("MakeDirerror");

return( AskContinue());
}

bool ShowCopyError(char*Name,char *ToName)
{
CopyError= 1;
fprintf(stderr," %s-> %s",Name,ToName); fflush(stderr);
perror("Copy error");

return(AskContinue() );
}

bool CopyFuncGen(char* FmName,char*ToName,bool FmIsDir)
{
Skipped=0;MadeDir=0;

if ( ! FileExists(FmName))
	{
	errputs("source file not found");
	Skipped=1;
	return(1);
	}

if ( FileExists(ToName))
	{
	if ( FmIsDir )
		{
		if ( NameIsDir(ToName) ) /*both dirs, fine*/
			{
			Skipped=1;
			return true;
			}
		else
			{
			errputs("Trying to copy a directory onto a file.");
			}
		}
	if (! CheckAndChangeAccess(ToName) )
		{
		bool ak = AskContinue();
		if ( ! ak )
			CopyError = true;
		return ak;
		}
	if ( ! AskOverwrite2(ToName,FmName))
		{
		Skipped=1;
		return true;
		}
	}

if ( FmIsDir )
	{
	if ( mkdir(ToName) ==0 )
		{
		MadeDir =1;
		return true;
		}
	else
		{
		return(ShowCopyMkDirError(ToName));
		}
	}
else
	{
	return( CopyFunc(FmName,ToName));
	}
}

bool CopyFunc(char * Name,char *ToName)
{
FILE *FmFH,*ToFH;
ulong FmLen,CurLen;

Skipped=0;

if ( ! CopyBuf )
	{
	if(! CopyFuncInit()) return(0);
	}

if ( ( FmFH= fopen(Name,"rb") ) ==NULL )
	return(ShowCopyError(Name,ToName));

if ( (FmLen= FileLengthofFH(FmFH))== FileLengthofFH_Error)
	{
	fclose(FmFH);
	return(ShowCopyError(Name,ToName));
	}

if ( ( ToFH= fopen(ToName,"wb") ) ==NULL )
	{
	fclose(FmFH);
	return(ShowCopyError(Name,ToName));
	}

if(CopyBufUsed) return(0); /*wait onsemaphore */

CopyBufUsed=1;

while( FmLen > 0 )
	{
	CurLen = min(CopyBufSize,FmLen);

	if ( FRead(FmFH,CopyBuf,CurLen)!= CurLen)
		{
		fclose(FmFH); fclose(ToFH);
		CopyBufUsed=0;
		return(ShowCopyError(Name,ToName));
		}

	if ( FWrite(ToFH,CopyBuf,CurLen) !=CurLen )
		{
		fclose(FmFH); fclose(ToFH);
		CopyBufUsed=0;
		return(ShowCopyError(Name,ToName));
		}

	FmLen -= CurLen;
	}

CopyBufUsed=0;

fclose(FmFH); fclose(ToFH);

copystat(ToName,Name);

return(1);
}

/********************* rename****************/

bool RenameError=0;

bool RenameFunc(char * FmName,char *ToName)
{
Skipped=0;

if ( ! FileExists(FmName))
	{
	errputs("source filenot found");
	Skipped=1;
	return(1);
	}

if ( FileExists(ToName))
	{
	if (! CheckAndChangeAccess(ToName) )
		{
		return(AskContinue());
		}
	if ( ! AskOverwrite2(ToName,FmName))
		{
		Skipped =1;
		return(1);
		}	
	}

if ( rename(FmName,ToName) !=0 )
	{
	//if (strcmp( DrivePart(FmName),DrivePart(ToName)) != 0 )
		if ( toupper(FmName[0]) != toupper(ToName[0]) )
		{
			bool fmIsDir = NameIsDir(FmName);

			if ( fmIsDir )
			{
				/**
				
				
				fails:

				t:\dl
				mov -d peter* "i:\mp3\Peter Bjorn and John"

				the problem is the parens are being treated as wild chars

				1. do a function that sticks a ` on wild chars to make them literals
				2. lots of the zcopy junk incorrectly uses FileExists() and shit like that on pat strs
					which will fail with literals chars


				
				**/
			
				//perror("rename");
				//errputs("ZCopy:");
				
				if ( ! ZCopy(FmName,ToName,true,true,false,false) )
				 {RenameError =1; return(0);}

				if ( CopyError )
				{
					 RenameError =1; return(0);
				}

				fprintf(stderr,"about to ZDel : %s-> %s ",FmName,ToName); //fflush(stderr);
				if ( ! AskYN() )
					return 0;
					
				//errputs("ZDel:");
				
				if ( ! ZDel(FmName,true) )
				 {RenameError =1; return(0);}
			
				return 1;
			}
			else
			{
				if ( ! CopyFuncGen(FmName,ToName,fmIsDir)) {RenameError =1; return(0);}
				if ( ! DeleteFunc(FmName,fmIsDir)) {RenameError =1; return(0);}
				return(1);
			}
		}

	RenameError = 1;
	fprintf(stderr," %s-> %s ",FmName,ToName); fflush(stderr);
	perror("rename");
	return( AskContinue());
	}
else
{
	return true;
}

}

/********************* overwrite ****************/

bool HaveOverwriteAnswer=FALSE,StoredOverwriteAnswer=FALSE,OverwriteOnlyNew=FALSE;

bool AskOverwrite2(char* Target,char*Source)
{
bool DoOverWrite,DoOverWriteNew;
bool srcDir,targetDir;

	DoOverWrite = DoOverWriteNew = false;

	srcDir = NameIsDir(Source);
	targetDir = NameIsDir(Target);

	if ( srcDir != targetDir )
	{
		if ( srcDir )
			fprintf(stderr,"Trying to copy a dir onto a file %s -> %s\n",Source,Target);
		else
			fprintf(stderr,"Trying to copy a file onto a dir %s -> %s\n",Source,Target);
	
		if ( ! AskContinue() )
			return false;
	}
	
	if (HaveOverwriteAnswer ){
		DoOverWrite =StoredOverwriteAnswer;
		DoOverWriteNew = OverwriteOnlyNew;
	} else {
		int c;
	
		fprintf(stderr," %s exists; overwrite? (y/n/A/N/u/U)",Target); fflush(stderr);
	
		for(;;) {
			c = getc(stdin); if ( c != '\n') while( getc(stdin)!= '\n');

			if ( c =='n' ) {
				DoOverWrite = false;
				break;
			} else if( c== 'N' ) {
				HaveOverwriteAnswer= true;
				StoredOverwriteAnswer = DoOverWrite = false;
				OverwriteOnlyNew = false;
				break;	
			} else if ( c =='A' ) {
				HaveOverwriteAnswer= true;
				StoredOverwriteAnswer = DoOverWrite = true;
				OverwriteOnlyNew = false;
				break;	
			} else if( c== 'y' ) {
				DoOverWrite = true;
				break;
			} else if ( c == 'u' ) {
				DoOverWriteNew = true;
				break;
			} else if ( c == 'U' ) {
				HaveOverwriteAnswer= true;
				OverwriteOnlyNew = DoOverWriteNew = true;
				break;
			} else {
				fprintf(stderr," (y=yes, n=no, A=all,N=none,u=update newer,U=all)");
				fflush(stderr);
			}
		}
	}

	if ( srcDir && targetDir )
	{
		return DoOverWrite;
	}

	if ( DoOverWriteNew ) {
		struct stat SourceStat,TargetStat;	

		if ( stat(Source,&SourceStat) != 0 ){
			fprintf(stderr,"<%s> ",Source); fflush(stderr);
			perror("stat");
			return false;
		}
		if ( stat(Target,&TargetStat) != 0 ){
			fprintf(stderr,"<%s> ",Target); fflush(stderr);
			perror("stat");
			return false;
		}

		if ( SourceStat.st_mtime > TargetStat.st_mtime ) {
			// printf("%s older than %s : overwriting\n",Target,Source);
			DoOverWrite = true;
		} else {
			// printf("%s newer than %s : skipping\n",Target,Source);
			DoOverWrite = false;
		}

	}

	if ( DoOverWrite ) {
		if ( remove(Target) != 0 ) {
			fprintf(stderr,"<%s> ",Target); fflush(stderr);
			perror("remove");
			return false;
		}
	}

return DoOverWrite;
}

bool AskOverwrite(char * Name)
{
if ( HaveOverwriteAnswer ) {
	if( StoredOverwriteAnswer) {
		if ( remove(Name)!= 0 ) {
			fprintf(stderr,"< %s> ",Name); fflush(stderr);
			perror("remove");
			return(0);
		}
		return(1);
	} else{
		return(0);
	}
} else {
	int c;
	
	fprintf(stderr," %sexists; overwrite? (y/n/A/N)",Name); fflush(stderr);
	
	for(;;)
		{
		c =getc(stdin); if( c!= '\n') while( getc(stdin)!= '\n');

		if ( c =='n' )return(0);
		else if( c== 'N' )
			{ HaveOverwriteAnswer= 1; StoredOverwriteAnswer = 0;return(0); }
		else if( c== 'y' ||c == 'A' )
			{
			if ( c =='A' )
				{ HaveOverwriteAnswer= TRUE;StoredOverwriteAnswer =TRUE; }
			if ( remove(Name)!= 0 )
				{
				fprintf(stderr,"< %s > ",Name);fflush(stderr);
				perror("remove");
				return(0);
				}
			return(1);
			}
		else
			{
			fprintf(stderr," (y=yes, n=no, A=all,N=none)");	fflush(stderr);
			}
		}
	}

}

bool AskYN()
{
	int c;
	
	//fprintf(stderr," %sexists; overwrite? (y/n/A/N)",Name); fflush(stderr);
	fprintf(stderr,"(y/n)?"); fflush(stderr);
	
	for(;;)
	{
		c =getc(stdin); if( c!= '\n') while( getc(stdin)!= '\n');

		if ( c == 'n' || c == 'N' ) return false;
		else if( c== 'y' ||c == 'Y' ) return true;
		else
		{
			fprintf(stderr," (y=yes, n=no)");	fflush(stderr);
		}
	}

}

/******************* access**********************/

static bool ChangeAccess_All = FALSE;

bool CheckAndChangeAccess(char * Name)
{

#ifdef __WATCOMC__ //{
if ( access(Name,W_OK) == 0 )
#else
#ifdef _MSC_VER
if ( access(Name,2) == 0 ) // @@ !?
#else
	intentional syntax error
#endif
#endif //}
	{
	return(1);
	}
else if( errno == EACCES)
	{
	if (! ChangeAccess_All )
		{
		int c;
		fprintf(stderr,"File %snot availablefor write\n",Name);
	Access_TryAgain:
		fprintf(stderr,"(C)hange,(I)gnore, or change(A)ll?");
		fflush(stderr);
		c =getc(stdin); c = toupper(c);
		if ( c !='\n' ) while(getc(stdin) != '\n') ;
		if ( c =='I' )return(0);
		else if( c== 'A' )
			{	ChangeAccess_All= TRUE;}	
		else if( c== 'C' ) { }
		else goto Access_TryAgain;
		}

	if (chmod(Name,S_IREAD|S_IWRITE|S_IEXEC) == 0 )
		return(1);

	perror("chmod failed");
	return(0);
	}

perror("access failed");
return(0);
}

/****************************continue ********************/

static bool AskContinue_All = false;

void AskContinueReset(void)
{
	AskContinue_All = false;
}

bool AskContinue(void)
{
if ( AskContinue_All ) return true;
else
	{
	int c;
	fprintf(stderr,"(Q)uit, keep(G)oing, orignore (A)ll errors?");
	fflush(stderr);
	c = getc(stdin);
	if( c!= '\n') getc(stdin);
	if( c== 'g' ||c == 'G' ) return true;
	if( c== 'a' ||c == 'A' )
		{
		AskContinue_All= TRUE;
		return true;
		}
	}
return false;
}


/****************************ts/tr stuff ********************/

/** this isn't so fast, cuz it has to go back and count \n to
		show the line number **/

void ShowFoundMatch(char *BufPtr,char *BufBase,
						bool ShowLineNum,
						bool UltraQuiet, //= false,
						bool noPrefix,
						bool vcFormat,
						char * FileName) //= false)
{
	char FoundLine[4096];
	char *FLP,*FoundBufPtr,*EOLs;
	int LineNumber=0;

	if ( vcFormat )
		ShowLineNum = 1;

	FoundBufPtr = BufPtr;

	while(*BufPtr != '\n' && BufPtr >= BufBase) BufPtr--;
	BufPtr++;

	if ( (FoundBufPtr - BufPtr) > 4000 )
		BufPtr = FoundBufPtr - 4000;

	strncpy(FoundLine,BufPtr,4095); FoundLine[4095] = 0;

	FLP = FoundLine;

	if ( noPrefix ) FLP += (FoundBufPtr - BufPtr);

	if ( ShowLineNum )
	{
		char *BufSeek = BufBase;
		LineNumber = 1;
		while ( BufSeek < BufPtr ) if ( *BufSeek++ == '\n' ) LineNumber++;
	}

	if ( strchr(FoundLine,'\n') )
		*(strchr(FoundLine,'\n')) = 0;
	if ( strchr(FoundLine,'\r') )
		*(strchr(FoundLine,'\r')) = 0;
	while ( strchr(FoundLine,'\t') )
		*(strchr(FoundLine,'\t')) = ' ';

	while( *FLP == ' ' ) FLP++;

	EOLs = FLP + strlen(FLP) - 1;
	while(*EOLs==' ') EOLs--;
	EOLs++; *EOLs = 0;

	if ( vcFormat )
	{
		printf("%s(%d) : %s\n",FileName,LineNumber,FLP);
	}
	else if ( UltraQuiet ) 
	{
		printf("%s\n",FLP);
	}
	else 
	{
		if ( ShowLineNum )
		{
			printf("%5d:%s\n",LineNumber,FLP);
		}
		else
		{
			printf("%08X:%s\n",(FoundBufPtr-BufBase),FLP);
		}
	}
}


/*********************************************************************************************/

#include <crblib/matchpat.h>
#include <crblib/walker.h>

int ZCopy_NumSpaces=0; 
bool ZCopy_ShowDirNames=false;

pattern ZCopy_FmPat = NULL;
pattern ZCopy_ToPat = NULL;
char * ZCopy_StartPath = NULL;

bool ZCopy_DirStartFunc(char * FullPath,int NestLevel)
{
int i = NestLevel<<1;
ZCopy_NumSpaces = i + 3;

if ( ZCopy_ShowDirNames )
	{
	puts("");
	while(i--) putc(' ',stdout);
	printf("%-s\n",FullPath);
	}

return true;
}

bool ZCopy_DirDoneFunc(char * FullPath,int NestLevel)
{
return true;
}

bool ZCopy_FileFunc(struct WalkInfo * WI)
{
if ( MatchPatternNoCase(WI->FullName,ZCopy_FmPat) )
	{
	bool ret;
	char *FmName=WI->Name;
	char ToName[1024];
	//char *CurPath = NULL;
	char CurPath[1024];

	if ( ! RenameByPat(ZCopy_FmPat,WI->FullName,ZCopy_ToPat,ToName) )
		{
		errputs("Unsupported RenameByPat");
		return(0);
		}

	if ( ZCopy_StartPath )
		{
		//CurPath = malloc(1024);
		//if ( !CurPath ) return 0;
		if ( !getcwd(CurPath,1024) ) return 0;
		chdir(ZCopy_StartPath);
		}

	PrefixCurDir(ToName);
	ret = CopyFuncGen(WI->FullName,ToName,WI->IsDir);

	if ( ZCopy_StartPath )
		{
		chdir(CurPath);
		//free(CurPath);
		}

	if ( ret && ! chsh_Quiet )
		{
		int i = ZCopy_NumSpaces; while(i--) putc(' ',stdout);
		if ( Skipped )
			{
			printf("%s skipped\n",FmName);
			}
		else if ( MadeDir )
			{
			printf("%s -> %s (dir created)\n",FmName,ToName);
			}
		else
			{
			printf("%s -> %s\n",FmName,ToName);
			}
		}
	return(ret);
	}

return(1);
}

bool ZCopy(char *FmFileName,char *ToFileName,
			bool RecurseFlag,bool DoDirsFlag,bool RunLocalFlag,
			bool TestFlag)			
{
char FmPatStr[1024],ToPatStr[1024];

ZCopy_FmPat = NULL;
ZCopy_ToPat = NULL;
ZCopy_StartPath = NULL;
CopyError = false; // @@ ??

if ( !CopyFuncInit() )
	{
	errputs("Malloc CopyBuffer failed!");
	return false;
	}

strcpy(FmPatStr,FmFileName);
strcpy(ToPatStr,ToFileName);
PrefixCurDir(FmPatStr);

if ( TestFlag ) printf("zcopy : %s -> %s\n",FmPatStr,ToPatStr);

if ( NameIsDir(FmPatStr) )
	{
	RecurseFlag = 1;

#if 1
	// this makes it so that if you do "zc r:\dir i:\" it makes "i:\dir"
	if ( NameIsDir(ToPatStr) && ToFileName[strlen(ToFileName)-1] == PathDelim )
		{
		//CutEndPath(FmPatStr);
		CatPaths(ToPatStr,FilePart(FmPatStr));
		}
#endif

	CatPaths(FmPatStr,"*");
	}

if ( TestFlag ) printf("dir correction : %s -> %s\n",FmPatStr,ToPatStr);

if ( IsWild(FmPatStr) && !IsWild(ToPatStr) && !NameIsDir(ToPatStr) )
	{
	if ( TestFlag ) printf("making dir : %s\n",ToPatStr);

		/** ToPatStr may be a path that doesn't exist.
				 create if so **/
	if ( mkdir(ToPatStr) != 0 )
	{
		if ( ! ShowCopyMkDirError(ToPatStr) )
			return false;
	}
		
	}

if ( NameIsDir(ToPatStr) ) CatPaths(ToPatStr,FilePart(FmPatStr));

if ( TestFlag ) printf("file correction : %s -> %s\n",FmPatStr,ToPatStr);

if ( IsWild(FmPatStr) )
	{
	ZCopy_FmPat = chshMakePattern(FmPatStr);
	ZCopy_ToPat = chshMakePattern(ToPatStr);
	if ( !ZCopy_FmPat || !ZCopy_ToPat )
		{
		errputs("MakePattern failed!");
		if ( ZCopy_FmPat ) FreePattern(ZCopy_FmPat);
		if ( ZCopy_ToPat ) FreePattern(ZCopy_ToPat);
		}
	else
		{
			char FmPatPath[1024];
			PathPartInsert(FmPatStr,FmPatPath);

		if ( ! RunLocalFlag )
			{
			if ( (ZCopy_StartPath = malloc(1024)) == NULL ) return false;
			if ( !getcwd(ZCopy_StartPath,1024) ) return false;
			}

		if ( RecurseFlag )
			{
			ZCopy_ShowDirNames = 1;

			if ( ! WalkDir_TwoPass(FmPatPath,RecurseFlag,
						(DoDirsFlag||RecurseFlag),
						ZCopy_FileFunc,ZCopy_DirStartFunc,ZCopy_DirDoneFunc) )
				{
				errputs("Error traversing directories!");
				CopyError = true;
				}
			}
		else
			{
			if ( ! WalkDir(FmPatPath,RecurseFlag,DoDirsFlag,
						ZCopy_FileFunc,ZCopy_DirStartFunc,ZCopy_DirDoneFunc) )
				{
				errputs("Error traversing directories!");
				CopyError = true;
				}
			}
		FreePattern(ZCopy_FmPat);
		FreePattern(ZCopy_ToPat);
		smartfree(ZCopy_StartPath);
		}
	}
else
	{
	if ( ! CopyFuncGen(FmPatStr,ToPatStr,0) )
		return false;
	}

CopyFuncFree();

if ( CopyError )
	return false;

return true;
}

/*****************************************************************************************/

typedef struct ZDel_FileNodeStruct ZDel_FileNode;
struct ZDel_FileNodeStruct
  {
  ZDel_FileNode * Next;
  bool IsDir;
  char FullName[1024];
  };
ZDel_FileNode * ZDel_ListHead = NULL;

pattern ZDel_FullPatStr = 0;

bool ZDel_DirDoneFunc(char * FullPath,int NestLevel)
{
ZDel_FileNode *CurNode,*NextNode;
bool KeepGoing = 1;

CurNode = ZDel_ListHead; ZDel_ListHead = NULL;
while(CurNode)
  {
  if ( KeepGoing )
    {
    KeepGoing = DeleteFunc(CurNode->FullName,CurNode->IsDir);
    }
  NextNode = CurNode->Next;
  free(CurNode);
  CurNode = NextNode;
  }
return(KeepGoing);
}

bool ZDel_FileFunc(struct WalkInfo * WI)
{
char FullName[1024];
strcpy(FullName,WI->Path);
strcat(FullName,WI->Name);
if ( MatchPatternNoCase(FullName,ZDel_FullPatStr) )
  {
  ZDel_FileNode * NewNode;
  if ( (NewNode = malloc(sizeof(ZDel_FileNode))) )
    {
    strcpy(NewNode->FullName,FullName);
    NewNode->IsDir = WI->IsDir;
    NewNode->Next = ZDel_ListHead;
   ZDel_ListHead = NewNode;
    }
  }
return(1);
}

bool ZDel(const char * spec,bool RecurseFlag)
{
	bool PatWasDir;
	char FullStr[1024];

	ZDel_ListHead = NULL;
	ZDel_FullPatStr = 0;

	strcpy(FullStr,spec);
	PrefixCurDir(FullStr);
	PatWasDir = NameIsDir(FullStr);
	
	if ( PatWasDir )
		CatPaths(FullStr,"*");

	if ( IsWild(FullStr) )
	{
		if ( (ZDel_FullPatStr = chshMakePattern(FullStr) ) == NULL )
		{
			errputs("MakePattern error");
		}
		else
		{
			char FullStrPath[1024];
			PathPartInsert(FullStr,FullStrPath);
			
			if ( ! WalkDir_DeepFirst(FullStrPath,RecurseFlag||PatWasDir,
									RecurseFlag||PatWasDir,ZDel_FileFunc,NULL,ZDel_DirDoneFunc) )
			{
    			errputs("Error traversing directories!");
			}

			FreePattern(ZDel_FullPatStr);

			if ( PatWasDir )
			{
				*(FilePart(FullStr)) = 0; /* cut the '*' */
				CutEndPath(FullStr);
				if ( ! DeleteFunc(FullStr,1) ) /* is dir */
					return false;
			}
		}
	}
	else
	{
		if ( ! DeleteFunc(FullStr,0) ) /* not dir */
			return false;
	}
	
	return true;
}