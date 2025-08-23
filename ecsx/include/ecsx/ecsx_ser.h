#pragma once
#include <flecs.h>

int ecsx_flecs_expr_ser_primitive(const ecs_world_t *world, ecs_primitive_kind_t kind, const void *base, ecs_strbuf_t *str, bool is_expr);
