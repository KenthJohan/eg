#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "flecs.h"
#include "eg_basics.h"
#include "EgGeometries.h"
#include "EgWindows.h"
#include "EgEvents.h"
#include "EgQuantities.h"

// If SDL is used as backend
#if 1
#include "EgSdl.h"
#include "EgSdlr.h"
#endif


typedef struct
{
	int dummy;
} EgPlayer;

typedef struct
{
	int dummy;
} EgCat;

typedef struct
{
	float dummy;
} EgProjectile;


typedef struct
{
	int dummy;
} EgPlayground;


ECS_COMPONENT_DECLARE(EgPlayer);
ECS_COMPONENT_DECLARE(EgCat);
ECS_COMPONENT_DECLARE(EgPlayground);
ECS_COMPONENT_DECLARE(EgProjectile);


float random1(float a, float b)
{
	float n = (float)rand() / (float)RAND_MAX;
	n *= (b-a);
	n += a;
	return EG_CLAMP(n, a, b);
}


static void System_Playground_Update(ecs_iter_t *it)
{
	EgPlayground   *playground = ecs_term(it, EgPlayground,   1); // [out]  This
	EgRectangleI32 *rectangle  = ecs_term(it, EgRectangleI32, 2); // [out]  This
	EgRectangleI32 *wrectangle = ecs_term(it, EgRectangleI32, 3); // [in]   Parent
	for (int i = 0; i < it->count; i ++)
	{
		playground[i].dummy = 0;
		rectangle[i].width = wrectangle[0].width;
		rectangle[i].height = wrectangle[0].height;
	}
}

static void System_Bounce(ecs_iter_t *it)
{
	EgPlayground   *playground = ecs_term(it, EgPlayground,   1); // [out]   Parent
	EgRectangleI32 *rectangle  = ecs_term(it, EgRectangleI32, 2); // [in]    Parent
	EgPosition2F32 *position   = ecs_term(it, EgPosition2F32, 3); // [in]    This
	EgVelocity2F32 *velocity   = ecs_term(it, EgVelocity2F32, 4); // [inout] This
	for (int i = 0; i < it->count; i ++)
	{
		playground[0].dummy = 0;
		velocity[i].x *= (position[i].x > rectangle[0].width) ? 1.0f : -1.0f;
		velocity[i].y *= (position[i].y > rectangle[0].height) ? 1.0f : -1.0f;
		velocity[i].x *= (position[i].x < 0) ? 1.0f : -1.0f;
		velocity[i].y *= (position[i].y < 0) ? 1.0f : -1.0f;
	}
}


static void System_Move_Enemy(ecs_iter_t *it)
{
	EgCat *e = ecs_term(it, EgCat, 1);
	EgForce2F32 *f = ecs_term(it, EgForce2F32, 2);
	for (int i = 0; i < it->count; i ++)
	{
		e[i].dummy += 0.001f;
		if(rand() < (RAND_MAX / 3000))
		{
			float a = random1(0, M_PI*2);
			f[i].x = sin(a) * 400.1f;
			f[i].y = cos(a) * 400.1f;
		}
		else
		{
			f[i].x = 0;
			f[i].y = 0;
		}
	}
}


static void System_Move_Player(ecs_iter_t *it)
{
	EgPlayer *player = ecs_term(it, EgPlayer, 1);
	EgUserEvent *u = ecs_term(it, EgUserEvent, 2); // Singleton
	EgForce2F32 *p = ecs_term(it, EgForce2F32, 3);
	EgRectangleF32 *r = ecs_term(it, EgRectangleF32, 4);
	for (int i = 0; i < it->count; i ++)
	{
		player[i].dummy = 0;
		float dx = EG_U64BITSET_GET(u->keyboard, EG_KEY_RIGHT) - EG_U64BITSET_GET(u->keyboard, EG_KEY_LEFT);
		float dy = EG_U64BITSET_GET(u->keyboard, EG_KEY_UP) - EG_U64BITSET_GET(u->keyboard, EG_KEY_DOWN);
		float g = 1.1f;
		p[i].x = g*dx;
		p[i].y = -g*dy;
		float k = 10.0f;
		k *= EG_U64BITSET_GET(u->keyboard_down, EG_KEY_KP_PLUS) - EG_U64BITSET_GET(u->keyboard_down, EG_KEY_KP_MINUS);
		r[i].width += k;
		r[i].height += k;
	}
}


ecs_entity_t spawn_player(ecs_world_t *world, ecs_entity_t parent, char const * name)
{
	ecs_entity_t e1 = ecs_new_w_pair(world, EcsChildOf, parent);
	ecs_set_name(world, e1, name);
	ecs_set(world, e1, EgDraw, {1});
	ecs_set(world, e1, EgPosition2F32, {50, 50});
	ecs_set(world, e1, EgRectangleF32, {50, 50});
	ecs_set(world, e1, EgVelocity2F32, {0, 0});
	ecs_add(world, e1, EgAcceleration2F32);
	ecs_set(world, e1, EgForce2F32, {0, 0});
	ecs_set(world, e1, EgMassF32, {100.0f});
	ecs_add(world, e1, EgMomentum2F32);
	ecs_set(world, e1, EgDrag2F32, {0, 0});
	return e1;
}



// https://www.flecs.dev/explorer/?remote=true
int main(int argc, char *argv[])
{
	eg_basics_enable_ternminal_color();
	ecs_log_set_level(0);
	ecs_world_t *world = ecs_init();
	ecs_singleton_set(world, EcsRest, {0});

	ECS_IMPORT(world, EgSdl);
	ECS_IMPORT(world, EgSdlr);
	ECS_IMPORT(world, EgGeometries);
	ECS_IMPORT(world, EgWindows);
	ECS_IMPORT(world, EgQuantities);


	ECS_COMPONENT_DEFINE(world, EgPlayer);
	ECS_COMPONENT_DEFINE(world, EgCat);
	ECS_COMPONENT_DEFINE(world, EgPlayground);
	ECS_COMPONENT_DEFINE(world, EgProjectile);

	ECS_SYSTEM(world, System_Move_Player, EcsOnUpdate,
	[inout] EgPlayer,
	[in]    EgUserEvent($),
	[inout] EgForce2F32,
	[inout] EgRectangleF32);
	ECS_SYSTEM(world, System_Move_Enemy, EcsOnUpdate,
	[inout] EgCat,
	[out]   EgForce2F32);
	ECS_SYSTEM(world, System_Bounce, EcsOnUpdate,
	[out]   EgPlayground(parent),
	[in]    EgRectangleI32(parent),
	[in]    EgPosition2F32,
	[out]   EgVelocity2F32);
	ECS_SYSTEM(world, System_Playground_Update, EcsOnUpdate,
	[out]   EgPlayground,
	[out]   EgRectangleI32,
	[in]    EgRectangleI32(parent));
	

	ecs_entity_t window1 = ecs_new(world, 0);
	ecs_entity_t window2 = ecs_new(world, 0);
	ecs_set_name(world, window1, "Window1");
	ecs_set_name(world, window2, "Window2");
	ecs_set(world, window1, EgRectangleI32, {800, 800});
	ecs_set(world, window2, EgRectangleI32, {800, 800});
	ecs_set(world, window1, EgWindow, {EG_WINDOW_OPENGL|EG_WINDOW_RESIZABLE, 0, false});
	ecs_set(world, window2, EgWindow, {EG_WINDOW_OPENGL|EG_WINDOW_RESIZABLE, 0, false});
	ecs_set(world, window1, EgTitle, {"Boncy1 Title"});
	ecs_set(world, window2, EgTitle, {"Boncy2 Title"});

	ecs_entity_t playground1 = ecs_new_w_pair(world, EcsChildOf, window1);
	ecs_entity_t playground2 = ecs_new_w_pair(world, EcsChildOf, window2);
	ecs_set_name(world, playground1, "playground1");
	ecs_set_name(world, playground2, "playground2");
	ecs_add(world, playground1, EgPlayground);
	ecs_add(world, playground2, EgPlayground);
	ecs_add(world, playground1, EgRectangleI32);
	ecs_add(world, playground2, EgRectangleI32);
	ecs_set(world, playground1, EgDensityF32, {8.0f});
	ecs_set(world, playground2, EgDensityF32, {2.0f});
	
	{
		ecs_entity_t e1 = spawn_player(world, playground1, "Player1");
		ecs_add(world, e1, EgPlayer);
		ecs_entity_t e2 = spawn_player(world, playground1, "Cat1");
		ecs_add(world, e2, EgCat);
	}

	{
		ecs_entity_t e1 = spawn_player(world, playground2, "Player1");
		ecs_add(world, e1, EgPlayer);
		ecs_entity_t e2 = spawn_player(world, playground2, "Cat2");
		ecs_add(world, e2, EgCat);
	}




	ecs_set(world, EcsWorld, EcsRest, {0});
	
	while (1)
	{
		ecs_os_sleep(0,100000);
		ecs_progress(world, 0);

		EgUserEvent const * k = ecs_singleton_get(world, EgUserEvent);
		if (EG_U64BITSET_GET(k->keyboard_down, EG_KEY_1))
		{
			ecs_set(world, window1, EgRectangleI32, {200, 200});
		}
		if (EG_U64BITSET_GET(k->keyboard_down, EG_KEY_2))
		{
			ecs_set(world, window1, EgRectangleI32, {400, 400});
		}
		if (EG_U64BITSET_GET(k->keyboard_down, EG_KEY_3))
		{
			ecs_set(world, window1, EgRectangleI32, {800, 800});
		}

	}

	return 0;
}
