#include "ecsx.h"
#include <stdlib.h>

int ecsx_get_entities_by_query(ecs_world_t *world, ecs_query_t *query, ecs_entity_t entities[], int count)
{
	ecs_iter_t it = ecs_query_iter(world, query);
	int i = 0;
	while (ecs_query_next(&it)) {
		for (int j = 0; j < it.count; j++) {
			if (i >= count) {
				goto exit_return;
			}
			entities[i] = it.entities[j];
			i++;
		}
	}
exit_return:
	return i;
}

int ecsx_get_entities_from_parent(ecs_world_t *world, ecs_entity_t parent, ecs_id_t component, ecs_entity_t entities[], int count)
{
	ecs_query_t *q = ecs_query(world,
	{.terms = {
	 {ecs_childof(parent), .inout = EcsInOutFilter},
	 {component, .inout = EcsInOutFilter},
	 }});
	int k = ecsx_get_entities_by_query(world, q, entities, count);
	ecs_query_fini(q);
	return k;
}

bool ecsx_has_children(ecs_world_t *world, ecs_entity_t entity)
{
	ecs_iter_t it = ecs_children(world, entity);
	if (it.world == NULL) {
		return false;
	}
	bool r = ecs_iter_is_true(&it);
	ecs_iter_fini(&it);
	return r;
}


int32_t ecsx_sum_offset(ecs_world_t *world, ecs_entity_t const members[], ecs_entity_t *last)
{
	int32_t o = 0;
	int i = 0;
	while (members[i]) {
		EcsMember const *m = ecs_get(world, members[i], EcsMember);
		if (m == NULL) {
			break;
		}
		o += m->offset;
		(*last) = members[i];
		i++;
	}
	return o;
}


int32_t ecsx_children_count(ecs_world_t *world, ecs_entity_t parent)
{
	ecs_iter_t it = ecs_children(world, parent);
	if (it.world == NULL) {
		return 0;
	}
	int32_t count = 0;
	while (ecs_children_next(&it)) {
		count += it.count;
	}
	ecs_iter_fini(&it);
	return count;
}





void ecsx_remove_by_it_term(ecs_iter_t *it, ecs_entity_t e, int index)
{
	ecs_entity_t trav = it->query->terms[index].trav;
	if (trav) {
		// ecs_entity_t src = it->sources[index];
		ecs_remove_pair(it->world, e, trav, index);
	} else {
		ecs_id_t id = it->ids[index];
		ecs_remove_id(it->world, e, id);
	}
}



/*
printf("remove1(%s,%s)\n", ecs_get_name(world, prefab), ecs_get_name(world, subject));
const ecs_type_t *type = ecs_get_type(world, subject);
char *type_str = ecs_type_str(world, type);
printf("ecs_type_str: %s\n\n", type_str);
ecs_os_free(type_str);
        char const * n1 = ecs_get_name(world, subject);
        char const * n2 = ecs_get_name(world, id);
        printf("ecs_remove_id(%s,%s)\n", n1, n2);
*/

/*
https://www.flecs.dev/flecs/md_docs_2ComponentTraits.html
*/
void remove_copies_from_prefab(ecs_world_t *world, ecs_entity_t prefab, ecs_entity_t subject)
{
	// Removes every component of subject that are also in prefab:
	const ecs_type_t *type = ecs_get_type(world, prefab);
	for (int i = 0; i < type->count; i++) {
		ecs_id_t id = type->array[i];
		if (ecs_has_id(world, subject, id) == false) {
			continue;
		}
		if (ecs_is_alive(world, id) == false) {
			continue;
		}
		ecs_remove_id(world, subject, id);
	}
	// Removes every child of subject that are also in prefab:
	ecs_query_t *q = ecs_query(world,
	{.cache_kind = EcsQueryCacheNone,
	.terms = {
	{.id = ecs_pair(prefab, EcsWildcard)},
	{.id = ecs_childof(subject)}}});
	ecs_iter_t it = ecs_query_iter(world, q);
	while (ecs_query_next(&it)) {
		for (int i = 0; i < it.count; ++i) {
			ecs_entity_t e = it.entities[i];
			ecs_delete(world, e);
		}
	}
	ecs_query_fini(q);
}

void ecsx_toggle(ecs_world_t *world, ecs_entity_t entity, ecs_entity_t relation, ecs_entity_t toggle)
{
	if (relation) {
		if (ecs_has_pair(world, entity, relation, toggle)) {
			ecs_remove_pair(world, entity, relation, toggle);
			remove_copies_from_prefab(world, toggle, entity);
		} else {
			ecs_add_pair(world, entity, relation, toggle);
		}
	} else {
		if (ecs_has_id(world, entity, toggle)) {
			ecs_remove_id(world, entity, toggle);
			remove_copies_from_prefab(world, toggle, entity);
		} else {
			ecs_add_id(world, entity, toggle);
		}
	}
}
