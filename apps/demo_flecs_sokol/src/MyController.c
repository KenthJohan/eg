#include "MyController.h"

#include <egsokol.h>
#include <egcomponents.h>
#include <egspatials.h>
#include <egcameras.h>
#include <egshapes.h>
#include "MiscLines.h"


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

static void PrintMousePos(ecs_iter_t *it)
{
	Window *win = ecs_field(it, Window, 1);
	Camera *cam = ecs_field(it, Camera, 2);
	Position3 *pos = ecs_field(it, Position3, 3);
	//Orientation *rot = ecs_field(it, Orientation, 4);

	win->dt = it->delta_time;
	win->fps = 1.0f / it->delta_time;
	win->pos[0] = pos->x;
	win->pos[1] = pos->y;
	win->pos[2] = pos->z;

	float mouse_pos[2] = {win->canvas_mouse_x, win->canvas_mouse_y};
	float rectangle[2] = {win->canvas_width, win->canvas_height};
	
	/*
	sdtx_canvas(win->w/ 2.0f, win->h/ 2.0f);
	sdtx_origin(1.0f, 1.0f);
	sdtx_color3f(1.0f, 1.0f, 1.0f);
	sdtx_pos(0, 0);
	sdtx_printf("FPS: %f", 1.0f / it->delta_time);
	sdtx_pos(0, 1);
	sdtx_printf("Pos: %f %f %f", pos->x, pos->y, pos->z);
	sdtx_pos(0, 2);
	sdtx_printf("Rot: %f %f %f %f", rot->x, rot->y, rot->z, rot->w);
	sdtx_canvas(win->w / 2.0f, win->h / 2.0f);
	sdtx_origin(win->mouse_x / 16.0f, win->mouse_y / 16.0f);
	sdtx_color3f(1.0f, 1.0f, 1.0f);
	*/

	float r[4];
	r[0] = 2.0f * (mouse_pos[0] / rectangle[0]) - 1.0f;
	r[1] = 2.0f * (mouse_pos[1] / rectangle[1]) - 1.0f;
	r[1] *= -1.0f; //Why flip, hmm?
	r[2] = -1.0;
	r[3] = 1.0;

	// Eye/Camera
	// vec4 ray_eye = mat4_mul_vec4(mat4_inverse(projection), ray_clip);

	float ray_eye[4];
	m4f32 pinv;
	m4f32_inverse((float *)&cam->projection, (float *)&pinv);
	m4f32_mulv(&pinv, r, ray_eye);
	ray_eye[2] = -1.0f;
	ray_eye[3] = 0.0f;

	// Convert to world coordinates;
	// r.direction = vec3_from_vec4(mat4_mul_vec4(view, ray_eye));
	// vec3_normalize(&r.direction);

	m4f32 vinv;
	float ray_world[4];
	m4f32_inverse((float *)&cam->view, (float *)&vinv);
	m4f32_mulv(&vinv, ray_eye, ray_world);

	/*
	sdtx_pos(3, 0);
	sdtx_printf("%f %f", win->mouse_x, win->mouse_y);
	sdtx_pos(3, 1);
	sdtx_printf("%f %f", r[0], r[1]);
	sdtx_pos(3, 2);
	sdtx_printf("%f %f %f %f", ray_eye[0], ray_eye[1], ray_eye[2], ray_eye[3]);
	sdtx_pos(3, 3);
	sdtx_printf("%f %f %f %f", ray_world[0], ray_world[1], ray_world[2], ray_world[3]);
	v3f32_normalize(ray_world, ray_world);
	sdtx_pos(3, 4);
	sdtx_printf("%f %f %f %f", ray_world[0], ray_world[1], ray_world[2], ray_world[3]);
	*/


	if(win->mouse_left_edge)
	{
		float length = 1000.0f;
		ecs_entity_t e = ecs_lookup_fullpath(it->world, "app.line1");
		Line line = {
			// TODO:
			// Camera position flipped, hmm?
			// Shoot ray from mouse position or camera position?
			.a = {-pos->x, -pos->y, -pos->z},
			.b = {-pos->x+ ray_world[0]*length, -pos->y+ ray_world[1]*length, -pos->z+ ray_world[2]*length}
		};
		ecs_set_ptr(it->world, e, Line, &line);
		printf("mouse_left_edge\n");
	}
}








static void KeyActionToggleEntity_OnUpdate(ecs_iter_t *it)
{
	Window *window = ecs_field(it, Window, 1);
	KeyActionToggleEntity *action = ecs_field(it, KeyActionToggleEntity, 2);
	uint8_t *keys_edge = window->keys_edge;
	for (int i = 0; i < it->count; ++i, ++action) {
		if(keys_edge[action->keycode]) {
			//ecs_add_id(it->world, it->entities[i], action->entity);
			if(ecs_has_pair(it->world, it->entities[i], EcsIsA, action->entity)) {
				ecs_remove_pair(it->world, it->entities[i], EcsIsA, action->entity);
			} else {
				ecs_add_pair(it->world, it->entities[i], EcsIsA, action->entity);
			}
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

	ECS_SYSTEM(world, ControllerRotate, EcsOnUpdate, KeyboardController, Rotate3, Window($));
	ECS_SYSTEM(world, ControllerMove, EcsOnUpdate, KeyboardController, Velocity3, Window($));
	ECS_SYSTEM(world, ControllerPerspective, EcsOnUpdate, KeyboardController, Camera, Window($));
	ECS_SYSTEM(world, PrintMousePos, EcsOnUpdate, Window($), Camera, Position3, Orientation);
	ECS_SYSTEM(world, KeyActionToggleEntity_OnUpdate, EcsOnUpdate, Window($), KeyActionToggleEntity);
}