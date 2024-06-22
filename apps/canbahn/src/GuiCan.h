#include <flecs.h>



typedef struct {
	int32_t index;
	ecs_entity_t kind;
} GuiCanTableRow;

typedef struct {
	ecs_vec_t v;
	uint32_t last_index;
	float min;
	float max;
} GuiCanPlot;

extern ECS_COMPONENT_DECLARE(GuiCanTableRow);
extern ECS_COMPONENT_DECLARE(GuiCanPlot);
extern ECS_TAG_DECLARE(GuiCanSelected);

void GuiCanImport(ecs_world_t *world);