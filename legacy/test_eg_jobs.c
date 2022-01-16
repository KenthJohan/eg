#include <stdio.h>
#include <flecs.h>
#include <ck_stack.h>
#include <ck_ring.h>




struct job_entry
{
	ecs_iter_t it;
	ecs_iter_action_t action;
};


#define TEST_JOBPOOL_NUM_PRIO 4
struct test_jobpool
{
	ck_ring_t ring[TEST_JOBPOOL_NUM_PRIO+1];
	ck_ring_buffer_t *buffer[TEST_JOBPOOL_NUM_PRIO+1];
	ecs_world_t * world;
};


void dojob(struct test_jobpool * pool, struct job_entry * job)
{

}






void * test_worker_thread(void * arg)
{
	struct test_jobpool * pool = arg;
	int i = 0;
	while(1)
	{
		struct job_entry * item = NULL;
		bool success = false;
		for(i = 0; i < TEST_JOBPOOL_NUM_PRIO; ++i)
		{
			success = ck_ring_dequeue_mpmc(pool->ring + i, pool->buffer[i], (void*)&item);
			if (success == true) {break;}
		}
		if (success == false)
		{
			// Zero jobs todo idling...
			continue;
		}
		dojob(pool, item);
		ck_ring_enqueue_mpmc(pool->ring + i, pool->buffer[i], (void*)item);
	}
}

























int main(int argc, char *argv[])
{

	return 0;
}
