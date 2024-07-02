#include <flecs.h>

typedef struct {
	float d;
} MyIntersectorsHit;

extern ECS_COMPONENT_DECLARE(MyIntersectorsHit);


void MyIntersectorsImport(ecs_world_t *world);