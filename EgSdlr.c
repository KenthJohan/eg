#include "EgSdlr.h"
#include "EgSdlw.h"
#include "EgGeometries.h"
#include "EgWindows.h"
#include "EgEvents.h"
#include "EgQuantities.h"
#include "eg_basics.h"
#include <SDL2/SDL.h>







typedef struct
{
	SDL_Renderer *renderer;
} Eg_SDL_Renderer;


typedef struct
{
	ecs_i32_t capacity;
	ecs_i32_t count;
	SDL_Vertex * v;
} Eg_SDL_Mesh;


static void Eg_SDL_Mesh_push(Eg_SDL_Mesh * m, SDL_Vertex * v, ecs_i32_t count)
{
	EG_ASSERT(m);
	EG_ASSERT(v);
	EG_ASSERT(m->v);
	ecs_i32_t left = m->capacity - m->count;
	ecs_i32_t n = ECS_MIN(left, count);
	if (n > 0)
	{
		ecs_os_memcpy(m->v + m->count, v, n * sizeof(SDL_Vertex));
		m->count += n;
	}
}


ECS_COMPONENT_DECLARE(Eg_SDL_Renderer);
ECS_COMPONENT_DECLARE(Eg_SDL_Mesh);



static void Eg_SDL_Mesh_CTOR(Eg_SDL_Mesh * ptr)
{
	EG_TRACE("Eg_SDL_Mesh::ECS_CTOR");
	ptr->v = NULL;
	ptr->count = 0;
	ptr->capacity = 0;
}

static void Eg_SDL_Mesh_DTOR(Eg_SDL_Mesh * ptr)
{
	EG_TRACE("Eg_SDL_Mesh::ECS_DTOR");
	if(ptr->v){ecs_os_free(ptr->v);}
	ptr->v = NULL;
	ptr->count = 0;
	ptr->capacity = 0;
}

static void Eg_SDL_Mesh_MOVE(Eg_SDL_Mesh * dst, Eg_SDL_Mesh * src)
{
	EG_TRACE("Eg_SDL_Mesh::ECS_MOVE");
	EG_ASSERT(dst);
	EG_ASSERT(src);
	if(dst->v){ecs_os_free(dst->v);}
	dst->v = src->v;
	dst->count = src->count;
	dst->capacity = src->capacity;
	src->v = NULL;
	src->count = 0;
	src->capacity = 0;
}

static void Eg_SDL_Mesh_COPY(Eg_SDL_Mesh * dst, Eg_SDL_Mesh * src)
{
	EG_TRACE("Eg_SDL_Mesh::ECS_COPY");
	EG_ASSERT(dst);
	EG_ASSERT(src);
	if(dst->v){ecs_os_free(dst->v);}
	if(src->v)
	{
		int32_t size = src->capacity * sizeof(SDL_Vertex);
		dst->v = ecs_os_calloc(size);
		ecs_os_memcpy(dst->v, src->v, size);
		dst->count = src->count;
		dst->capacity = src->capacity;
	}
	else
	{
		dst->v = NULL;
		dst->count = 0;
		dst->capacity = 0;
	}
}

ECS_CTOR(Eg_SDL_Mesh, ptr, {Eg_SDL_Mesh_CTOR(ptr);});
ECS_DTOR(Eg_SDL_Mesh, ptr, {Eg_SDL_Mesh_DTOR(ptr);});
ECS_MOVE(Eg_SDL_Mesh, dst, src, {Eg_SDL_Mesh_MOVE(dst, src);})
ECS_COPY(Eg_SDL_Mesh, dst, src, {Eg_SDL_Mesh_COPY(dst, src);})






static void Vertex_Rect(SDL_Vertex v[6], EgPosition2F32 * p, EgRectangleF32 * r)
{
	float w2 = r->width / 2;
	float h2 = r->height / 2;
	v[0].position.x = p->x - w2;
	v[0].position.y = p->y - h2;
	v[1].position.x = p->x + w2;
	v[1].position.y = p->y - h2;
	v[2].position.x = p->x + w2;
	v[2].position.y = p->y + h2;

	v[3].position.x = p->x - h2;
	v[3].position.y = p->y - w2;
	v[4].position.x = p->x - h2;
	v[4].position.y = p->y + h2;
	v[5].position.x = p->x + w2;
	v[5].position.y = p->y + h2;
}


static void Draw_Rectangle(ecs_iter_t *it)
{
	Eg_SDL_Mesh *m = ecs_term(it, Eg_SDL_Mesh, 1); // Parent
	EgPosition2F32 *p = ecs_term(it, EgPosition2F32, 2);
	EgRectangleF32 *r = ecs_term(it, EgRectangleF32, 3);
	for (int i = 0; i < it->count; i ++)
	{
		SDL_Vertex vert[6];
		Vertex_Rect(vert, p + i, r + i);
		vert[0].color.r = 255;
		vert[0].color.g = 0;
		vert[0].color.b = 0;
		vert[0].color.a = 255;
		vert[1].color.r = 0;
		vert[1].color.g = 0;
		vert[1].color.b = 255;
		vert[1].color.a = 255;
		vert[2].color.r = 0;
		vert[2].color.g = 255;
		vert[2].color.b = 0;
		vert[2].color.a = 255;
		vert[3].color.r = 255;
		vert[3].color.g = 100;
		vert[3].color.b = 100;
		vert[3].color.a = 255;
		vert[4].color.r = 100;
		vert[4].color.g = 100;
		vert[4].color.b = 255;
		vert[4].color.a = 255;
		vert[5].color.r = 100;
		vert[5].color.g = 255;
		vert[5].color.b = 100;
		vert[5].color.a = 255;
		Eg_SDL_Mesh_push(m+0, vert, 6);
	}
}



static void Render_Mesh(ecs_iter_t *it)
{
	//EG_ITER_INFO(it);
	Eg_SDL_Renderer *r = ecs_term(it, Eg_SDL_Renderer, 1);
	Eg_SDL_Mesh *m = ecs_term(it, Eg_SDL_Mesh, 2);
	for (int i = 0; i < it->count; i ++)
	{
		SDL_Renderer *renderer = r[i].renderer;
		SDL_Vertex * v = m[i].v;
		EG_ASSERT(renderer);
		EG_ASSERT(v);
		if (renderer == NULL){continue;}
		if (m[i].count <= 0){continue;}
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);
		SDL_RenderGeometry(renderer, NULL, v, m[i].count, NULL, 0);
		SDL_RenderPresent(renderer);
		m[i].count = 0;
	}
}


static void Create_Renderer(ecs_iter_t *it)
{
	//EG_ITER_INFO(it);
	ecs_world_t * world = it->world;
	Eg_SDL_Window *m = ecs_term(it, Eg_SDL_Window, 1);
	for (int i = 0; i < it->count; i ++)
	{
		ecs_entity_t e = it->entities[i];
		Eg_SDL_Mesh mesh;
		mesh.capacity = 6*100;
		mesh.count = 0;
		mesh.v = ecs_os_calloc(mesh.capacity * sizeof(SDL_Vertex));
		ecs_set(world, e, Eg_SDL_Mesh, {mesh.capacity, mesh.count, mesh.v});
		SDL_Renderer *renderer = SDL_CreateRenderer(m[i].window, -1, SDL_RENDERER_ACCELERATED);
		EG_ASSERT(renderer);
		ecs_set(world, e, Eg_SDL_Renderer, {renderer});
	}
}





void EgSdlrImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgSdlr);
	ECS_IMPORT(world, EgSdlw);
	ECS_IMPORT(world, EgWindows);
	ECS_IMPORT(world, EgGeometries);
	ECS_IMPORT(world, EgEvents);
	ECS_IMPORT(world, EgQuantities);
	ecs_set_name_prefix(world, "Eg");

	ECS_COMPONENT_DEFINE(world, Eg_SDL_Renderer);
	ECS_COMPONENT_DEFINE(world, Eg_SDL_Mesh);



	ecs_set_component_actions(world, Eg_SDL_Mesh, {
	//.ctor = ecs_ctor(Eg_SDL_Mesh),
	ecs_default_ctor,
	.dtor = ecs_dtor(Eg_SDL_Mesh),
	.copy = ecs_copy(Eg_SDL_Mesh),
	.move = ecs_move(Eg_SDL_Mesh),
	});



	// https://discord.com/channels/633826290415435777/731400638637932604/927345118632091718
	// But because that system becomes inactive (no matching entities) as soon as it's ran once, it is removed from the schedule
	// Reactive systems
	ECS_SYSTEM(world, Create_Renderer, EcsOnLoad,
	[in]   Eg_SDL_Window,
	[out] !Eg_SDL_Renderer);
	ECS_SYSTEM(world, Draw_Rectangle, EcsOnUpdate,
	[out] Eg_SDL_Mesh(parent),
	[in]  EgPosition2F32,
	[in]  EgRectangleF32);
	ECS_SYSTEM(world, Render_Mesh, EcsOnUpdate,
	[in]  Eg_SDL_Renderer,
	[out] Eg_SDL_Mesh);
}

