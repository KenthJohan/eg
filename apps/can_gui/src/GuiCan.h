#include <flecs.h>

#include "microui.h"

void GuiCanImport(ecs_world_t *world);


typedef struct {
	mu_Context * muctx;
} GuiContext;

typedef struct {
	uint8_t value;
} GuiSlider;



typedef struct {
	uint8_t value;
	uint8_t type;
	uint8_t order;
	uint8_t mode;
	uint8_t start;
	uint8_t length;
	float factor;
	uint8_t offset;
	uint8_t min;
} CanSignal;

extern ECS_COMPONENT_DECLARE(GuiContext);
extern ECS_COMPONENT_DECLARE(GuiSlider);