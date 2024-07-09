#include "MyIntersectors.h"

#include <egcomponents.h>
#include <egspatials.h>
#include <egshapes.h>
#include <egmath.h>

ECS_COMPONENT_DECLARE(MyIntersectorsHit);

static void Cylinder_Intersect(ecs_iter_t *it)
{
	Cylinder *cyl = ecs_field(it, Cylinder, 1);                   // shared
	Position3World *cp = ecs_field(it, Position3World, 2);        // self
	OrientationWorld *cr = ecs_field(it, OrientationWorld, 3);    // self
	Scale3 *s = ecs_field(it, Scale3, 4);                         // self
	MyIntersectorsHit *ish = ecs_field(it, MyIntersectorsHit, 5); // self

	ecs_entity_t line_e = ecs_lookup_fullpath(it->world, "app.line1");
	Line const *line = ecs_get(it->world, line_e, Line);

	// Vector that defines the line direction in world space
	float v[4];
	v[0] = line->b[0] - line->a[0];
	v[1] = line->b[1] - line->a[1];
	v[2] = line->b[2] - line->a[2];
	v3f32_normalize(v, v, 0.000001);
	v[3] = 0.0;

	

	float h[3] = {0, 1, 0};

	for (int i = 0; i < it->count; ++i, ++cp, ++cr, ++s, ++ish) {
		
		m4f32 t = {0};
		m4f32_trs_inverse((float const *)cp, (float const *)cr, (float const *)s, &t);
		float b2_4ac = v3f32_intersect_cylinder2(v, line->a, (float*)cp, h, &t);
		
		//float b2_4ac = v3f32_intersect_cylinder(v, line->a, (float const*)cp, h, (float const*)cr);
		printf("b2_4ac: %+f\n", b2_4ac);
	}
}

void MyIntersectorsImport(ecs_world_t *world)
{
	ECS_MODULE(world, MyIntersectors);
	ECS_IMPORT(world, EgComponents);
	ECS_IMPORT(world, EgSpatials);
	ECS_IMPORT(world, EgShapes);
	ecs_set_name_prefix(world, "MyIntersectors");

	ECS_COMPONENT_DEFINE(world, MyIntersectorsHit);

	ecs_struct(world,
	{.entity = ecs_id(MyIntersectorsHit),
	.members = {
	{.name = "d", .type = ecs_id(ecs_f32_t)},
	}});

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.name = "Cylinder_Intersect", .add = {ecs_dependson(EcsPostUpdate)}}),
	.callback = Cylinder_Intersect,
	.query.filter.terms =
	{
	{.id = ecs_id(Cylinder), .src.trav = EgUse, .src.flags = EcsUp},
	{.id = ecs_id(Position3World), .src.flags = EcsSelf},
	{.id = ecs_id(OrientationWorld), .src.flags = EcsSelf},
	{.id = ecs_id(Scale3), .src.flags = EcsSelf},
	{.id = ecs_id(MyIntersectorsHit), .src.flags = EcsSelf}

	}});
}