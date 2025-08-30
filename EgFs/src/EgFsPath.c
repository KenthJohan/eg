#include "EgFs/EgFsPath.h"
#include "EgFs.h"
#include "hash.h"

uint64_t EgFsPath_name_index_hash(const void *ptr)
{
	const EgFsPath *str = ptr;
	ecs_assert(str->hash != 0, ECS_INTERNAL_ERROR, NULL);
	return str->hash;
}

int EgFsPath_name_index_compare(const void *ptr1, const void *ptr2)
{
	const EgFsPath *str1 = ptr1;
	const EgFsPath *str2 = ptr2;
	ecs_size_t len1 = str1->length;
	ecs_size_t len2 = str2->length;
	if (len1 != len2) {
		return (len1 > len2) - (len1 < len2);
	}
	return ecs_os_memcmp(str1->value, str2->value, len1);
}

EgFsPath EgFsPath_get_hashed_string(const char *name, ecs_size_t length, uint64_t hash)
{
	if (!length) {
		length = ecs_os_strlen(name);
	} else {
		ecs_assert(length == ecs_os_strlen(name), ECS_INTERNAL_ERROR, NULL);
	}

	if (!hash) {
		hash = flecs_hash(name, length);
	} else {
		uint64_t h = flecs_hash(name, length);
		ecs_assert(hash == h, ECS_INTERNAL_ERROR, NULL);
	}

	return (EgFsPath){
	.value = ECS_CONST_CAST(char *, name),
	.length = length,
	.hash = hash};
}

const uint64_t *EgFsPath_name_index_find_ptr(const ecs_hashmap_t *map, const char *name, ecs_size_t length, uint64_t hash)
{
	EgFsPath hs = EgFsPath_get_hashed_string(name, length, hash);
	ecs_hm_bucket_t *b = flecs_hashmap_get_bucket(map, hs.hash);
	if (!b) {
		return NULL;
	}

	EgFsPath *keys = ecs_vec_first(&b->keys);
	int32_t i, count = ecs_vec_count(&b->keys);

	for (i = 0; i < count; i++) {
		EgFsPath *key = &keys[i];
		ecs_assert(key->hash == hs.hash, ECS_INTERNAL_ERROR, NULL);

		if (hs.length != key->length) {
			continue;
		}

		if (!ecs_os_strcmp(name, key->value)) {
			uint64_t *e = ecs_vec_get_t(&b->values, uint64_t, i);
			ecs_assert(e != NULL, ECS_INTERNAL_ERROR, NULL);
			return e;
		}
	}

	
	return NULL;
}

uint64_t EgFsPath_name_index_find(const ecs_hashmap_t *map,const char *name,ecs_size_t length,uint64_t hash)
{
    const uint64_t *id = EgFsPath_name_index_find_ptr(map, name, length, hash);
    if (id) {
        return id[0];
    }
    return 0;
}


void EgFsPath_name_index_ensure(ecs_hashmap_t *map,uint64_t id,const char *name,ecs_size_t length,uint64_t hash)
{
    ecs_check(name != NULL, ECS_INVALID_PARAMETER, NULL);

    EgFsPath key = EgFsPath_get_hashed_string(name, length, hash);
    
    uint64_t existing = EgFsPath_name_index_find(map, name, key.length, key.hash);
    if (existing) {
        if (existing != id) {
            ecs_abort(ECS_ALREADY_DEFINED, 
                "conflicting entity registered with name '%s' "
                "(existing = %u, new = %u)", 
                name, (uint32_t)existing, (uint32_t)id);
        }
    }

    flecs_hashmap_result_t hmr = flecs_hashmap_ensure(
        map, &key, uint64_t);
    *((uint64_t*)hmr.value) = id;
error:
    return;
}

void ecs_on_set(EgFsPath)(ecs_iter_t *it) 
{
    ecs_world_t *world = it->real_world;
    EgFsPath *ptr = ecs_field(it, EgFsPath, 0);
	ecs_assert(ecs_has(world, ecs_id(EgFsLookup), EgFsLookup), ECS_INTERNAL_ERROR, NULL);
	EgFsLookup * lookup = ecs_singleton_get_mut(world, EgFsLookup);
	ecs_assert(lookup != NULL, ECS_INTERNAL_ERROR, NULL);
	ecs_assert(ptr != NULL, ECS_INTERNAL_ERROR, NULL);
	for (int i = 0; i < it->count; i ++) {
		EgFsPath * curr = &ptr[i];
		ecs_entity_t e = it->entities[i];
		curr->length = ecs_os_strlen(curr->value);
		curr->hash = flecs_hash(curr->value, curr->length);
		EgFsPath_name_index_ensure(&lookup->hm, e, curr->value, curr->length, curr->hash);
	}
}