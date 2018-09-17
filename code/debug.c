#include "debug.h"
#include <stdarg.h>
#include <stdio.h>

	
void debugLogging(int show, Boolean toFile,char *fmt, ...)
{

    if(show > 0)
    {
		va_list argptr;
		
		va_start(argptr,fmt);
		
		if(toFile)
		{
			fprintf(debugLog,fmt,argptr);
		}
		else
		{
			vprintf(fmt,argptr);
		}
		va_end(argptr);
    }
}

