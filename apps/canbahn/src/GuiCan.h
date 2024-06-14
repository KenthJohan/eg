#include <flecs.h>


typedef struct {
	int32_t index;
	ecs_entity_t parent;
	ecs_entity_t representation;
} GuiCanTableRow;

typedef struct {
	ecs_vec_t v;
	uint32_t last_index;
	float min;
	float max;
} GuiCanPlot;

extern ECS_COMPONENT_DECLARE(GuiCanTableRow);
extern ECS_COMPONENT_DECLARE(GuiCanPlot);

void GuiCanImport(ecs_world_t *world);