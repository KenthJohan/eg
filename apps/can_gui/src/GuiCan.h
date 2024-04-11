#include <flecs.h>

#include "microui.h"

void GuiCanImport(ecs_world_t *world);


typedef struct {
	mu_Context * muctx;
} GuiContext;

typedef struct {
	uint8_t value;
} GuiSlider;

extern ECS_COMPONENT_DECLARE(GuiContext);
extern ECS_COMPONENT_DECLARE(GuiSlider);