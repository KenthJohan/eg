#include <gs/gs.h>
#include <gs/util/gs_idraw.h>
#include <gs/util/gs_gui.h>
#include <gs_ddt/gs_ddt.h>
#include <flecs.h>
#include "EgQuantities.h"

typedef struct app_t
{
	ecs_world_t *world;
	gs_command_buffer_t cb;
	gs_immediate_draw_t gsi;
	gs_gui_context_t gui;
} app_t;

static int bg, window = 1, embeded, summons;

static void toggle_bg(int argc, char **argv);
static void toggle_window(int argc, char **argv);
static void toggle_embedded(int argc, char **argv);
static void help(int argc, char **argv);
static void echo(int argc, char **argv);
static void spam(int argc, char **argv);
void summon(int argc, char **argv);
void sz(int argc, char **argv);

gs_ddt_command_t commands[] = {
	{
		.func = echo,
		.name = "echo",
		.desc = "repeat what was entered!!!",
	},
	{
		.func = spam,
		.name = "spam",
		.desc = "send the word arg1, arg2 amount of times, HURRY UP!!!",
	},
	{
		.func = help,
		.name = "help",
		.desc = "sends a list of commands",
	},
	{
		.func = toggle_bg,
		.name = "bg",
		.desc = "toggles background",
	},
	{
		.func = toggle_window,
		.name = "window",
		.desc = "toggles gui window",
	},
	{
		.func = toggle_embedded,
		.name = "embed",
		.desc = "places the ddt inside the window",
	},
	{
		.func = summon,
		.name = "summon",
		.desc = "summons a gui window",
	},
	{
		.func = sz,
		.name = "sz",
		.desc = "change ddt size",
	},
};

gs_ddt_t ddt = {
	.tb = "",
	.cb = "",
	.commands = commands,
	.commands_len = gs_array_size(commands),
	.size = 0.4,
	.open_speed = 0.2,
	.close_speed = 0.3,
	.autoscroll = 1,
};

void toggle_bg(int argc, char **argv)
{
	gs_ddt_printf(&ddt, "Background turned %s\n", (bg = !bg) ? "on" : "off");
}

void sz(int argc, char **argv)
{
	if (argc != 2)
	{
		gs_ddt_printf(&ddt, "[sz]: needs 1 argument!\n");
		return;
	}
	float sz = atof(argv[1]);
	if (sz > 1 || sz < 0)
	{
		gs_ddt_printf(&ddt, "[sz]: number needs to be between (0, 1)");
		return;
	}
	ddt.size = sz;

	gs_ddt_printf(&ddt, "ddt size is now %f\n", sz);
}

void toggle_window(int argc, char **argv)
{
	if (window && embeded)
		gs_ddt_printf(&ddt, "Unable to turn off window, ddt is embeded!\n");
	else
		gs_ddt_printf(&ddt, "GUI Window turned %s\n", (window = !window) ? "on" : "off");
}

void toggle_embedded(int argc, char **argv)
{
	if (!window && !embeded)
		gs_ddt_printf(&ddt, "Unable to embed into window, open window first!\n");
	else
		gs_ddt_printf(&ddt, "ddt embedded turned %s\n", (embeded = !embeded) ? "on" : "off");
}

void summon(int argc, char **argv)
{
	gs_ddt_printf(&ddt, "A summoner has cast his spell! A window has appeared!!!!\n");
	summons++;
}

void spam(int argc, char **argv)
{
	if (argc != 3)
		goto spam_invalid_command;
	int count = atoi(argv[2]);
	if (!count)
		goto spam_invalid_command;
	while (count--)
		gs_ddt_printf(&ddt, "%s\n", argv[1]);
	return;
spam_invalid_command:
	gs_ddt_printf(&ddt, "[spam]: invalid usage. It should be 'spam word [int count]''\n");
}

void echo(int argc, char **argv)
{
	for (int i = 1; i < argc; i++)
		gs_ddt_printf(&ddt, "%s ", argv[i]);
	gs_ddt_printf(&ddt, "\n");
}

void help(int argc, char **argv)
{
	for (int i = 0; i < gs_array_size(commands); i++)
	{
		if (commands[i].name)
			gs_ddt_printf(&ddt, "* Command: %s\n", commands[i].name);
		if (commands[i].desc)
			gs_ddt_printf(&ddt, "- desc: %s\n", commands[i].desc);
	}
}







void Draw_Rectangle(ecs_iter_t* it)
{
	gs_immediate_draw_t* gsi = it->ctx;
    const EgV2F32 *p  = ecs_field(it, EgV2F32, 1);
    const EgV2F32 *r  = ecs_field(it, EgV2F32, 2);
    const EgV4U8 *c  = ecs_field(it, EgV4U8, 3);
    for (int i = 0; i < it->count; i ++)
	{
		gs_vec2 xy = {p[i].x, p[i].y};
		gs_vec2 wh = {r[i].x, r[i].y};
		gs_color_t color = {c[i].x, c[i].y, c[i].z, c[i].w};
        gsi_rectvd(gsi, xy, wh, gs_v2(0.f, 0.f), gs_v2(1.f, 1.f), color, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
    }
}

void Draw_Rectangle1(ecs_iter_t* it)
{
	gs_immediate_draw_t* gsi = it->ctx;
    const EgV2F32 *p  = ecs_field(it, EgV2F32, 1);
    const EgV2F32 *r  = ecs_field(it, EgV2F32, 2);
    const EgV4U8 *c  = ecs_field(it, EgV4U8, 3);
    for (int i = 0; i < it->count; i ++)
	{
		gs_vec2 xy = {p[i].x, p[i].y};
		gs_vec2 wh = {r[i].x, r[i].y};
		gs_color_t color = {255, 255, 255, 255};
        gsi_rectvd(gsi, xy, wh, gs_v2(0.f, 0.f), gs_v2(1.f, 1.f), color, GS_GRAPHICS_PRIMITIVE_TRIANGLES);
    }
}


void Update_Mouse(ecs_iter_t* it)
{
    EgV2F32 *p  = ecs_field(it, EgV2F32, 1);
	gs_vec2 mouse_pos = gs_platform_mouse_positionv();
	p[0].x = mouse_pos.x;
	p[0].y = mouse_pos.y;
}














void app_init()
{
	app_t *app = gs_user_data(app_t);
	app->cb = gs_command_buffer_new();
	app->gsi = gs_immediate_draw_new(gs_platform_main_window());
	gs_gui_init(&app->gui, gs_platform_main_window());



    ecs_entity_t e_Draw_Rectangle = ecs_system(app->world, {
        .entity = ecs_entity(app->world, {
			.name = "Draw_Rectangle",
			.add = { ecs_dependson(EcsOnUpdate) }
		}),
        .query.filter.terms = {
            { .id = ecs_pair(ecs_id(EgV2F32), EgPosition), .inout = EcsIn },
            { .id = ecs_pair(ecs_id(EgV2F32), EgRectangle), .inout = EcsIn },
            { .id = ecs_pair(ecs_id(EgV4U8), EgColor), .inout = EcsIn },
            { .id = EgHover1, .oper = EcsNot },
        },
        .callback = Draw_Rectangle,
		.ctx = &app->gsi
    });

    ecs_entity_t e_Draw_Rectangle1 = ecs_system(app->world, {
        .entity = ecs_entity(app->world, {
			.name = "Draw_Rectangle1",
			.add = { ecs_dependson(EcsOnUpdate) }
		}),
        .query.filter.terms = {
            { .id = ecs_pair(ecs_id(EgV2F32), EgPosition), .inout = EcsIn },
            { .id = ecs_pair(ecs_id(EgV2F32), EgRectangle), .inout = EcsIn },
            { .id = ecs_pair(ecs_id(EgV4U8), EgColor), .inout = EcsIn },
            { .id = EgHover1 },
        },
        .callback = Draw_Rectangle1,
		.ctx = &app->gsi
    });


    ecs_entity_t e_Update_Mouse = ecs_system(app->world, {
        .entity = ecs_entity(app->world, {
			.name = "Update_Mouse",
			.add = { ecs_dependson(EcsOnUpdate) }
		}),
        .query.filter.terms = {
            { .id = ecs_pair(ecs_id(EgV2F32), EgPosition), .src.id = ecs_id(EgMouse) }
        },
        .callback = Update_Mouse
    });


}

void app_update()
{
	app_t *app = gs_user_data(app_t);

	gs_command_buffer_t *cb = &app->cb;
	gs_immediate_draw_t *gsi = &app->gsi;
	gs_gui_context_t *gui = &app->gui;

	gs_vec2 fbs = gs_platform_framebuffer_sizev(gs_platform_main_window());
	const float t = gs_platform_elapsed_time() * 0.0001f;

	if (gs_platform_key_pressed(GS_KEYCODE_ESC))
	{
		ddt.open = !ddt.open;
	}
	else if (gs_platform_key_pressed(GS_KEYCODE_TAB) && ddt.open)
	{
		ddt.autoscroll = !ddt.autoscroll;
	}

	if (bg)
	{
		gsi_camera3D(gsi, fbs.x, fbs.y);
		gsi_rotatev(gsi, gs_deg2rad(90.f), GS_ZAXIS);
		gsi_rotatev(gsi, t, GS_YAXIS);
		gsi_sphere(gsi, 0.f, 0.f, 0.f, 1.f, 50, 150, 200, 50, GS_GRAPHICS_PRIMITIVE_LINES);
	}
	gsi_camera2D(gsi, fbs.x, fbs.y);
	
	ecs_progress(app->world, 0);
	gsi_text(gsi, fbs.x * 0.5f - 198.f, fbs.y * 0.5f, "ESC to open term, TAB toggle autoscroll, help for more", NULL, false, 255, 255, 255, 255);
	gsi_renderpass_submit(gsi, cb, gs_v4(0.f, 0.f, fbs.x, fbs.y), gs_color(10, 10, 10, 255));

	// Render gui
	gs_gui_begin(gui, NULL);

	gs_gui_layout_t l;
	if (window && gs_gui_window_begin(gui, "App", gs_gui_rect(100, 100, 200, 200)))
	{
		l = *gs_gui_get_layout(gui);
		gs_gui_layout_row(gui, 1, (int[]){-1}, 0);
		gs_gui_text(gui, "Hello, Gunslinger.");
		gs_gui_window_end(gui);
	}

	int s = summons;
	while (s--)
	{
		gs_gui_push_id(gui, &s, sizeof(s));
		if (gs_gui_window_begin(gui, "Summon", gs_gui_rect(100, 100, 200, 200)))
		{
			gs_gui_layout_row(gui, 1, (int[]){-1}, 0);
			gs_gui_text(gui, "At your Service!");
			gs_gui_window_end(gui);
		}
		gs_gui_pop_id(gui);
	}

	gs_vec2 fb = gui->framebuffer_size;
	gs_gui_rect_t screen;
	if (embeded)
		screen = l.body;
	else
		screen = gs_gui_rect(0, 0, fb.x, fb.y);
	gs_ddt(&ddt, gui, screen, NULL);

	gs_gui_end(gui);
	gs_gui_render(gui, cb);

	// Submit command buffer for GPU
	gs_graphics_command_buffer_submit(cb);
}

void app_shutdown()
{
	app_t *app = gs_user_data(app_t);
	gs_immediate_draw_free(&app->gsi);
	gs_command_buffer_free(&app->cb);
	gs_gui_free(&app->gui);
}

gs_app_desc_t gs_main(int32_t argc, char **argv)
{
	app_t *app = gs_malloc_init(app_t);
	app->world = ecs_init();
	ECS_IMPORT(app->world, EgQuantities);
	// https://www.flecs.dev/explorer/?remote=true
	ecs_set(app->world, EcsWorld, EcsRest, {.port = 0});
	//ecs_add_pair(app->world, ecs_pair(ecs_id(EgV2F32), EgMousePosition), ecs_id(EgV2F32), EgMousePosition);
	ecs_add_id(app->world, EgMouse, ecs_pair(ecs_id(EgV2F32), EgPosition));
	//ecs_add_id(app->world, ecs_pair(ecs_id(EgV2F32), EgMousePosition), ecs_pair(ecs_id(EgV2F32), EgMousePosition));
	ecs_plecs_from_file(app->world, "test.flecs");


	return (gs_app_desc_t){
		.user_data = app,
		.window.width = 800,
		.window.height = 600,
		.init = app_init,
		.update = app_update,
		.shutdown = app_shutdown};
}
