#include <flecs.h>


typedef struct {
	int32_t value;
	int32_t list_index;
} GuiSlider;

typedef struct {
	int32_t value;
} GuiCanSignalInfo;

extern ECS_COMPONENT_DECLARE(GuiSlider);
extern ECS_COMPONENT_DECLARE(GuiCanSignalInfo);

void gui_can_progress1(ecs_world_t *world, ecs_query_t *q);

void GuiCanImport(ecs_world_t *world);