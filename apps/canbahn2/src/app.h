#pragma once
#include <flecs.h>
#include <stdint.h>


typedef struct {
	void * data;
	uint32_t size;
	uint32_t width;
	uint32_t height;
	uint8_t depth;
} image_t;


typedef struct {
	ecs_world_t *world;
	ecs_query_t *query_canids;
	ecs_query_t *query_signals;
	ecs_query_t *query_gui;
	ecs_query_t *query_ifaces;
	ecs_query_t *query_plots;
	ecs_query_t *query_exporter;
	ecs_query_t *query_exporter2;
	double gui_time_seconds;
	uint64_t last_time;

	bool show_window_main;
	bool show_window_extra1;
	bool show_window_extra2;



	uint32_t image_id;
	uint32_t image_id2;
	image_t image;
} app_t;