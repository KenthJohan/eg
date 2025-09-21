#include "EgFs/EgFsPaths.h"
#include "EgFs.h"
#include "hash.h"

ECS_COMPONENT_DECLARE(EgFsPathsHashed);
ECS_COMPONENT_DECLARE(EgFsPathsLookup);

uint64_t EgFsPathsLookup_hash(const EgFsPathsHashed *str)
{
	ecs_assert(str->hash != 0, ECS_INTERNAL_ERROR, NULL);
	return str->hash;
}

int EgFsPathsLookup_compare(const EgFsPathsHashed *ptr1, const EgFsPathsHashed *ptr2)
{
	ecs_size_t len1 = ptr1->length;
	ecs_size_t len2 = ptr2->length;
	if (len1 != len2) {
		return (len1 > len2) - (len1 < len2);
	}
	return ecs_os_memcmp(ptr1->value, ptr2->value, len1);
}

EgFsPathsHashed EgFsPath_get_hashed_string(const char *name, ecs_size_t length, uint64_t hash)
{
	if (length) {
		ecs_assert(length == ecs_os_strlen(name), ECS_INTERNAL_ERROR, NULL);
	} else {
		length = ecs_os_strlen(name);
	}
	if (hash) {
		uint64_t h = flecs_hash(name, length);
		ecs_assert(hash == h, ECS_INTERNAL_ERROR, NULL);
	} else {
		hash = flecs_hash(name, length);
	}
	return (EgFsPathsHashed){.value = ECS_CONST_CAST(char *, name), .length = length, .hash = hash};
}

const uint64_t *EgFsPath_name_index_find_ptr(const ecs_hashmap_t *hm, const char *name, ecs_size_t length, uint64_t hash)
{
	EgFsPathsHashed hs = EgFsPath_get_hashed_string(name, length, hash);
	ecs_hm_bucket_t *b = flecs_hashmap_get_bucket(hm, hs.hash);
	if (b == NULL) {
		return NULL;
	}
	EgFsPathsHashed *keys = ecs_vec_first(&b->keys);
	int32_t count = ecs_vec_count(&b->keys);
	for (int32_t i = 0; i < count; i++) {
		EgFsPathsHashed *key = &keys[i];
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

uint64_t EgFsPath_name_index_find(const ecs_hashmap_t *hm, const char *name, ecs_size_t length, uint64_t hash)
{
	const uint64_t *id = EgFsPath_name_index_find_ptr(hm, name, length, hash);
	if (id) {
		return id[0];
	}
	return 0;
}

void EgFsPath_name_index_ensure(ecs_hashmap_t *hm, uint64_t id, const char *name, ecs_size_t length, uint64_t hash)
{
	ecs_check(name != NULL, ECS_INVALID_PARAMETER, NULL);
	EgFsPathsHashed key = EgFsPath_get_hashed_string(name, length, hash);
	uint64_t existing = EgFsPath_name_index_find(hm, name, key.length, key.hash);
	if (existing && (existing != id)) {
		ecs_abort(ECS_ALREADY_DEFINED, "conflicting entity registered with name '%s' (existing = %u, new = %u)", name, (uint32_t)existing, (uint32_t)id);
	}
	flecs_hashmap_result_t hmr = flecs_hashmap_ensure(hm, &key, uint64_t);
	*((uint64_t *)hmr.value) = id;
error:
	return;
}

void ecs_on_set(EgFsPathsHashed)(ecs_iter_t *it)
{
	ecs_world_t *world = it->real_world;
	EgFsPathsHashed *ptr = ecs_field(it, EgFsPathsHashed, 0);
	ecs_assert(ecs_has(world, ecs_id(EgFsPathsLookup), EgFsPathsLookup), ECS_INTERNAL_ERROR, NULL);
	EgFsPathsLookup *lookup = ecs_singleton_get_mut(world, EgFsPathsLookup);
	ecs_assert(lookup != NULL, ECS_INTERNAL_ERROR, NULL);
	ecs_assert(ptr != NULL, ECS_INTERNAL_ERROR, NULL);
	for (int i = 0; i < it->count; i++) {
		EgFsPathsHashed *curr = &ptr[i];
		ecs_entity_t e = it->entities[i];
		curr->length = ecs_os_strlen(curr->value);
		curr->hash = flecs_hash(curr->value, curr->length);
		EgFsPath_name_index_ensure(&lookup->hm, e, curr->value, curr->length, curr->hash);
	}
}

ecs_entity_t EgFs_path_new(ecs_world_t *world, const char *path)
{
	EgFsPathsLookup const *lookup = ecs_singleton_get(world, EgFsPathsLookup);
	const ecs_hashmap_t *hm = &lookup->hm;
	ecs_entity_t a = EgFsPath_name_index_find(hm, path, 0, 0);
	if (a == 0) {
		a = ecs_entity_init(world,
		&(ecs_entity_desc_t){
		.parent = EgFsFiles,
		});
		ecs_set(world, a, EgFsPathsHashed, {.value = path});
		ecs_doc_set_name(world, a, path);
	}
	return a;
}

static void callback_newpath(const ecs_function_ctx_t *ctx, int argc, const ecs_value_t *argv, ecs_value_t *result)
{
	(void)ctx;
	(void)argc;
	ecs_world_t *world = ctx->world;
	const char *path = *(char **)argv[0].ptr;
	*(int64_t *)result->ptr = EgFs_path_new(world, path);
}

/*
The destructor should free resources.
The move hook should move resources from one location to another.
The copy hook should copy resources from one location to another.
*/

ECS_CTOR(EgFsPathsHashed, ptr, {
	ecs_os_zeromem(ptr);
})

ECS_DTOR(EgFsPathsHashed, ptr, {
	ecs_os_free((void *)ptr->value);
})

ECS_MOVE(EgFsPathsHashed, dst, src, {
	ecs_os_free((void *)dst->value);
	*dst = *src;
	ecs_os_zeromem(src);
})

ECS_COPY(EgFsPathsHashed, dst, src, {
	ecs_os_free((void *)dst->value);
	dst->value = ecs_os_strdup(src->value);
})

/*
The epoll API performs a similar task to poll(2): monitoring
multiple file descriptors to see if I/O is possible on any of
them.  The epoll API can be used either as an edge-triggered or a
level-triggered interface and scales well to large numbers of
watched file descriptors.

The destructor should free resources.
The move hook should move resources from one location to another.
*/

ECS_CTOR(EgFsPathsLookup, ptr, {
	flecs_hashmap_init_(
	&ptr->hm,
	ECS_SIZEOF(EgFsPathsHashed),
	ECS_SIZEOF(uint64_t),
	(ecs_hash_value_action_t)EgFsPathsLookup_hash,
	(ecs_compare_action_t)EgFsPathsLookup_compare,
	NULL);
})

ECS_DTOR(EgFsPathsLookup, ptr, {
	flecs_hashmap_fini(&ptr->hm);
})

ECS_MOVE(EgFsPathsLookup, dst, src, {
	flecs_hashmap_fini(&dst->hm);
	dst->hm = src->hm;
	src->hm = (ecs_hashmap_t){0};
})

void EgFsPathsImport(ecs_world_t *world)
{
	ECS_MODULE(world, EgFsPaths);
	ecs_set_name_prefix(world, "EgFsPaths");

	ECS_COMPONENT_DEFINE(world, EgFsPathsHashed);
	ECS_COMPONENT_DEFINE(world, EgFsPathsLookup);

	ecs_set_hooks_id(world, ecs_id(EgFsPathsHashed),
	&(ecs_type_hooks_t){
	.ctor = ecs_ctor(EgFsPathsHashed),
	.move = ecs_move(EgFsPathsHashed),
	.copy = ecs_copy(EgFsPathsHashed),
	.dtor = ecs_dtor(EgFsPathsHashed),
	.on_set = ecs_on_set(EgFsPathsHashed),
	});

	ecs_set_hooks_id(world, ecs_id(EgFsPathsLookup),
	&(ecs_type_hooks_t){
	.ctor = ecs_ctor(EgFsPathsLookup),
	.move = ecs_move(EgFsPathsLookup),
	.dtor = ecs_dtor(EgFsPathsLookup),
	});

	ecs_struct_init(world,
	&(ecs_struct_desc_t){
	.entity = ecs_id(EgFsPathsHashed),
	.members = {
	{.name = "value", .type = ecs_id(ecs_string_t)},
	{.name = "length", .type = ecs_id(ecs_i32_t)},
	{.name = "hash", .type = ecs_id(ecs_u64_t)},
	}});

	{
		ecs_entity_t m = ecs_function_init(world,
		&(ecs_function_desc_t){
		.name = "new",
		.return_type = ecs_id(ecs_entity_t),
		.params = {{.name = "path", .type = ecs_id(ecs_string_t)}},
		.callback = callback_newpath});
		ecs_doc_set_brief(world, m, "Lookup child by name");
	}

	ecs_singleton_add(world, EgFsPathsLookup);
}
