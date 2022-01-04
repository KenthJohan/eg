#pragma once
#include <ck_stack.h>


#define EG_MEMORY_POOL_GRID_COUNT 32
struct eg_memory_pool
{
	_Atomic int amount_malloc;
	_Atomic int amount_reuse;
	ck_stack_t stack[EG_MEMORY_POOL_GRID_COUNT] CK_CC_CACHELINE;
};

struct eg_memory_entry
{
	struct eg_memory_pool * pool;
	_Atomic int id;
	int grid;
	int requested_size;
	ck_stack_entry_t next;
	uint8_t memory[0];
};

CK_STACK_CONTAINER(struct eg_memory_entry, next, eg_memory_entry_get);

void eg_memory_entry_assert(struct eg_memory_pool * pool, struct eg_memory_entry * entry);
void eg_memory_pool_init(struct eg_memory_pool * pool);
void eg_memory_pool_reclaim(struct eg_memory_pool * pool, struct eg_memory_entry * entry);
struct eg_memory_entry * eg_memory_pool_get_grid(struct eg_memory_pool * pool, unsigned grid);
struct eg_memory_entry * eg_memory_pool_get(struct eg_memory_pool * pool, int size);

void test_eg_memory_pool();

