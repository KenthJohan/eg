#include "eg_memory_pool.h"
#include <assert.h>
#include <stdlib.h>
//#include <stdio.h>



//https://stackoverflow.com/questions/11376288/fast-computing-of-log2-for-64-bit-integers
#define LOG2(x) ((unsigned) (8*sizeof (unsigned long long) - __builtin_clzll((x)) - 1))
#define POW2(x) (1 << (x))
//#define POW2_NEAREST(x) 1 << (32 - __builtin_clz((x) - 1));



void * eg_memory_pool_malloc(unsigned size)
{
	void * v = malloc(size);
	return v;
}



void eg_memory_pool_init(struct eg_memory_pool * pool)
{
	assert(pool);
	for (int i = 0; i < EG_MEMORY_POOL_GRID_COUNT; ++i)
	{
		ck_stack_init(pool->stack + i);
		//size_t size = POW2(i);
		//printf("grid=%i, size=%i\n", i, (int)size);
	}
}


struct eg_memory_entry * eg_memory_pool_get_grid(struct eg_memory_pool * pool, unsigned grid)
{
	assert(pool);
	assert(grid < EG_MEMORY_POOL_GRID_COUNT);
	ck_stack_entry_t *ref = ck_stack_pop_mpmc(pool->stack + grid);
	struct eg_memory_entry * entry;
	if (ref)
	{
		entry = eg_memory_entry_get(ref);
		//printf("Reuse grid=%i\n", grid);
	}
	else
	{
		int size = POW2(grid);
		entry = eg_memory_pool_malloc(sizeof(struct eg_memory_entry) + size);
		entry->grid = grid;
		//printf("Malloc size=%i, grid=%i\n", size, grid);
	}
	return entry;
}

void eg_memory_pool_reclaim(struct eg_memory_pool * pool, struct eg_memory_entry * entry)
{
	assert(pool);
	ck_stack_push_mpmc(pool->stack + entry->grid, &entry->next);
}


struct eg_memory_entry * eg_memory_pool_get(struct eg_memory_pool * pool, unsigned size)
{
	assert(pool);
	int grid = LOG2(size);
	return eg_memory_pool_get_grid(pool, grid);
}





void test_eg_memory_pool()
{
	struct eg_memory_pool pool;
	eg_memory_pool_init(&pool);
	{
		struct eg_memory_entry * m1 = eg_memory_pool_get(&pool, 100);
		struct eg_memory_entry * m2 = eg_memory_pool_get(&pool, 1253);
		struct eg_memory_entry * m3 = eg_memory_pool_get(&pool, 333);
		struct eg_memory_entry * m4 = eg_memory_pool_get(&pool, 444);
		struct eg_memory_entry * m5 = eg_memory_pool_get(&pool, 666);
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
		struct eg_memory_entry * m1 = eg_memory_pool_get(&pool, 100);
		struct eg_memory_entry * m2 = eg_memory_pool_get(&pool, 1253);
		struct eg_memory_entry * m3 = eg_memory_pool_get(&pool, 333);
		struct eg_memory_entry * m4 = eg_memory_pool_get(&pool, 444);
		struct eg_memory_entry * m5 = eg_memory_pool_get(&pool, 666);
		assert(strcmp((char*)m1->memory, "Entry 100") == 0);
		assert(strcmp((char*)m2->memory, "Entry 1253") == 0);
		assert(strcmp((char*)m3->memory, "Entry 444") == 0);// Note: this is before Entry 333
		assert(strcmp((char*)m4->memory, "Entry 333") == 0);
		assert(strcmp((char*)m5->memory, "Entry 666") == 0);
	}
}

