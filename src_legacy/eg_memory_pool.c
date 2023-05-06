#include "eg_memory_pool.h"
#include "eg_basics.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>



//https://stackoverflow.com/questions/11376288/fast-computing-of-log2-for-64-bit-integers
#define LOG2(x) ((unsigned) (8*sizeof (unsigned long long) - __builtin_clzll((x)) - 1))
#define POW2(x) (1 << (x))
//#define POW2_NEAREST(x) 1 << (32 - __builtin_clz((x) - 1));




void eg_memory_pool_assert(struct eg_memory_pool * pool, struct eg_memory_entry * entry)
{
	EG_ASSERT(pool != NULL);
	EG_ASSERT(entry != NULL);
	EG_ASSERT(pool == entry->pool);
	EG_ASSERT(entry->grid < EG_MEMORY_POOL_GRID_COUNT);
}


void * eg_memory_pool_malloc(unsigned size)
{
	void * v = malloc(size);
	memset(v, 0, size);
	return v;
}


void eg_memory_pool_init(struct eg_memory_pool * pool)
{
	EG_ASSERT(pool);
	pool->amount_malloc = 0; // Not necessary
	pool->amount_reuse = 0; // Not necessary
	for (int i = 0; i < EG_MEMORY_POOL_GRID_COUNT; ++i)
	{
		ck_stack_init(pool->stack + i);
		EG_TRACE("grid=%i, size=%i\n", i, (int)POW2(i));
	}
}


struct eg_memory_entry * eg_memory_pool_request_by_grid(struct eg_memory_pool * pool, uint32_t grid)
{
	EG_ASSERT(pool);
	EG_ASSERT(grid < EG_MEMORY_POOL_GRID_COUNT);
	ck_stack_entry_t *ref = ck_stack_pop_mpmc(pool->stack + grid);
	struct eg_memory_entry * entry;
	if (ref)
	{
		entry = eg_memory_entry_get(ref);
		ck_pr_faa_32(&pool->amount_reuse, 1); // Not necessary
		EG_TRACE("Reuse grid=%i\n", grid);
	}
	else
	{
		uint32_t size = POW2(grid);
		entry = eg_memory_pool_malloc(sizeof(struct eg_memory_entry) + size);
		entry->pool = pool; // Not necessary
		entry->grid = grid; // Not necessary
		entry->id = ck_pr_faa_32(&pool->amount_malloc, 1); // Not necessary
		EG_TRACE("Malloc size=%i, grid=%i\n", size, grid);
	}
	eg_memory_pool_assert(pool, entry);
	return entry;
}

void eg_memory_pool_reclaim(struct eg_memory_pool * pool, struct eg_memory_entry * entry)
{
	EG_ASSERT(pool);
	eg_memory_pool_assert(pool, entry);
	ck_stack_push_mpmc(pool->stack + entry->grid, &entry->next);
}


struct eg_memory_entry * eg_memory_pool_request(struct eg_memory_pool * pool, uint32_t requested_size)
{
	EG_ASSERT(pool);
	uint32_t grid = LOG2(requested_size) + 1;
	uint32_t size = POW2(grid);
	EG_ASSERT(requested_size < size);
	struct eg_memory_entry * entry;
	entry = eg_memory_pool_request_by_grid(pool, grid);
	entry->requested_size = requested_size; // Not necessary
	return entry;
}






void test_eg_memory_pool()
{
	struct eg_memory_pool pool;
	eg_memory_pool_init(&pool);
	{
		struct eg_memory_entry * m1 = eg_memory_pool_request(&pool, 100);
		struct eg_memory_entry * m2 = eg_memory_pool_request(&pool, 1253);
		struct eg_memory_entry * m3 = eg_memory_pool_request(&pool, 333);
		struct eg_memory_entry * m4 = eg_memory_pool_request(&pool, 444);
		struct eg_memory_entry * m5 = eg_memory_pool_request(&pool, 666);
		sprintf((char*)m1->memory, "Entry %i", 100);
		sprintf((char*)m2->memory, "Entry %i", 1253);
		sprintf((char*)m3->memory, "Entry %i", 333);
		sprintf((char*)m4->memory, "Entry %i", 444);
		sprintf((char*)m5->memory, "Entry %i", 666);
		eg_memory_pool_reclaim(&pool, m1);
		eg_memory_pool_reclaim(&pool, m2);
		eg_memory_pool_reclaim(&pool, m3);
		eg_memory_pool_reclaim(&pool, m4);
		eg_memory_pool_reclaim(&pool, m5);
	}

	{
		struct eg_memory_entry * m1 = eg_memory_pool_request(&pool, 100);
		struct eg_memory_entry * m2 = eg_memory_pool_request(&pool, 1253);
		struct eg_memory_entry * m3 = eg_memory_pool_request(&pool, 333);
		struct eg_memory_entry * m4 = eg_memory_pool_request(&pool, 444);
		struct eg_memory_entry * m5 = eg_memory_pool_request(&pool, 666);
		EG_ASSERT(strcmp((char*)m1->memory, "Entry 100") == 0);
		EG_ASSERT(strcmp((char*)m2->memory, "Entry 1253") == 0);
		EG_ASSERT(strcmp((char*)m3->memory, "Entry 444") == 0);// Note: this is before Entry 333
		EG_ASSERT(strcmp((char*)m4->memory, "Entry 333") == 0);
		EG_ASSERT(strcmp((char*)m5->memory, "Entry 666") == 0);
	}
}

