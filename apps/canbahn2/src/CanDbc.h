#pragma once
#include <stdbool.h>
#include <flecs.h>

typedef struct {
	char name[128];
	int type;
	int order;
	int mode;
	int start;
	int length;
	double factor;
	double offset;
	double min;
	double max;
	char unit[128];
} CanDbcSignal;

typedef uint32_t CanDbcFlags;
#define CanDbcFlagsBacon 0x1
#define CanDbcFlagsLettuce 0x2
#define CanDbcFlagsTomato 0x4

extern ECS_COMPONENT_DECLARE(CanDbcSignal);
extern ECS_COMPONENT_DECLARE(CanDbcFlags);

int dbcsig_meta_bitpos_to_signal(CanDbcSignal meta[], int length, int bitpos);

void CanDbcImport(ecs_world_t *world);