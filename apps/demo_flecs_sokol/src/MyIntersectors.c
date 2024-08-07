#include "MyIntersectors.h"

#include <egbase.h>
#include <egspatials.h>
#include <egshapes.h>
#include <egmath.h>
#include <egcolors.h>

ECS_COMPONENT_DECLARE(MyIntersectorsHit);

#if 1
static void Cylinder_Intersect(ecs_iter_t *it)
{
	Position3 *l = ecs_field(it, Position3, 0);                       // shared
	Ray3 *v = ecs_field(it, Ray3, 1);                                 // shared
	Cylinder *cyl = ecs_field(it, Cylinder, 2);                       // shared
	Position3World *o = ecs_field(it, Position3World, 3);             // self
	OrientationWorld *r = ecs_field(it, OrientationWorld, 4);         // self
	Scale3 *s = ecs_field(it, Scale3, 5);                             // self
	EgColorsV4F32_RGBA *color = ecs_field(it, EgColorsV4F32_RGBA, 6); // self
	//printf("%f %f %f, %f %f %f\n", l->x, l->y, l->z, v->x, v->y, v->z);

	float h[3] = {0, 1, 0}; // The non transformed cylinder axis
	for (int i = 0; i < it->count; ++i, ++o, ++r, ++s, ++color) {
		m3f32 tt = {0};
		m3f32_rs_inverse_transposed((float const *)r, (float const *)s, &tt);
		int hit1 = v3f32_intersect_cylinder((float *)v, (float *)l, (float *)o, h, &tt);
		if (hit1) {
			color->r = 1;
			color->g = 1;
			color->b = 1;
			color->a = 1;
		} else {
			color->r = 0;
			color->g = 0;
			color->b = 0;
			color->a = -3;
		}
		// printf("hit1: %i\n", hit1);
	}
}

#else
static void Cylinder_Intersect(ecs_iter_t *it)
{
	// Cylinder *cyl = ecs_field(it, Cylinder, 1);                   // shared
	Position3World *o = ecs_field(it, Position3World, 2);         // self
	OrientationWorld *r = ecs_field(it, OrientationWorld, 3);     // self
	Scale3 *s = ecs_field(it, Scale3, 4);                         // self
	MyIntersectorsHit *hit = ecs_field(it, MyIntersectorsHit, 5); // self

	ecs_entity_t line_e = ecs_lookup_fullpath(it->world, "app.line1");
	Line const *line = ecs_get(it->world, line_e, Line);

	ecs_entity_t hitpoint1_e = ecs_lookup_fullpath(it->world, "app.items.hitpoint1");
	ecs_entity_t hitpoint2_e = ecs_lookup_fullpath(it->world, "app.items.hitpoint2");

	// Vector that defines the line direction in world space
	float v[3];
	v[0] = line->b[0] - line->a[0];
	v[1] = line->b[1] - line->a[1];
	v[2] = line->b[2] - line->a[2];
	v3f32_normalize(v, v, 0.000001);

	float h[3] = {0, 1, 0};

	for (int i = 0; i < it->count; ++i, ++o, ++r, ++s, ++hit) {
		m3f32 tt = {0};
		m3f32_rs_inverse_transposed((float const *)r, (float const *)s, &tt);
		/*
		Position3 hit[2];
		float b2_4ac = v3f32_intersect_cylinder(v, line->a, (float *)o, h, &tt, (float*)(hit+0), (float*)(hit+1));
		ecs_set_ptr(it->world, hitpoint1_e, Position3, hit + 0);
		ecs_set_ptr(it->world, hitpoint2_e, Position3, hit + 1);
		*/
		int hit1 = v3f32_intersect_cylinder(v, line->a, (float *)o, h, &tt);
		printf("hit1: %i\n", hit1);
	}
}
#endif

void MyIntersectorsImport(ecs_world_t *world)
{
	ECS_MODULE(world, MyIntersectors);
	ECS_IMPORT(world, EgBase);
	ECS_IMPORT(world, EgSpatials);
	ECS_IMPORT(world, EgShapes);
	ECS_IMPORT(world, EgColors);
	ecs_set_name_prefix(world, "MyIntersectors");

	ECS_COMPONENT_DEFINE(world, MyIntersectorsHit);
	ecs_add_id(world, ecs_id(MyIntersectorsHit), EcsTraversable);

	ecs_struct(world,
	{.entity = ecs_id(MyIntersectorsHit),
	.members = {
	{.name = "d", .type = ecs_id(ecs_f32_t)},
	}});



	ecs_system(world,{
	.entity = ecs_entity(world, {.name = "Cylinder_Intersect", .add = ecs_ids(ecs_dependson(EcsPostUpdate))}),
	.callback = Cylinder_Intersect,
	.query.terms =
	{
	{.id = ecs_id(Position3), .trav = ecs_id(MyIntersectorsHit), .src.id = EcsUp},
	{.id = ecs_id(Ray3), .trav = ecs_id(MyIntersectorsHit), .src.id = EcsUp},
	{.id = ecs_id(Cylinder), .trav = EgBaseUse, .src.id = EcsUp}, // TODO
	{.id = ecs_id(Position3World), .src.id = EcsSelf},
	{.id = ecs_id(OrientationWorld), .src.id = EcsSelf},
	{.id = ecs_id(Scale3), .src.id = EcsSelf},
	{.id = ecs_id(EgColorsV4F32_RGBA), .src.id = EcsSelf}
	}});
}