#include <stdio.h>
#include <flecs.h>
#include <ck_stack.h>
#include <ck_ring.h>



enum job_t
{
JOB1,
JOB2,
JOB3,
JOB4
};

int dojob(enum job_t job)
{
	int prioyield = -2; // Unknown job
	switch(job)
	{
	case JOB1:
		ecs_os_sleep(1,0); //Simulate heavy job
		prioyield = 2; // Forever job, third priority
		break;
	case JOB2:
		ecs_os_sleep(2,0); //Simulate heavy job
		prioyield = 0; // Forever job, first priority
		break;
	case JOB3:
		prioyield = 1; // Forever job, second priority
		break;
	case JOB4:
		ecs_os_sleep(2,0); //Simulate heavy job
		if (1) {prioyield = -1;} // Job complete
		break;
	};
	return prioyield;
}

struct job_entry
{
	enum job_t job;
	ck_stack_entry_t next;
};
CK_STACK_CONTAINER(struct job_entry, next, job_entry_get);


#define TEST_JOBPOOL_NUM_PRIO 4
struct test_jobpool
{
	ck_stack_t stack[TEST_JOBPOOL_NUM_PRIO] CK_CC_CACHELINE;
};


void * test_worker_thread(void * arg)
{
	struct test_jobpool * pool = arg;
	for(int i = 0; i < TEST_JOBPOOL_NUM_PRIO; ++i)
	{
		ck_stack_entry_t *ref;
		do
		{
			ref = ck_stack_pop_mpmc(pool->stack + i);
		} while(ref == NULL);
		struct job_entry * entry = job_entry_get(ref);
		int jobprio = dojob(entry->job);
		if (jobprio > 0)
		{
			i = 0;
			ck_stack_push_mpmc(pool->stack + jobprio, &entry->next);
		}
		if (jobprio == -1)
		{
			ecs_os_free(entry);
			i = 0; // Keep finding jobs todo.
		}
	}
}

























int main(int argc, char *argv[])
{

	return 0;
}
