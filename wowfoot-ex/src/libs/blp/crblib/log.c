#ifdef WIN32
#include <windows.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

static FILE * TeeFP = NULL; // should default to null ?

void Log_Out(const char * string)
{
#ifdef WIN32
	OutputDebugString(string);
#endif
	if ( TeeFP )
		fprintf(TeeFP,string);
}


void Log_Puts(const char * string)
{
	Log_Out(string);
	Log_Out("\n");
}

void Log_Printf(const char * String, ...)
{
	va_list			ArgPtr;
    char			TempStr[4096];

	va_start(ArgPtr, String);
    vsprintf(TempStr, String, ArgPtr);
	va_end(ArgPtr);

	Log_Out(TempStr);
}

void Log_TeeFile( FILE * FP )
{
	TeeFP = FP;
}

#ifndef NO_LOG
#pragma message("LOG on")
#endif