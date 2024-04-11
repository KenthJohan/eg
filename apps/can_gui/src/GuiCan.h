#include <flecs.h>
#include "Can.h"
#include "microui.h"


typedef struct {
	mu_Context *muctx;
} GuiContext;

typedef struct {
	uint8_t value;
} GuiSlider;

extern ECS_COMPONENT_DECLARE(GuiContext);
extern ECS_COMPONENT_DECLARE(GuiSlider);


void GuiCanImport(ecs_world_t *world);