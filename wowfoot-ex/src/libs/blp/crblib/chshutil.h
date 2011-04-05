#ifndef CRB_SH_UTIL_H
#define CRB_SH_UTIL_H

extern bool chsh_Quiet;

extern bool Skipped,MadeDir;

extern bool RenameError;
extern bool RenameFunc(char * Name,char *ToName);

extern bool CopyError;
extern bool CopyFuncInit(void);
extern void CopyFuncFree(void);
extern bool ShowCopyMkDirError(char *Name);
extern bool ShowCopyError(char *Name,char *ToName);
extern bool CopyFuncGen(char * FmName,char *ToName,bool FmIsDir);
extern bool CopyFunc(char * Name,char *ToName);

extern bool DeleteError;
extern bool DeleteFunc(char * Name,bool IsDir);

extern bool HaveOverwriteAnswer,StoredOverwriteAnswer,OverwriteOnlyNew;
extern bool AskOverwrite(char * Name);
extern bool AskOverwrite2(char* Target,char*Source);

extern bool CheckAndChangeAccess(char * Name);

extern void AskContinueReset(void);
extern bool AskContinue(void);

extern bool AskYN(void);

// stuff for ts/tr : 
extern void ShowFoundMatch(char *BufPtr,char *BufBase,
						bool ShowLineNum,
						bool UltraQuiet, //= false,
						bool noPrefix,
						bool vcFormat,
						char * FileName); //= false)

bool ZCopy(char *FmFileName,char *ToFileName,
			bool RecurseFlag,bool DoDirsFlag,bool RunLocalFlag,
			bool TestFlag);

bool ZDel(const char * spec,bool RecurseFlag);
			
#endif /* module */

