#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <stdio.h>


// Display error message text, given an error code.
// Typically, the parameter passed to this function is retrieved
// from GetLastError().
static void eg_win32_PrintCSBackupAPIErrorMessage(DWORD e)
{
	char buf[512];  // Buffer for text.
	DWORD n;  // Number of chars returned.
	DWORD dwFlags = FORMAT_MESSAGE_IGNORE_INSERTS;
	// Try to get the message from the system errors.
	n = FormatMessageA( FORMAT_MESSAGE_FROM_SYSTEM |FORMAT_MESSAGE_IGNORE_INSERTS,NULL,e,0,buf,sizeof(buf),NULL);
	if (0 == n)
	{
		// The error code did not exist in the system errors.
		// Try Ntdsbmsg.dll for the error code.
		// Load the library.
		HINSTANCE h = LoadLibrary("Ntdsbmsg.dll");
		if ( NULL == h )
		{
			printf("cannot load Ntdsbmsg.dll\n");
			exit(1);  // Could 'return' instead of 'exit'.
		}
		// Try getting message text from ntdsbmsg.
		n = FormatMessageA(FORMAT_MESSAGE_FROM_HMODULE |FORMAT_MESSAGE_IGNORE_INSERTS,h,e,0,buf,sizeof(buf),NULL);
		// Free the library.
		FreeLibrary( h );
	}
	// Display the error message, or generic text if not found.
	fprintf(stderr, "Error value: %ld Message: %s\n",e, n ? buf : "Error message not found." );
}


