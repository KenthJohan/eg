#include "MyIntersectors.h"

#include <egcomponents.h>
#include <egspatials.h>
#include <egshapes.h>
#include <egmath.h>


ECS_COMPONENT_DECLARE(MyIntersectorsHit);

/*

https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-sphere-intersection.html

bool solveQuadratic(const float &a, const float &b, const float &c, float &x0, float &x1)
{
    float discr = b * b - 4 * a * c;
    if (discr < 0) return false;
    else if (discr == 0) x0 = x1 = - 0.5 * b / a;
    else {
        float q = (b > 0) ?
            -0.5 * (b + sqrt(discr)) :
            -0.5 * (b - sqrt(discr));
        x0 = q / a;
        x1 = c / q;
    }
    if (x0 > x1) std::swap(x0, x1);
    
    return true;
}

bool intersect(const Ray &ray) const
{
        float t0, t1; // solutions for t if the ray intersects
#if 0
        // Geometric solution
        Vec3f L = center - ray.orig;
        float tca = L.dotProduct(ray.dir);
        // if (tca < 0) return false;
        float d2 = L.dotProduct(L) - tca * tca;
        if (d2 > radius * radius) return false;
        float thc = sqrt(radius * radius - d2);
        t0 = tca - thc;
        t1 = tca + thc;
#else
        // Analytic solution
        Vec3f L = ray.orig - center;
        float a = ray.dir->dotProduct(ray.dir);
        float b = 2 * ray.dir->dotProduct(L);
        float c = L.dotProduct(L) - radius * radius;
        if (!solveQuadratic(a, b, c, t0, t1)) return false;
#endif
        if (t0 > t1) std::swap(t0, t1);

        if (t0 < 0) {
            t0 = t1; // If t0 is negative, let's use t1 instead.
            if (t0 < 0) return false; // Both t0 and t1 are negative.
        }

        t = t0;

        return true;
}

*/





float intersect(float const rp[3], float const rd[3], float const cp[3], float const cd[3])
{   
	float rc[3];
	rc[0] = rp[0] - cp[0];
	rc[1] = rp[1] - cp[1];
	rc[2] = rp[2] - cp[2];
	float n[3];
	n[0] = V3_CROSS0(rd, cd);
	n[1] = V3_CROSS1(rd, cd);
	n[2] = V3_CROSS2(rd, cd);
	v3f32_normalize(n, n, 0.0001);
	float d = fabs(V3_DOT(rc, n));
	return d;
}




static void Cylinder_Intersect(ecs_iter_t *it)
{
	Cylinder *cyl = ecs_field(it, Cylinder, 1); // self
	Position3World *cp = ecs_field(it, Position3World, 2); // self
	OrientationWorld *cr = ecs_field(it, OrientationWorld, 3); // self
	MyIntersectorsHit *ish = ecs_field(it, MyIntersectorsHit, 4); // self
	
	
	ecs_entity_t line_e = ecs_lookup_fullpath(it->world, "app.line1");
	Line const * line = ecs_get(it->world, line_e, Line);

	float rd[3];
	rd[0] = line->b[0] - line->a[0];
	rd[1] = line->b[1] - line->a[1];
	rd[2] = line->b[2] - line->a[2];

	for (int i = 0; i < it->count; ++i, ++cyl, ++cp, ++cr, ++ish) {
		m3f32 r = {0};
		qf32_unit_to_m3((float*)cr, &r);
		ish->d = intersect(line->a, (float*)rd, (float*)cp, r.c1);
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
	.entity = ecs_entity(world, {.name = "Cylinder_Intersect", .add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = Cylinder_Intersect,
	.query.filter.terms =
	{
	{.id = ecs_id(Cylinder), .src.flags = EcsSelf},
	{.id = ecs_id(Position3World), .src.flags = EcsSelf},
	{.id = ecs_id(OrientationWorld), .src.flags = EcsSelf},
	{.id = ecs_id(MyIntersectorsHit), .src.flags = EcsSelf}
	}});


}