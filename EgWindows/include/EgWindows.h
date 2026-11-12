#pragma once
#include <flecs.h>

typedef struct
{
	void *object;
	bool  terminate_on_close;
} EgWindowsWindow;

/* State is refreshed by the window backend after processing input events.
 * Sizes and mouse coordinates are in logical window units unless noted.
 * Coordinates are meaningful only when mouse_focus is true. object identifies
 * the native window for which this snapshot was collected. */
typedef struct
{
	void *object;
	int width, height;
	int pixel_width, pixel_height;
	float mouse_x, mouse_y;
	bool mouse_focus, keyboard_focus;
	bool hidden, minimized;
	bool valid;
} EgWindowsWindowState;

typedef struct
{
	ecs_entity_t mouse_window;
	/* Focus can belong to a window not managed by this world. */
	bool mouse_focus;
} EgWindowsInputState;

/* Opaque native handles, valid only while their window and context exist. */
typedef struct
{
	void *window;
	void *context;
} EgWindowsGlBinding;

typedef struct
{
	/* Backends report failures from make_current, including restoration. */
	EgWindowsGlBinding (*current)(void);
	bool (*make_current)(EgWindowsGlBinding binding);
} EgWindowsGlBackend;

/* Invoked after presentation, or with displayed=false on failure. */
typedef struct
{
	void (*complete)(ecs_world_t *world, ecs_entity_t window, bool displayed);
} EgWindowsFrameCallback;

typedef struct
{
	bool debug;
	bool enable_opengl;
} EgWindowsWindowCreateInfo;

typedef void *(*EgWindowsWindowCallback)(void *user_data);

typedef struct
{
	void        *gl_context;
	char const  *gl_version;
	char const  *glsl_version;
	ecs_query_t *inner_systems;
} EgWindowsOpenGLContext;

typedef struct
{
	int dummy;
} EgWindowsOpenGLContextCreate;

typedef struct
{
	int32_t dummy;
} EgWindowsMouse;

extern ECS_COMPONENT_DECLARE(EgWindowsWindow);
extern ECS_COMPONENT_DECLARE(EgWindowsWindowState);
extern ECS_COMPONENT_DECLARE(EgWindowsInputState);
extern ECS_COMPONENT_DECLARE(EgWindowsGlBackend);
extern ECS_COMPONENT_DECLARE(EgWindowsFrameCallback);
extern ECS_COMPONENT_DECLARE(EgWindowsWindowCreateInfo);
extern ECS_COMPONENT_DECLARE(EgWindowsOpenGLContext);
extern ECS_COMPONENT_DECLARE(EgWindowsOpenGLContextCreate);
extern ECS_COMPONENT_DECLARE(EgWindowsMouse);
extern ECS_TAG_DECLARE(EgWindowsEventResize);
extern ECS_TAG_DECLARE(EgWindowsCloseRequest);
extern ECS_DECLARE(EgWindows);

void EgWindowsImport(ecs_world_t *world);
