#pragma once
#include <flecs.h>


typedef struct {
	char prefix[128];
	char export_destination_c[128];
	char export_destination_python[128];
} gui_exporter_t;

void gui_exporter_progress(ecs_world_t *world, ecs_query_t *q, gui_exporter_t * exporter, bool *p_open);
ecs_query_t * gui_exporter_query(ecs_world_t *world);