#include <stdio.h>
#include <stdlib.h>

#include "flecs.h"
#include "eg_sdl.h"
#include "eg_geometry.h"
#include "eg_window.h"
#include "eg_userinput.h"
#include "eg_base.h"


typedef struct
{
	int dummy;
} EgPlayer;

typedef struct
{
	int dummy;
} EgEnemy;


ECS_COMPONENT_DECLARE(EgPlayer);
ECS_COMPONENT_DECLARE(EgEnemy);



static void Move_Enemy(ecs_iter_t *it)
{
	EgEnemy *e = ecs_term(it, EgEnemy, 1);
	EgPosition2F32 *p = ecs_term(it, EgPosition2F32, 2);
	for (int i = 0; i < it->count; i ++)
	{
		e[i].dummy = 0;
		p[i].x += 0.1f;
		p[i].y += 0.1f;
	}
}


static void Move_Player(ecs_iter_t *it)
{
	EgPlayer *p = ecs_term(it, EgPlayer, 1);
	EgUserinput *u = ecs_term(it, EgUserinput, 2); // Singleton
	EgPosition2F32 *r = ecs_term(it, EgPosition2F32, 3);
	ecs_u64_t * keyboard = u->keyboard;
	for (int i = 0; i < it->count; i ++)
	{
		p[i].dummy = 0;
		float dx = EG_U64BITSET_GET(keyboard, EG_KEY_RIGHT) - EG_U64BITSET_GET(keyboard, EG_KEY_LEFT);
		float dy = EG_U64BITSET_GET(keyboard, EG_KEY_UP) - EG_U64BITSET_GET(keyboard, EG_KEY_DOWN);
		//EG_TRACE("%f %f", dx, dy);
		r[i].x += 0.1f*dx;
		r[i].y += -0.1f*dy;
	}
}



// https://www.flecs.dev/explorer/?remote=true
int main(int argc, char *argv[])
{
	ecs_log_set_level(0);
	//ecs_world_t * world = ecs_init_w_args(argc, argv);
	ecs_world_t *world = ecs_init_w_args(1, (char*[]){
	"rest_test", NULL // Application name, optional
	});

	ECS_IMPORT(world, FlecsComponentsEgSDL);
	ECS_IMPORT(world, FlecsComponentsEgGeometry);
	ECS_IMPORT(world, FlecsComponentsEgWindow);


	ECS_COMPONENT_DEFINE(world, EgPlayer);
	ECS_COMPONENT_DEFINE(world, EgEnemy);

	ECS_SYSTEM(world, Move_Player, EcsOnUpdate, EgPlayer, $EgUserinput, EgPosition2F32);
	ECS_SYSTEM(world, Move_Enemy, EcsOnUpdate, EgEnemy, EgPosition2F32);
	

	ecs_entity_t e1 = ecs_new(world, 0);
	ecs_set_name(world, e1, "My app");
	ecs_set(world, e1, EgWindow, {NULL, 0});
	ecs_set(world, e1, EgRectangleI32, {400, 300});
	
	ecs_entity_t e2 = ecs_new_w_pair(world, EcsChildOf, e1);
	ecs_set(world, e2, EgDraw, {1});
	ecs_set(world, e2, EgPosition2F32, {50, 50});
	ecs_set(world, e2, EgRectangleF32, {50, 50});
	ecs_add(world, e2, EgPlayer);

	ecs_entity_t e3 = ecs_new_w_pair(world, EcsChildOf, e1);
	ecs_set(world, e3, EgDraw, {1});
	ecs_set(world, e3, EgEnemy, {0});
	ecs_set(world, e3, EgPosition2F32, {0, 0});
	ecs_set(world, e3, EgRectangleF32, {80, 80});

	ecs_set(world, EcsWorld, EcsRest, {0});
	
	while (1)
	{
		ecs_os_sleep(0,100000);
		ecs_progress(world, 0);
	}

	return 0;
}
