#include <flecs.h>
#include <stdio.h>

// Shows how to match a component on the direct parent only.
//
// `.src.id = EcsUp` with `.trav = EcsChildOf` matches the component on ANY ancestor.
// Using a variable as the source and tying it to the entity with (ChildOf, $parent)
// restricts the match to the first parent.

ECS_COMPONENT_DECLARE(Marker);
typedef struct {
	int dummy;
} Marker;

static void PrintAnyAncestor(ecs_iter_t *it)
{
	for (int i = 0; i < it->count; ++i) {
		printf("  any ancestor: %s\n", ecs_get_name(it->world, it->entities[i]));
	}
}

static void PrintDirectParent(ecs_iter_t *it)
{
	for (int i = 0; i < it->count; ++i) {
		printf("  direct parent: %s\n", ecs_get_name(it->world, it->entities[i]));
	}
}

int main(int argc, char *argv[])
{
	ecs_world_t *world = ecs_init_w_args(argc, argv);

	ECS_COMPONENT_DEFINE(world, Marker);
	ECS_TAG(world, Child);

	// root (Marker) -> mid -> leaf. `leaf` has a Marker ancestor but not a Marker parent.
	ecs_entity_t root = ecs_entity(world, {.name = "root"});
	ecs_set(world, root, Marker, {0});
	ecs_entity_t mid  = ecs_entity(world, {.name = "mid", .parent = root});
	ecs_entity_t leaf = ecs_entity(world, {.name = "leaf", .parent = mid});
	ecs_add(world, mid, Child);
	ecs_add(world, leaf, Child);

	ecs_query_t *any = ecs_query(world,
	{.terms = {
	 {.id = Child},
	 {.id = ecs_id(Marker), .trav = EcsChildOf, .src.id = EcsUp},
	 }});

	ecs_query_t *direct = ecs_query(world,
	{.terms = {
	 {.id = Child},
	 {.id = ecs_id(Marker), .src = {.id = EcsIsVariable, .name = "parent"}},
	 {.first.id = EcsChildOf, .second = {.id = EcsIsVariable, .name = "parent"}},
	 }});

	printf("EcsUp query (expects mid and leaf):\n");
	ecs_iter_t it = ecs_query_iter(world, any);
	while (ecs_query_next(&it)) {
		PrintAnyAncestor(&it);
	}

	printf("Variable query (expects only mid):\n");
	it = ecs_query_iter(world, direct);
	while (ecs_query_next(&it)) {
		PrintDirectParent(&it);
	}

	ecs_query_fini(any);
	ecs_query_fini(direct);
	ecs_fini(world);
	return 0;
}
