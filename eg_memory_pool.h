#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <ck_ring.h>
#include <ck_queue.h>
#include <ck_stack.h>


#define EG_MEMORY_POOL_GRID_COUNT 32

struct eg_memory_entry
{
	int grid;
	ck_stack_entry_t next;
	uint8_t memory[0];
};
CK_STACK_CONTAINER(struct eg_memory_entry, next, eg_memory_entry_get);

struct eg_memory_pool
{
	ck_stack_t stack[EG_MEMORY_POOL_GRID_COUNT] CK_CC_CACHELINE;
};

void eg_memory_pool_init(struct eg_memory_pool * pool);
void eg_memory_pool_reclaim(struct eg_memory_pool * pool, struct eg_memory_entry * entry);
struct eg_memory_entry * eg_memory_pool_get_grid(struct eg_memory_pool * pool, unsigned grid);
struct eg_memory_entry * eg_memory_pool_get(struct eg_memory_pool * pool, unsigned size);

void test_eg_memory_pool();

