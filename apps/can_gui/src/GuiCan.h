#include <flecs.h>
#include "Can.h"
#include "microui.h"


typedef struct {
	mu_Context *muctx;
} GuiContext;

typedef struct {
	uint8_t value;
	uint8_t list_index;
} GuiSlider;

typedef struct {
	uint8_t value;
} GuiCanSignalInfo;

extern ECS_COMPONENT_DECLARE(GuiContext);
extern ECS_COMPONENT_DECLARE(GuiSlider);
extern ECS_COMPONENT_DECLARE(GuiCanSignalInfo);


void GuiCanImport(ecs_world_t *world);