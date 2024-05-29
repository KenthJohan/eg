#include <flecs.h>


typedef struct {
	int32_t value;
	int32_t list_index;
} GuiCanSlider;

typedef struct {
	ecs_vec_t v;
	uint32_t last_index;
	float min;
	float max;
} GuiCanPlot;

extern ECS_COMPONENT_DECLARE(GuiCanSlider);
extern ECS_COMPONENT_DECLARE(GuiCanPlot);

void GuiCanImport(ecs_world_t *world);