#include "MyController.h"

#include <egsokol.h>
#include <egcomponents.h>
#include <egspatials.h>
#include <egcameras.h>
#include <egshapes.h>
#include "MiscLines.h"

ECS_TAG_DECLARE(MyControllerTestCopyOnClick);

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
        float a = ray.dir.dotProduct(ray.dir);
        float b = 2 * ray.dir.dotProduct(L);
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

static void ControllerRotate(ecs_iter_t *it)
{
	KeyboardController *controller = ecs_field(it, KeyboardController, 1);
	Rotate3 *rotate = ecs_field(it, Rotate3, 2);
	Window *window = ecs_field(it, Window, 3);
	uint8_t *keys = window->keys;
	float k = 0.8f * it->delta_time;
	for (int i = 0; i < it->count; ++i, ++rotate) {
		rotate->dx = keys[controller->key_rotate_dx_plus] - keys[controller->key_rotate_dx_minus];
		rotate->dy = keys[controller->key_rotate_dy_plus] - keys[controller->key_rotate_dy_minus];
		rotate->dz = keys[controller->key_rotate_dz_plus] - keys[controller->key_rotate_dz_minus];
		v3f32_mul((float *)rotate, (float *)rotate, k);
	}
}

static void ControllerMove(ecs_iter_t *it)
{
	KeyboardController *controller = ecs_field(it, KeyboardController, 1);
	Velocity3 *vel = ecs_field(it, Velocity3, 2);
	Window *window = ecs_field(it, Window, 3);
	uint8_t *keys = window->keys;
	float moving_speed = 1.1f;
	float k = it->delta_time * moving_speed;
	for (int i = 0; i < it->count; i++) {
		vel->x = keys[controller->key_move_dx_plus] - keys[controller->key_move_dx_minus];
		vel->y = keys[controller->key_move_dy_plus] - keys[controller->key_move_dy_minus];
		vel->z = keys[controller->key_move_dz_plus] - keys[controller->key_move_dz_minus];
		v3f32_mul((float *)vel, (float *)vel, k);
	}
}

static void ControllerPerspective(ecs_iter_t *it)
{
	KeyboardController *controller = ecs_field(it, KeyboardController, 1);
	Camera *camera = ecs_field(it, Camera, 2);
	Window *window = ecs_field(it, Window, 3);
	uint8_t *keys = window->keys;
	for (int i = 0; i < it->count; i++) {
		camera->fov = keys[controller->key_fov_reset] ? 45 : camera->fov;
		camera->fov -= keys[controller->key_fov_minus];
		camera->fov += keys[controller->key_fov_plus];
		// sdtx_printf("FOV: %f", camera->fov);
	}
}

static void KeyActionToggleEntity_OnUpdate(ecs_iter_t *it)
{
	Window *window = ecs_field(it, Window, 1);
	KeyActionToggleEntity *action = ecs_field(it, KeyActionToggleEntity, 2);
	uint8_t *keys_edge = window->keys_edge;
	for (int i = 0; i < it->count; ++i, ++action) {
		if (keys_edge[action->keycode]) {
			// ecs_add_id(it->world, it->entities[i], action->entity);
			if (ecs_has_pair(it->world, it->entities[i], EcsIsA, action->entity)) {
				ecs_remove_pair(it->world, it->entities[i], EcsIsA, action->entity);
			} else {
				ecs_add_pair(it->world, it->entities[i], EcsIsA, action->entity);
			}
		}
	}
}

static void System_MyControllerTestCopyOnClick(ecs_iter_t *it)
{
	Window *window = ecs_field(it, Window, 1);       // shared
	Position3 *p0 = ecs_field(it, Position3, 2); // shared
	Ray3 *r0 = ecs_field(it, Ray3, 3);           // shared
	Position3 *p = ecs_field(it, Position3, 4); // self
	Ray3 *r = ecs_field(it, Ray3, 5);           // self
	for (int i = 0; i < it->count; ++i, ++p, ++r) {
		if (window->mouse_left_edge) {
			*p = *p0;
			*r = *r0;
		}
	}
}

void MyControllerImport(ecs_world_t *world)
{
	ECS_MODULE(world, MyController);
	ECS_IMPORT(world, EgComponents);
	ECS_IMPORT(world, EgSpatials);
	ECS_IMPORT(world, EgCameras);
	ECS_IMPORT(world, EgShapes);

	ecs_set_name_prefix(world, "MyController");

	ECS_TAG_DEFINE(world, MyControllerTestCopyOnClick);
	ecs_add_id(world, MyControllerTestCopyOnClick, EcsTraversable);

	ECS_SYSTEM(world, ControllerRotate, EcsOnUpdate, KeyboardController, Rotate3, Window($));
	ECS_SYSTEM(world, ControllerMove, EcsOnUpdate, KeyboardController, Velocity3, Window($));
	ECS_SYSTEM(world, ControllerPerspective, EcsOnUpdate, KeyboardController, Camera, Window($));
	// ECS_SYSTEM(world, PrintMousePos, EcsOnUpdate, Window($), Camera, Position3, Orientation);
	ECS_SYSTEM(world, KeyActionToggleEntity_OnUpdate, EcsOnUpdate, Window($), KeyActionToggleEntity);

	ecs_system_init(world,
	&(ecs_system_desc_t){
	.entity = ecs_entity(world, {.name = "System_MyControllerTestCopyOnClick", .add = {ecs_dependson(EcsOnUpdate)}}),
	.callback = System_MyControllerTestCopyOnClick,
	.query.filter.terms =
	{
	{.id = ecs_id(Window), .src.id = ecs_id(Window)},
	{.id = ecs_id(Position3), .src.trav = MyControllerTestCopyOnClick, .src.flags = EcsUp},
	{.id = ecs_id(Ray3), .src.trav = MyControllerTestCopyOnClick, .src.flags = EcsUp},
	{.id = ecs_id(Position3), .src.flags = EcsSelf},
	{.id = ecs_id(Ray3), .src.flags = EcsSelf},
	}});
}