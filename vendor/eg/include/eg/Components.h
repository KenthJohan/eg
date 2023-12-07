#pragma once
#include <flecs.h>
#include <eg/gmath.h>

typedef struct {
	float w;
	float h;
	float dt;
	float mouse_x;
	float mouse_y;
	float mouse_dx;
	float mouse_dy;
	uint8_t mouse_left;
	uint8_t mouse_right;
	uint8_t mouse_left_edge;
	uint8_t mouse_right_edge;
	uint8_t keys[512];
} Window;

typedef struct {
	float x;
	float y;
} Position2;

typedef struct {
	float x;
	float y;
	float z;
} Position3;

typedef struct {
	float x;
	float y;
} Velocity2;

typedef struct {
	float x;
	float y;
	float z;
} Velocity3;

typedef struct {
	float r;
	float g;
	float b;
	float a;
} Color;

typedef struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
} Color32;

typedef struct {
	char *value;
} String;

typedef struct {
	char *value;
} Pass;

typedef struct {
	float radius;
	float ring_radius;
	int32_t rings;
	int32_t sides;
	int32_t random_colors;
} Torus;

typedef struct {
	float radius;
	float height;
	int32_t slices;
	int32_t stacks;
	int32_t random_colors;
} Cylinder;

typedef struct {
	float radius;
	int32_t slices;
	int32_t stacks;
	int32_t random_colors;
} Sphere;

typedef struct {
	float a[3];
	float b[3];
} Line;

typedef struct {
	int32_t base_element;
	int32_t num_elements;
} ShapeElement;

typedef struct {
	void *ptr;
	int32_t size;
	int32_t cap;
} Memory;

typedef struct {
	int32_t id;
	int32_t cap;
} MemoryGPU;

typedef struct {
	Memory buffer;        // pointer/size pair of output buffer
	int32_t data_size;    // size in bytes of valid data in buffer
	int32_t shape_offset; // data offset of the most recent shape
} ShapeBufferItem;

typedef struct {
	ShapeBufferItem vertices;
	ShapeBufferItem indices;
	MemoryGPU vbuf;
	MemoryGPU ibuf;
} ShapeBuffer;

typedef struct {
	float fov;
	m4f32 view;
	m4f32 projection;
	m4f32 vp;
} Camera;

typedef struct {
	m4f32 matrix;
} Transformation;

typedef struct {
	float x;
	float y;
	float z;
	float w;
} Orientation;

typedef struct {
	float dx;
	float dy;
	float dz;
} Rotate3;

extern ECS_COMPONENT_DECLARE(Window);
extern ECS_COMPONENT_DECLARE(Pass);
extern ECS_COMPONENT_DECLARE(Position2);
extern ECS_COMPONENT_DECLARE(Position3);
extern ECS_COMPONENT_DECLARE(Velocity2);
extern ECS_COMPONENT_DECLARE(Velocity3);
extern ECS_COMPONENT_DECLARE(Color);
extern ECS_COMPONENT_DECLARE(Color32);
extern ECS_COMPONENT_DECLARE(String);
extern ECS_COMPONENT_DECLARE(Torus);
extern ECS_COMPONENT_DECLARE(Cylinder);
extern ECS_COMPONENT_DECLARE(Sphere);
extern ECS_COMPONENT_DECLARE(Line);
extern ECS_COMPONENT_DECLARE(ShapeElement);
extern ECS_COMPONENT_DECLARE(ShapeBuffer);
extern ECS_COMPONENT_DECLARE(Camera);
extern ECS_COMPONENT_DECLARE(Orientation);
extern ECS_COMPONENT_DECLARE(Rotate3);
extern ECS_COMPONENT_DECLARE(Transformation);
extern ECS_TAG_DECLARE(EgUse);
extern ECS_TAG_DECLARE(EgUpdate);

void ComponentsImport(ecs_world_t *world);