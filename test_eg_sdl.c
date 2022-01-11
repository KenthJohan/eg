#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "flecs.h"
#include "eg_geometry.h"
#include "eg_window.h"
#include "eg_userevent.h"
#include "eg_base.h"
#include "eg_quantity.h"

// If SDL is used as backend
#if 1
#include "eg_sdl.h"
#endif


typedef struct
{
	int dummy;
} EgPlayer;

typedef struct
{
	float dummy;
} EgEnemy;

typedef struct
{
	float dummy;
} EgProjectile;


typedef struct
{
	int dummy;
} EgPlayground;


ECS_COMPONENT_DECLARE(EgPlayer);
ECS_COMPONENT_DECLARE(EgEnemy);
ECS_COMPONENT_DECLARE(EgPlayground);
ECS_COMPONENT_DECLARE(EgProjectile);


static void Playground_Update(ecs_iter_t *it)
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

static void Bounce(ecs_iter_t *it)
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

static void Move_Enemy(ecs_iter_t *it)
{
	EgEnemy *e = ecs_term(it, EgEnemy, 1);
	EgForce2F32 *a = ecs_term(it, EgForce2F32, 2);
	for (int i = 0; i < it->count; i ++)
	{
		e[i].dummy += 0.001f;;
		a[i].x = sin(e[i].dummy);
		a[i].y = cos(e[i].dummy + i*a[i].x);
	}
}


static void Move_Player(ecs_iter_t *it)
{
	EgPlayer *player = ecs_term(it, EgPlayer, 1);
	EgUserEvent *u = ecs_term(it, EgUserEvent, 2); // Singleton
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
		p[i].x -= k * 0.5f;
		p[i].y -= k * 0.5f;
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
	ECS_COMPONENT_DEFINE(world, EgProjectile);

	ECS_SYSTEM(world, Move_Player, EcsOnUpdate,
	[inout] EgPlayer,
	[in]    $EgUserEvent,
	[inout] EgPosition2F32,
	[inout] EgRectangleF32);
	ECS_SYSTEM(world, Move_Enemy, EcsOnUpdate,
	[inout] EgEnemy,
	[out]   EgForce2F32);
	ECS_SYSTEM(world, Bounce, EcsOnUpdate,
	[out]   EgPlayground(parent),
	[in]    EgRectangleI32(parent),
	[in]    EgPosition2F32,
	[out]   EgVelocity2F32);
	ECS_SYSTEM(world, Playground_Update, EcsOnUpdate,
	[out]   EgPlayground,
	[out]   EgRectangleI32,
	[in]    EgRectangleI32(parent));
	

	ecs_entity_t window1 = ecs_new(world, 0);
	ecs_entity_t window2 = ecs_new(world, 0);
	ecs_set_name(world, window1, "Window1");
	ecs_set_name(world, window2, "Window2");
	ecs_set(world, window1, EgRectangleI32, {800, 800});
	ecs_set(world, window2, EgRectangleI32, {800, 800});
	ecs_set(world, window1, EgWindow, {EG_WINDOW_OPENGL|EG_WINDOW_RESIZABLE, 0, false, false});
	ecs_set(world, window2, EgWindow, {EG_WINDOW_OPENGL|EG_WINDOW_RESIZABLE, 0, false, false});
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
		ecs_set(world, e2, EgVelocity2F32, {0, 0});
		ecs_add(world, e2, EgAcceleration2F32);
		ecs_set(world, e2, EgForce2F32, {0, 0});
		ecs_set(world, e2, EgMassF32, {1000.0f});
		ecs_add(world, e2, EgMomentum2F32);
	}

	{
		ecs_entity_t e1 = ecs_new_w_pair(world, EcsChildOf, playground2);
		ecs_set_name(world, e1, "Enemy2");
		ecs_set(world, e1, EgDraw, {1});
		ecs_set(world, e1, EgEnemy, {0});
		ecs_set(world, e1, EgPosition2F32, {50, 50});
		ecs_set(world, e1, EgRectangleF32, {50, 50});
		ecs_set(world, e1, EgVelocity2F32, {0, 0});
		ecs_add(world, e1, EgAcceleration2F32);
		ecs_set(world, e1, EgForce2F32, {0, 0});
		ecs_set(world, e1, EgMassF32, {1000.0f});
		ecs_add(world, e1, EgMomentum2F32);

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
