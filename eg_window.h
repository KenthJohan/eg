#pragma once
#include "flecs.h"


typedef struct
{
	ecs_u64_t flags;
	ecs_u64_t counter;
	ecs_bool_t should_destroy;
	ecs_bool_t grabbed;
	void * glcontext;
	void * window;
} EgWindow;

typedef struct
{
	int dummy;
} EgDraw;

typedef struct
{
	ecs_string_t value;
} EgTitle;


extern ECS_COMPONENT_DECLARE(EgWindow);
extern ECS_COMPONENT_DECLARE(EgDraw);
extern ECS_COMPONENT_DECLARE(EgTitle);


void FlecsComponentsEgWindowImport(ecs_world_t *world);


void eg_gl_make_current(EgWindow const *);
void eg_gl_create_context(EgWindow *);
void eg_gl_swap_buffer(EgWindow const *);
