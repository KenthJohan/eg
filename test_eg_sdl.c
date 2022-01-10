#include <stdio.h>
#include <stdlib.h>

#include "flecs.h"
#include "eg_sdl.h"
#include "eg_geometry.h"
#include "eg_window.h"
#include "eg_userinput.h"
#include "eg_base.h"
#include "eg_quantity.h"


typedef struct
{
	int dummy;
} EgPlayer;

typedef struct
{
	int dummy;
} EgEnemy;


typedef struct
{
	int dummy;
} EgPlayground;


ECS_COMPONENT_DECLARE(EgPlayer);
ECS_COMPONENT_DECLARE(EgEnemy);
ECS_COMPONENT_DECLARE(EgPlayground);


static void Playground_Update(ecs_iter_t *it)
{
	EgPlayground *y = ecs_term(it, EgPlayground, 1);
	EgRectangleI32 *r = ecs_term(it, EgRectangleI32, 2);
	EgRectangleI32 *wr = ecs_term(it, EgRectangleI32, 3); // Parent
	for (int i = 0; i < it->count; i ++)
	{
		y[i].dummy = 0;
		r[i].width = wr[0].width;
		r[i].height = wr[0].height;
	}
}

static void Bounce(ecs_iter_t *it)
{
	EgPlayground *y = ecs_term(it, EgPlayground, 1); // Parent
	EgRectangleI32 *r = ecs_term(it, EgRectangleI32, 2); // Parent
	EgPosition2F32 *p = ecs_term(it, EgPosition2F32, 3);
	EgVelocity2F32 *v = ecs_term(it, EgVelocity2F32, 4);
	for (int i = 0; i < it->count; i ++)
	{
		v[i].x *= (p[i].x > r[0].width) ? 1.0f : -1.0f;
		v[i].y *= (p[i].y > r[0].height) ? 1.0f : -1.0f;
		v[i].x *= (p[i].x < 0) ? 1.0f : -1.0f;
		v[i].y *= (p[i].y < 0) ? 1.0f : -1.0f;
	}
}

static void Move_Enemy(ecs_iter_t *it)
{
	EgEnemy *e = ecs_term(it, EgEnemy, 1);
	EgAcceleration2F32 *a = ecs_term(it, EgAcceleration2F32, 2);
	for (int i = 0; i < it->count; i ++)
	{
	}
}


static void Move_Player(ecs_iter_t *it)
{
	EgPlayer *player = ecs_term(it, EgPlayer, 1);
	EgUserinput *u = ecs_term(it, EgUserinput, 2); // Singleton
	EgPosition2F32 *p = ecs_term(it, EgPosition2F32, 3);
	EgRectangleF32 *r = ecs_term(it, EgRectangleF32, 4);
	for (int i = 0; i < it->count; i ++)
	{
		player[i].dummy = 0;
		float dx = EG_U64BITSET_GET(u->keyboard, EG_KEY_RIGHT) - EG_U64BITSET_GET(u->keyboard, EG_KEY_LEFT);
		float dy = EG_U64BITSET_GET(u->keyboard, EG_KEY_UP) - EG_U64BITSET_GET(u->keyboard, EG_KEY_DOWN);
		//EG_TRACE("%f %f", dx, dy);
		p[i].x += 0.1f*dx;
		p[i].y += -0.1f*dy;
		float k = 10.0f;
		k *= EG_U64BITSET_GET(u->keyboard_down, EG_KEY_KP_PLUS) - EG_U64BITSET_GET(u->keyboard_down, EG_KEY_KP_MINUS);
		r[i].width += k;
		r[i].height += k;
		p[i].x -= k*0.5f;
		p[i].y -= k*0.5f;
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

	ECS_IMPORT(world, FlecsComponentsEgSdl);
	ECS_IMPORT(world, FlecsComponentsEgGeometry);
	ECS_IMPORT(world, FlecsComponentsEgWindow);
	ECS_IMPORT(world, FlecsComponentsEgQuantity);


	ECS_COMPONENT_DEFINE(world, EgPlayer);
	ECS_COMPONENT_DEFINE(world, EgEnemy);
	ECS_COMPONENT_DEFINE(world, EgPlayground);

	ECS_SYSTEM(world, Move_Player, EcsOnUpdate, EgPlayer, $EgUserinput, EgPosition2F32, EgRectangleF32);
	ECS_SYSTEM(world, Move_Enemy, EcsOnUpdate, EgEnemy, EgAcceleration2F32);
	ECS_SYSTEM(world, Bounce, EcsOnUpdate, EgPlayground(parent), EgRectangleI32(parent), EgPosition2F32, EgVelocity2F32);
	ECS_SYSTEM(world, Playground_Update, EcsOnUpdate, EgPlayground, EgRectangleI32, EgRectangleI32(parent));
	

	ecs_entity_t app1 = ecs_new(world, 0);
	ecs_entity_t app2 = ecs_new(world, 0);
	ecs_set_name(world, app1, "App1");
	ecs_set_name(world, app2, "App2");
	ecs_set(world, app1, EgRectangleI32, {800, 800});
	ecs_set(world, app2, EgRectangleI32, {800, 800});
	ecs_set(world, app1, EgWindow, {NULL, 0});
	ecs_set(world, app2, EgWindow, {NULL, 0});

	ecs_entity_t playground1 = ecs_new_w_pair(world, EcsChildOf, app1);
	ecs_entity_t playground2 = ecs_new_w_pair(world, EcsChildOf, app2);
	ecs_set_name(world, playground1, "playground1");
	ecs_set_name(world, playground2, "playground2");
	ecs_add(world, playground1, EgPlayground);
	ecs_add(world, playground2, EgPlayground);
	ecs_add(world, playground1, EgRectangleI32);
	ecs_add(world, playground2, EgRectangleI32);
	
	{
		ecs_entity_t e1 = ecs_new_w_pair(world, EcsChildOf, playground1);
		ecs_set_name(world, e1, "Player1");
		ecs_set(world, e1, EgDraw, {1});
		ecs_set(world, e1, EgPosition2F32, {50, 50});
		ecs_set(world, e1, EgRectangleF32, {50, 50});
		ecs_add(world, e1, EgPlayer);

		ecs_entity_t e2 = ecs_new_w_pair(world, EcsChildOf, playground1);
		ecs_set_name(world, e2, "Enemy1");
		ecs_set(world, e2, EgDraw, {1});
		ecs_set(world, e2, EgEnemy, {0});
		ecs_set(world, e2, EgPosition2F32, {0, 0});
		ecs_set(world, e2, EgRectangleF32, {80, 80});
		ecs_set(world, e2, EgVelocity2F32, {0.1f, 0.2f});
		ecs_set(world, e2, EgAcceleration2F32, {0.0001f, -0.001f});
	}

	{
		ecs_entity_t e1 = ecs_new_w_pair(world, EcsChildOf, playground2);
		ecs_set_name(world, e1, "Enemy2");
		ecs_set(world, e1, EgDraw, {1});
		ecs_set(world, e1, EgEnemy, {0});
		ecs_set(world, e1, EgPosition2F32, {50, 50});
		ecs_set(world, e1, EgRectangleF32, {50, 50});
		ecs_set(world, e1, EgVelocity2F32, {0.2f, 0.1f});
		ecs_set(world, e1, EgAcceleration2F32, {0.001f, 0.001f});

		ecs_entity_t e2 = ecs_new_w_pair(world, EcsChildOf, playground2);
		ecs_set_name(world, e2, "Player2");
		ecs_set(world, e2, EgDraw, {1});
		ecs_set(world, e2, EgPosition2F32, {50, 50});
		ecs_set(world, e2, EgRectangleF32, {50, 50});
		ecs_add(world, e2, EgPlayer);
	}




	ecs_set(world, EcsWorld, EcsRest, {0});
	
	while (1)
	{
		ecs_os_sleep(0,100000);
		ecs_progress(world, 0);

		EgUserinput const * k = ecs_singleton_get(world, EgUserinput);
		if (EG_U64BITSET_GET(k->keyboard_down, EG_KEY_1))
		{
			ecs_set(world, app1, EgRectangleI32, {200, 200});
		}
		if (EG_U64BITSET_GET(k->keyboard_down, EG_KEY_2))
		{
			ecs_set(world, app1, EgRectangleI32, {400, 400});
		}
		if (EG_U64BITSET_GET(k->keyboard_down, EG_KEY_3))
		{
			ecs_set(world, app1, EgRectangleI32, {800, 800});
		}

	}

	return 0;
}
