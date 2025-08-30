#pragma once
#include <stdint.h>
#include <flecs.h>

int EgFsPath_name_index_compare(const void *ptr1, const void *ptr2);

uint64_t EgFsPath_name_index_hash(const void *ptr);

void EgFsPath_name_index_ensure(ecs_hashmap_t *map,uint64_t id,const char *name,ecs_size_t length,uint64_t hash);

void ecs_on_set(EgFsPath)(ecs_iter_t *it);

uint64_t EgFsPath_name_index_find(const ecs_hashmap_t *map,const char *name,ecs_size_t length,uint64_t hash);