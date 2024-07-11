#include <sokol_app.h>
#include <sokol_gfx.h>
#include <sokol_log.h>
#include <sokol_debugtext.h>
#include <sokol_glue.h>
#include <sokol_gl.h>
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>
#include <sokol_imgui.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <float.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>

#include <flecs.h>
#include <egcameras.h>
#include <egbase.h>
#include <egspatials.h>
#include <egsokol.h>
#include <egwindows.h>

#include "MiscShapes.h"
#include "MiscLines.h"
#include "MiscPoints.h"
#include "MyController.h"
#include "MyIntersectors.h"

#include "argparse.h"


#define IM_COL32(r,g,b,a) ((r) << 0 | (g) << 8 | (b) << 16 | (a) << 24)

//https://github.com/lyte2d/lyte2d/blob/2110025f51246c82a15b42d2dee0842f639515b5/deps/sokol/sokol_gfx_ext.h#L86
static void _sg_gl_query_pixels(int x, int y, int w, int h, bool origin_top_left, void *pixels) {
	/*
    (void)x; (void)y; (void)w; (void)h; (void)origin_top_left; (void)pixels;
#ifdef _WIN32
    printf("This API is not yet supported on Windows.\n");
    exit(1);
#else
    SOKOL_ASSERT(pixels);
    GLuint gl_fb;
    GLint dims[4];
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&gl_fb);
    _SG_GL_CHECK_ERROR();
    glGetIntegerv(GL_VIEWPORT, dims);
    int cur_height = dims[3];
    y = origin_top_left ? (cur_height - (y+h)) : y;
    _SG_GL_CHECK_ERROR();
#if defined(SOKOL_GLES2) // use NV extension instead
    glReadBufferNV(gl_fb == 0 ? GL_BACK : GL_COLOR_ATTACHMENT0);
#else
    glReadBuffer(gl_fb == 0 ? GL_BACK : GL_COLOR_ATTACHMENT0);
#endif
    _SG_GL_CHECK_ERROR();
    glReadPixels(x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    _SG_GL_CHECK_ERROR();
#endif
*/
}

static void WindowLastFrame(ecs_iter_t *it)
{
	Window *window = ecs_field(it, Window, 1);
	window->mouse_left_edge = 0;
	window->mouse_right_edge = 0;
	memset(window->keys_edge, 0, sizeof(uint8_t) * 512);
}

typedef struct {
	ecs_world_t *world;
} app_t;

#define OFFSCREEN_WIDTH (512)
#define OFFSCREEN_HEIGHT (512)
#define OFFSCREEN_COLOR_FORMAT (SG_PIXELFORMAT_RGBA8)
// #define OFFSCREEN_DEPTH_FORMAT (SG_PIXELFORMAT_DEPTH)
#define OFFSCREEN_DEPTH_FORMAT SG_PIXELFORMAT_DEPTH_STENCIL
#define OFFSCREEN_SAMPLE_COUNT (1)
// #define OFFSCREEN_SAMPLE_COUNT (4)

static sg_attachments global_offscreen_attachments;
static simgui_image_t global_simg;


static void init_cb(app_t *app)
{
	ecs_world_t *world = app->world;

	sg_setup(&(sg_desc){
	.environment = sglue_environment(),
	.logger.func = slog_func,
	});

	sgl_setup(&(sgl_desc_t){
	.logger.func = slog_func,
	});

	simgui_desc_t simgui_desc = {0};
	simgui_desc.no_default_font = true;
	simgui_desc.logger.func = slog_func;
	simgui_setup(&simgui_desc);

	// setup sokol-debugtext
	sdtx_setup(&(sdtx_desc_t){
	.fonts[0] = sdtx_font_z1013(),
	.logger.func = slog_func,
	});

	{
		sg_image img_col = sg_make_image(&(sg_image_desc){
		.render_target = true,
		.width = OFFSCREEN_WIDTH,
		.height = OFFSCREEN_HEIGHT,
		.pixel_format = OFFSCREEN_COLOR_FORMAT,
		.sample_count = OFFSCREEN_SAMPLE_COUNT,
		});
		sg_image img_depth = sg_make_image(&(sg_image_desc){
		.render_target = true,
		.width = OFFSCREEN_WIDTH,
		.height = OFFSCREEN_HEIGHT,
		.pixel_format = OFFSCREEN_DEPTH_FORMAT,
		.sample_count = OFFSCREEN_SAMPLE_COUNT,
		});
		global_simg = simgui_make_image(&(simgui_image_desc_t){
		.image = img_col,
		.sampler = sg_make_sampler(&(sg_sampler_desc){
		.min_filter = SG_FILTER_NEAREST,
		.mag_filter = SG_FILTER_NEAREST,
		.wrap_u = SG_WRAP_CLAMP_TO_EDGE,
		.wrap_v = SG_WRAP_CLAMP_TO_EDGE,
		})});
		global_offscreen_attachments = sg_make_attachments(&(sg_attachments_desc){
		.colors[0].image = img_col,
		.depth_stencil.image = img_depth,
		});
	}

	ECS_IMPORT(world, EgWindows);
	ECS_IMPORT(world, EgCameras);
	ECS_IMPORT(world, EgBase);
	ECS_IMPORT(world, EgSpatials);
	ECS_IMPORT(world, MiscShapes);
	ECS_IMPORT(world, MiscLines);
	ECS_IMPORT(world, MiscPoints);
	ECS_IMPORT(world, Sg);
	ECS_IMPORT(world, MyController);
	ECS_IMPORT(world, MyIntersectors);





	ecs_log_set_level(1);
	ecs_plecs_from_file(app->world, "config/keycode_sokol.flecs");
	ecs_plecs_from_file(app->world, "config/graphics_attributes.flecs");
	ecs_plecs_from_file(app->world, "config/graphics_pipes.flecs");
	ecs_plecs_from_file(app->world, "config/graphics_shaders.flecs");
	ecs_plecs_from_file(app->world, "config/graphics_ubs.flecs");
	ecs_plecs_from_file(app->world, "config/app.flecs");
	ecs_log_set_level(-1);

	// https://www.flecs.dev/explorer/?remote=true
	ecs_set(world, EcsWorld, EcsRest, {.port = 0});
	printf("https://www.flecs.dev/explorer/?remote=true\n");

	ECS_SYSTEM(world, WindowLastFrame, EcsPostUpdate, Window($));

	ecs_singleton_set(app->world, Window, {.w = 0, .h = 0});
}

static void frame_cb(app_t *app)
{
	{
		float dt = sapp_frame_duration();
		float w = sapp_widthf();
		float h = sapp_heightf();
		Window *window = ecs_get_mut(app->world, ecs_id(Window), Window);
		window->w = w;
		window->h = h;
		window->dt = dt;
	}

	{
		simgui_new_frame(&(simgui_frame_desc_t){
		.width = sapp_width(),
		.height = sapp_height(),
		.delta_time = sapp_frame_duration(),
		.dpi_scale = sapp_dpi_scale(),
		});
#ifdef IMGUI_HAS_VIEWPORT
		ImGuiViewport *viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->GetWorkPos());
		ImGui::SetNextWindowSize(viewport->GetWorkSize());
		ImGui::SetNextWindowViewport(viewport->ID);
#else
		igSetNextWindowPos((ImVec2){0.0f, 0.0f}, 0, (ImVec2){0.0f, 0.0f});
		igSetNextWindowSize(igGetIO()->DisplaySize, 0);
#endif
		bool show_app_main_menu_bar;
		bool show_app_console;
		bool show_app_console123;
		Window *window = ecs_get_mut(app->world, ecs_id(Window), Window);
		// igPushStyleVar_Float(ImGuiStyleVar_WindowRounding, 0.0f);
		// igPushItemWidth(igGetFontSize() * -12);
		if (igBegin("demo_flecs_sokol", 0, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDecoration)) {
			// igText("dear imgui says hello!");
			if (igBeginMenuBar()) {
				if (igBeginMenu("Menu", true)) {
					igMenuItem_BoolPtr("Menu1", NULL, &show_app_main_menu_bar, true);
					igSeparatorText("Separator");
					igMenuItem_BoolPtr("Menu2", NULL, &show_app_console, true);
					igEndMenu();
				}
				if (igBeginMenu("Examples", true)) {
					igMenuItem_BoolPtr("Menu3", NULL, &show_app_main_menu_bar, true);
					igEndMenu();
				}
				if (igBeginMenu("Tools", true)) {
					igMenuItem_BoolPtr("Menu4", NULL, &show_app_main_menu_bar, true);
					igEndMenu();
				}

				char buf[128];
				snprintf(buf, sizeof(buf), "[%+5.3f %+5.3f %+5.3f]", window->pos[0], window->pos[1], window->pos[2]);
				if (igMenuItem_BoolPtr(buf, NULL, &show_app_console123, true)) {
				}
				snprintf(buf, sizeof(buf), "[%5.3fms %5.3fHz]", window->dt*1000.0f, window->fps);
				if (igMenuItem_BoolPtr(buf, NULL, &show_app_console123, true)) {
				}
				snprintf(buf, sizeof(buf), "[%+5.3f %+5.3f]", window->mouse_x, window->mouse_y);
				if (igMenuItem_BoolPtr(buf, NULL, &show_app_console123, true)) {
				}
				snprintf(buf, sizeof(buf), "[%+5.3f %+5.3f]", window->canvas_mouse_x, window->canvas_mouse_y);
				if (igMenuItem_BoolPtr(buf, NULL, &show_app_console123, true)) {
				}

				igEndMenuBar();
			}

			//igSetNextWindowPos((ImVec2){0, 0}, ImGuiCond_Once, (ImVec2){0, 0});
			//igSetNextWindowSize((ImVec2){OFFSCREEN_WIDTH, OFFSCREEN_HEIGHT}, ImGuiCond_Once);


			{
				ImVec2 vMin;
				igGetWindowContentRegionMin(&vMin);
				ImVec2 vMax;
				igGetWindowContentRegionMax(&vMax);
				ImVec2 p;
				igGetWindowPos(&p);
				ImVec2 size;
				igGetWindowSize(&size);
				ImVec2 c[2] = {{vMin.x + p.x, vMin.y + p.y},{vMax.x + p.x, vMax.y + p.y}};
				ImDrawList * dl = igGetForegroundDrawList_Nil();
				ImDrawList_AddRect(dl, c[0], c[1], IM_COL32( 255, 255, 0, 255 ), 0, 0, 1);
				ImVec2 d = {vMax.x - vMin.x, vMax.y - vMin.y};
				window->canvas_mouse_x = (window->mouse_x - vMin.x) * OFFSCREEN_WIDTH / d.x;
				window->canvas_mouse_y = (window->mouse_y - vMin.y) * OFFSCREEN_HEIGHT / d.y;
				window->canvas_width = OFFSCREEN_WIDTH;
				window->canvas_height = OFFSCREEN_HEIGHT;
				igImage(simgui_imtextureid(global_simg), d, (ImVec2){0, 1}, (ImVec2){1, 0}, (ImVec4){1, 1, 1, 1}, (ImVec4){0, 0, 0, 0});
			}



		}
		igEnd();
		// igPopStyleVar(1);
		// igPopItemWidth();
	}

	{
		sg_color_attachment_action color0 = {.load_action = SG_LOADACTION_CLEAR, .clear_value = {0.1f, 0.1f, 0.1f, 1.0f}};
		sg_pass_action action = {.colors[0] = color0};
		sg_pass pass = {.action = action, .attachments = global_offscreen_attachments};
		sg_begin_pass(&pass);
		ecs_progress(app->world, 0.0f);
		sg_end_pass();
	}

	{
		// then the display pass with the Dear ImGui scene
		sg_color_attachment_action color0 = {.load_action = SG_LOADACTION_CLEAR, .clear_value = {0.5f, 0.5f, 1.0f, 1.0f}};
		sg_pass_action action = {.colors[0] = color0};
		sg_begin_pass(&(sg_pass){.action = action, .swapchain = sglue_swapchain()});
		simgui_render();
		// sdtx_draw();
		sg_end_pass();
	}

	/*
	{
	    sg_color_attachment_action color0 = {.load_action = SG_LOADACTION_DONTCARE,.clear_value = {0.1f, 0.1f, 0.1f, 1.0f}};
	    sg_pass_action action = {.colors[0] = color0};
	    sg_pass pass = {.action = action, .swapchain = sglue_swapchain()};
	    sg_begin_pass(&pass);
	    //sdtx_draw();
	    //__dbgui_draw();
	    sg_end_pass();
	}
	*/

	sg_commit();
}

// https://github.com/floooh/sokol/blob/fa3d7cbe9ca85b8b87824ac366c724cb0e33a6af/util/sokol_nuklear.h#L2567

static void event_cb(const sapp_event *evt, app_t *app)
{
	simgui_handle_event(evt);
	Window *window = ecs_get_mut(app->world, ecs_id(Window), Window);
	uint8_t *keys = window->keys;
	uint8_t *keyse = window->keys_edge;

	switch (evt->type) {
	case SAPP_EVENTTYPE_MOUSE_DOWN:
		if (evt->mouse_button == SAPP_MOUSEBUTTON_LEFT) {
			if (window->mouse_left == 0) {
				window->mouse_left_edge = 1;
			}
			window->mouse_left = 1;
		}

		if (evt->mouse_button == SAPP_MOUSEBUTTON_RIGHT) {
		}
		break;
	case SAPP_EVENTTYPE_MOUSE_UP:
		if (evt->mouse_button == SAPP_MOUSEBUTTON_LEFT) {
			window->mouse_left = 0;
		}

		if (evt->mouse_button == SAPP_MOUSEBUTTON_RIGHT) {
		}
		break;
	case SAPP_EVENTTYPE_MOUSE_SCROLL:
		break;
	case SAPP_EVENTTYPE_MOUSE_MOVE:
		window->mouse_x = evt->mouse_x;
		window->mouse_y = evt->mouse_y;
		window->mouse_dx = evt->mouse_dx;
		window->mouse_dy = evt->mouse_dy;
		break;
	case SAPP_EVENTTYPE_KEY_UP:
		assert(evt->key_code < 512);
		keys[evt->key_code] = 0;
		break;
	case SAPP_EVENTTYPE_KEY_DOWN:
		assert(evt->key_code < 512);
		if (keys[evt->key_code] == 0) {
			keyse[evt->key_code] = 1;
		}
		keys[evt->key_code] = 1;
		break;
	case SAPP_EVENTTYPE_RESIZED: {
		break;
	}
	default:
		break;
	}

	if (keys[SAPP_KEYCODE_ESCAPE]) {
		sapp_quit();
	}
}

static void cleanup_cb(app_t *app)
{
	sdtx_shutdown();
	sgl_shutdown();
	simgui_shutdown();
	sg_shutdown();
	ecs_fini(app->world);
	free(app);
}

sapp_desc sokol_main(int argc, char /*const*/ *argv[])
{

	app_t *app = calloc(1, sizeof(app_t));
	app->world = ecs_init();
	// Disables warnings
	// Disables warnings about flecs HTTP request to timeout.
	ecs_log_set_level(-3);

	return (sapp_desc){
	.user_data = app,
	.init_userdata_cb = (void (*)(void *))init_cb,
	.frame_userdata_cb = (void (*)(void *))frame_cb,
	.cleanup_userdata_cb = (void (*)(void *))cleanup_cb,
	.event_userdata_cb = (void (*)(const sapp_event *, void *))event_cb,
	.width = 300,
	.height = 200,
	.sample_count = 4,
	.window_title = "demo_flecs_sokol",
	.icon.sokol_default = true,
	.logger.func = slog_func,
	};
}
