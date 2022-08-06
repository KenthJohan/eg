#include "eg_basics.h"
#include <stdint.h>
#include <stdio.h>
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




void eg_basics_enable_ternminal_color()
{
#ifdef _WIN32
	void* hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	uint32_t dwMode = 0;
	GetConsoleMode(hOut, &dwMode);
	dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	SetConsoleMode(hOut, dwMode);
#endif
}

void eg_soft_assert(char const * file, int line, char const * e)
{
	fprintf(stderr, "%s:%i: %s\n", file, line, e);
}

void eg_iter_info(ecs_iter_t *it, const char *fname, const char *file, int32_t line)
{
	ecs_world_t * world = it->world;
	for (int i = 0; i < it->count; i ++)
	{
		ecs_entity_t e = it->entities[i];
		if (ecs_is_valid(world, e))
		{
			char const * name = ecs_get_name(world, e);
			ecs_type_t const * type = ecs_get_type(world, e);
			char const * typestr = ecs_type_str(world, type);
			fprintf(stdout,
			ECS_YELLOW "EG %s:%i:" ECS_CYAN "%s()"ECS_NORMAL": 0x%08llx: name=%s, type=(%s)\n",
			file, line, fname, e, name, typestr
			);
		}
		else
		{
			fprintf(stdout,
			ECS_YELLOW "EG %s:%i:" ECS_CYAN "%s()"ECS_NORMAL": 0x%08llx: invalid\n",
			file, line, fname, e);
		}

	}
}

