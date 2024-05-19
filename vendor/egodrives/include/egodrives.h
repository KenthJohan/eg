#pragma once



#include <flecs.h>
#include <stdint.h>
#include <egquantities.h>

typedef struct {
	uint32_t nodeid;
	uint32_t cmd;
} EgOdrivesCmd;


typedef enum {
	EG_ODRIVE_INPUT_MODE_INACTIVE = 0,
	EG_ODRIVE_INPUT_MODE_PASSTHROUGH = 1,
	EG_ODRIVE_INPUT_MODE_VEL_RAMP = 2,
	EG_ODRIVE_INPUT_MODE_POS_FILTER = 3,
	EG_ODRIVE_INPUT_MODE_MIX_CHANNELS = 4,
	EG_ODRIVE_INPUT_MODE_TRAP_TRAJ = 5,
	EG_ODRIVE_INPUT_MODE_TORQUE_RAMP = 6,
	EG_ODRIVE_INPUT_MODE_MIRROR = 7,
	EG_ODRIVE_INPUT_MODE_TUNING = 8,
} eg_odrive_input_mode_t;



extern ECS_COMPONENT_DECLARE(eg_odrive_input_mode_t);
extern ECS_COMPONENT_DECLARE(EgOdrivesCmd);

void EgOdrivesImport(ecs_world_t *world);








