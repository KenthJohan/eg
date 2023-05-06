#include "EgLogs.h"
#include <stdio.h>

ECS_DECLARE(EgLogsVerbose);
ECS_DECLARE(EgLogsInfo);
ECS_DECLARE(EgLogsWarning);
ECS_DECLARE(EgLogsError);

ECS_DECLARE(EgEventsIdling);
ECS_DECLARE(EgEventsPrint);


void colhex_to_ansi24(char const colhex[7], char ansi24[20])
{
	char t[128] = {0};
	t['0'] = 0;
	t['1'] = 1;
	t['2'] = 2;
	t['3'] = 3;
	t['4'] = 4;
	t['5'] = 5;
	t['6'] = 6;
	t['7'] = 7;
	t['8'] = 8;
	t['9'] = 9;
	t['A'] = 10;
	t['B'] = 11;
	t['C'] = 12;
	t['D'] = 13;
	t['E'] = 14;
	t['F'] = 15;
	t['a'] = 10;
	t['b'] = 11;
	t['c'] = 12;
	t['d'] = 13;
	t['e'] = 14;
	t['f'] = 15;
	int r = 10*t[(int)colhex[1]] + t[(int)colhex[2]];
	int g = 10*t[(int)colhex[3]] + t[(int)colhex[4]];
	int b = 10*t[(int)colhex[5]] + t[(int)colhex[6]];
	ansi24[0] = '\033';
	ansi24[1] = '[';
	ansi24[2] = '3';
	ansi24[3] = '8';
	ansi24[4] = ';';
	ansi24[5] = '2';
	ansi24[6] = ';';
	ansi24[9] = '0'+(r % 10); r /= 10;
	ansi24[8] = '0'+(r % 10); r /= 10;
	ansi24[7] = '0'+(r % 10); r /= 10;
	ansi24[10] = ';';
	ansi24[13] = '0'+(g % 10); g /= 10;;
	ansi24[12] = '0'+(g % 10); g /= 10;;
	ansi24[11] = '0'+(g % 10); g /= 10;
	ansi24[14] = ';';
	ansi24[17] = '0'+(b % 10); b /= 10;
	ansi24[16] = '0'+(b % 10); b /= 10;
	ansi24[15] = '0'+(b % 10); b /= 10;
	ansi24[18] = 'm';
	ansi24[19] = '\0';
}


void eg_event_submit(ecs_world_t * world, ecs_entity_t channel, void * data, size_t size)
{
	if (ecs_has(world, channel, EgEventsPrint))
	{
		char const * cstring = data;
		fputs(cstring, stdout);
	}
	if (ecs_has(world, channel, EgEventsIdling))
	{
		while(1)
		{
			ecs_progress(world, 0);
			ecs_os_sleep(0, 1000000);
		}
	}
}



void eg_event_strf(ecs_world_t * world, ecs_entity_t channel, const char *file, int32_t line, const char *fmt, ...)
{
	//char const * name = ecs_get_name(world, channel);
	char const * name = ecs_get_fullpath(world, channel);
	char buf[1024];
	char * p = buf;
	va_list args;
	va_start(args, fmt);
	//p += snprintf(p, 512, ECS_YELLOW "EG %s:%i " ECS_NORMAL, file, line);
	char const * color = ecs_doc_get_color(world, channel);
	if (color)
	{
		char acolor[20];
		colhex_to_ansi24(color, acolor);
		p += snprintf(p, 512, "%s%s " ECS_NORMAL, acolor, name);
	}
	p += vsnprintf(p, 512, fmt, args);
	eg_event_submit(world, channel, buf, 0);
	va_end(args);
}




void EgLogsImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgLogs);
	ecs_set_name_prefix(world, "Eg");

	ECS_TAG_DEFINE(world, EgLogsVerbose);
	ECS_TAG_DEFINE(world, EgLogsInfo);
	ECS_TAG_DEFINE(world, EgLogsWarning);
	ECS_TAG_DEFINE(world, EgLogsError);


	ECS_TAG_DEFINE(world, EgEventsIdling);
	ECS_TAG_DEFINE(world, EgEventsPrint);

	ecs_add(world, EgLogsError, EgEventsIdling);

	ecs_add(world, EgLogsError,    EgEventsPrint);
	ecs_add(world, EgLogsVerbose,  EgEventsPrint);
	ecs_add(world, EgLogsInfo,     EgEventsPrint);
	ecs_add(world, EgLogsWarning,  EgEventsPrint);

	ecs_doc_set_color(world, EgLogsError,   EG_LOGS_COLOR_ERROR);
	ecs_doc_set_color(world, EgLogsVerbose, EG_LOGS_COLOR_VERBOSE);
	ecs_doc_set_color(world, EgLogsInfo,    EG_LOGS_COLOR_INFO);
	ecs_doc_set_color(world, EgLogsWarning, EG_LOGS_COLOR_WARNING);


}




