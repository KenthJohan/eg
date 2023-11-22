#include "eg_term.h"
#include <stdint.h>

#ifdef _WIN32
//#include <windows.h>
#define STD_OUTPUT_HANDLE ((uint32_t)-11)
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x4
//WINBASEAPI HANDLE WINAPI GetStdHandle (DWORD nStdHandle);
//WINBASEAPI WINBOOL WINAPI GetConsoleMode(HANDLE hConsoleHandle,LPDWORD lpMode);
//WINBASEAPI WINBOOL WINAPI SetConsoleMode(HANDLE hConsoleHandle,DWORD dwMode);
__declspec (dllimport) void* __stdcall GetStdHandle(uint32_t nStdHandle);
__declspec (dllimport) int __stdcall GetConsoleMode(void * hConsoleHandle, uint32_t * dwMode);
__declspec (dllimport) int __stdcall SetConsoleMode(void * hConsoleHandle, uint32_t dwMode);
#endif


void eg_term_enable_color()
{
#ifdef _WIN32
	void* hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	uint32_t dwMode = 0;
	GetConsoleMode(hOut, &dwMode);
	dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	SetConsoleMode(hOut, dwMode);
#endif
}