#include "eg_log.h"
#include "eg_memory_pool.h"

#include <ck_ring.h>
#include <ck_backoff.h>



static char const * get_color(int level)
{
	switch (level)
	{
	case -1: return "";
	case -2: return ECS_YELLOW;
	case -3: return ECS_RED;
	case -4: return ECS_RED;
	default: return ECS_MAGENTA;
	}
}
static char const * get_text(int level)
{
	switch (level)
	{
	case -1: return "";
	case -2: return "warning";
	case -3: return "error";
	case -4: return "fatal";
	default: return "info";
	}
}


enum channel
{
	CHANNEL_STDOUT,
	CHANNEL_STDERR,
	CHANNEL_COUNT
};
#define EG_LOG_RING_CAPACITY 64
static ck_ring_t g_ring[CHANNEL_COUNT];
static ck_ring_buffer_t * g_rbuffer[CHANNEL_COUNT];
static struct eg_memory_pool pool;



static void enqueue_blocking(enum channel q, struct eg_memory_entry * entry)
{
	ck_backoff_t backoff = CK_BACKOFF_INITIALIZER;
	bool rv;
enqueue:
	rv = ck_ring_enqueue_mpmc(g_ring+q, g_rbuffer[q], (void*)entry);
	if (CK_CC_UNLIKELY(rv == false))
	{
		ck_pr_stall();
		ecs_os_sleep(0, backoff);
		ck_backoff_eb(&backoff);
		goto enqueue;
	}
}


static struct eg_memory_entry * dequeue_blocking(enum channel q)
{
	struct eg_memory_entry * entry;
	ck_backoff_t backoff = CK_BACKOFF_INITIALIZER;
	bool rv;
enqueue:
	rv = ck_ring_dequeue_mpmc(g_ring+q, g_rbuffer[q], (void*)&entry);
	if(rv == false)
	{
		ck_pr_stall();
		ecs_os_sleep(0, backoff);
		ck_backoff_eb(&backoff);
		goto enqueue;
	}
	return entry;
}






static void eg_log_msg1(int32_t level, const char *file, int32_t line, const char *msg)
{
	char const * color = get_color(level);
	char const * text = get_text(level);
	int len = strlen(msg);
	struct eg_memory_entry * entry = eg_memory_pool_get(&pool, len);
	eg_memory_entry_assert(&pool, entry);
	char * buf = (char*)entry->memory;
	char indent[32] = {'\0'};
	if (level >= 0)
	{
		if (ecs_os_api.log_indent_)
		{
			int i;
			for (i = 0; i < ecs_os_api.log_indent_; i ++)
			{
				indent[i * 2] = '|';
				indent[i * 2 + 1] = ' ';
			}
			indent[i * 2] = '\0';
		}
	}

	enum channel q;
	if (level < 0)
	{
		sprintf(buf, "%s%s"ECS_NORMAL": %s%s:%d: %s", color, text, indent, file, line, msg);
		q = CHANNEL_STDERR;
	}
	else
	{
		sprintf(buf, "%s%s"ECS_NORMAL":%s %s", color, text, indent, msg);
		q = CHANNEL_STDOUT;
	}
	enqueue_blocking(q, entry);

}








static bool consume_print(enum channel q)
{
	struct eg_memory_entry * entry;
	bool rv;
	// TODO: Print a batch of messages.
	rv = ck_ring_dequeue_mpmc(g_ring+q, g_rbuffer[q], (void*)&entry);
	if (rv == true)
	{
		eg_memory_entry_assert(&pool, entry);
		char * buf = (char*)entry->memory;
		//printf("Q%i: %s\n", q, buf);
		printf("q=%i, reuses=%i, mallocs=%i, id=%i", q, pool.amount_reuse, pool.amount_malloc, entry->id);
		printf("%s\n", buf);
		eg_memory_pool_reclaim(&pool, entry);
	}
	return rv;
}


//static int g_stall_count = 0;
static void * eg_log_printer_thread(void * arg)
{
	ck_backoff_t backoff = CK_BACKOFF_INITIALIZER;
	while(1)
	{
		bool rv = consume_print(CHANNEL_STDERR) || consume_print(CHANNEL_STDOUT);
		if (rv == true)
		{
			backoff = CK_BACKOFF_INITIALIZER;
		}
		else
		{
			//g_stall_count++;
			//ck_pr_stall();
			ecs_os_sleep(0, backoff);
			ck_backoff_eb(&backoff);
		}
	}
}



void FlecsComponentsEgLogImport(ecs_world_t *world)
{
	ECS_MODULE(world, FlecsComponentsEgLog);
	ecs_set_name_prefix(world, "Eg");
	for (int i = 0; i < CHANNEL_COUNT; ++i)
	{
		ck_ring_init(g_ring + i, EG_LOG_RING_CAPACITY);
		g_rbuffer[i] = ecs_os_calloc(sizeof(ck_ring_buffer_t) * EG_LOG_RING_CAPACITY);
	}
	eg_memory_pool_init(&pool);

	ecs_os_api.log_ = eg_log_msg1;
	ecs_os_thread_t t1 = ecs_os_thread_new(eg_log_printer_thread, NULL);
}



