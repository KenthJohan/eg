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

static void PrintQuery(ecs_iter_t *it)
{
	const char *source = ecs_get_name(it->world, ecs_field_src(it, 1));
	printf("# %s\n", source);
	for (int i = 0; i < it->count; ++i) {
		printf("  - %s\n", ecs_get_name(it->world, it->entities[i]));
	}
}

static void PrintMatches(ecs_world_t *world, ecs_entity_t child, const char *any_label, const char *direct_label)
{
	ecs_query_t *any = ecs_query(world,
	{.terms = {
	 {.id = child},
	 {.id = ecs_id(Marker), .trav = EcsChildOf, .src.id = EcsUp},
	 }});

	ecs_query_t *direct = ecs_query(world,
	{.terms = {
	 {.id = child},
	 {.id = ecs_id(Marker), .src = {.id = EcsIsVariable, .name = "parent"}},
	 {.first.id = EcsChildOf, .second = {.id = EcsIsVariable, .name = "parent"}},
	 }});

	printf("%s\n", any_label);
	ecs_iter_t it = ecs_query_iter(world, any);
	while (ecs_query_next(&it)) {
		PrintQuery(&it);
	}

	printf("%s\n", direct_label);
	it = ecs_query_iter(world, direct);
	while (ecs_query_next(&it)) {
		PrintQuery(&it);
	}

	ecs_query_fini(any);
	ecs_query_fini(direct);
}

static void ExampleRoot(int argc, char *argv[])
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

	PrintMatches(world, Child,"EcsUp query (expects mid and leaf):", "Variable query (expects only mid):");
	printf("\n");
	ecs_fini(world);
}

static void ExampleRoot2(int argc, char *argv[])
{
	ecs_world_t *world = ecs_init_w_args(argc, argv);
	ECS_COMPONENT_DEFINE(world, Marker);
	ECS_TAG(world, Child);

	// root2 (Marker) -> mid1 (Marker) -> mid2 -> leaf2.
	ecs_entity_t root = ecs_entity(world, {.name = "root2"});
	ecs_set(world, root, Marker, {0});
	ecs_entity_t mid1 = ecs_entity(world, {.name = "mid1", .parent = root});
	ecs_set(world, mid1, Marker, {0});
	ecs_entity_t mid2 = ecs_entity(world, {.name = "mid2", .parent = mid1});
	ecs_entity_t leaf = ecs_entity(world, {.name = "leaf2", .parent = mid2});
	ecs_add(world, mid1, Child);
	ecs_add(world, mid2, Child);
	ecs_add(world, leaf, Child);

	PrintMatches(world, Child,"EcsUp query (expects mid1, mid2, and leaf2):","Variable query (expects mid1 and mid2):");
	printf("\n");
	ecs_fini(world);
}

int main(int argc, char *argv[])
{
	ExampleRoot(argc, argv);
	ExampleRoot2(argc, argv);
	return 0;
}
