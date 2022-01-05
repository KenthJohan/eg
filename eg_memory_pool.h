#pragma once
#include <ck_stack.h>


#define EG_MEMORY_POOL_GRID_COUNT 32
struct eg_memory_pool
{
	uint32_t amount_malloc; // Not necessary
	uint32_t amount_reuse; // Not necessary
	ck_stack_t stack[EG_MEMORY_POOL_GRID_COUNT] CK_CC_CACHELINE;
};

struct eg_memory_entry
{
	struct eg_memory_pool * pool;
	uint32_t id; // Not necessary
	uint32_t grid; // Not necessary
	uint32_t requested_size; // Not necessary
	ck_stack_entry_t next;
	uint8_t memory[0];
};

CK_STACK_CONTAINER(struct eg_memory_entry, next, eg_memory_entry_get);

void eg_memory_pool_assert(struct eg_memory_pool * pool, struct eg_memory_entry * entry);
void eg_memory_pool_init(struct eg_memory_pool * pool);
void eg_memory_pool_reclaim(struct eg_memory_pool * pool, struct eg_memory_entry * entry);
struct eg_memory_entry * eg_memory_pool_get_grid(struct eg_memory_pool * pool, uint32_t grid);
struct eg_memory_entry * eg_memory_pool_get(struct eg_memory_pool * pool, uint32_t size);

void test_eg_memory_pool();

