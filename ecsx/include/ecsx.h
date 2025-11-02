#pragma once

#include <flecs.h>

#include "ecsx/ecsx_os_popen.h"
#include "ecsx/ecsx_reparent.h"
#include "ecsx/ecsx_ser.h"
#include "ecsx/ecsx_ops.h"
#include "ecsx/ecsx_trace.h"
#include "ecsx/ecsx_script.h"

int ecsx_get_entities_by_query(ecs_world_t *world, ecs_query_t *query, ecs_entity_t entities[], int count);
int ecsx_get_entities_from_parent(ecs_world_t *world, ecs_entity_t parent, ecs_id_t component, ecs_entity_t entities[], int count);

bool ecsx_has_children(ecs_world_t *world, ecs_entity_t entity);

int32_t ecsx_sum_offset(ecs_world_t *world, ecs_entity_t const members[], ecs_entity_t *last);

int32_t ecsx_children_count(ecs_world_t *world, ecs_entity_t parent);


#ifdef ecs_field
#undef ecs_field
#endif

#ifdef ecs_field_shared
#undef ecs_field_shared
#endif

#ifdef ecs_field_self
#undef ecs_field_self
#endif

#define ecsx_field(it, T, index) (ECS_CAST(T*, ecs_field_w_size(it, sizeof(T), index)))
#define ecsx_auto_id(id) (ECS_IS_PAIR((id)) ? ECS_PAIR_FIRST((id)) : (id))
#define ecsx_field_id1(it, index) ecsx_auto_id(ecs_field_id((it), (index)))
#define ecsx_get_symbol1(world, T) ecs_get_symbol(world, ecsx_auto_id(ecs_id(T)))
#define ecsx_get_symbol1_it(it, index) ecs_get_symbol(it->world, ecsx_field_id1(it, index))

#define ecs_field(it, T, index)\
    (ecsx_field_id1(it, index) == ecs_id(T)) ? ecsx_field(it, T, index) : \
    (ecs_abort_(ECS_INVALID_PARAMETER, __FILE__, __LINE__, \
    "Field %i '%s' does not match %s", \
    index, ecsx_get_symbol1_it(it, index), ecsx_get_symbol1(it->world, T)), \
    ecs_os_abort(), abort(), NULL)


#define ecs_field_shared(it, T, index)\
    (ecs_field_is_self(it, index) == false) ? ecsx_field(it, T, index) : \
    (ecs_abort_(ECS_INVALID_PARAMETER, __FILE__, __LINE__, \
    "Field %i '%s' is not shared", \
    index, ecsx_get_symbol1_it(it, index)), \
    ecs_os_abort(), abort(), NULL)

#define ecs_field_self(it, T, index)\
    (ecs_field_is_self(it, index) == true) ? ecsx_field(it, T, index) : \
    (ecs_abort_(ECS_INVALID_PARAMETER, __FILE__, __LINE__, \
    "Field %i '%s' is not self", \
    index, ecsx_get_symbol1_it(it, index)), \
    ecs_os_abort(), abort(), NULL)

