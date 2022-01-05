#include "eg_log.h"
#include "eg_memory_pool.h"

#include <ck_ring.h>
#include <ck_backoff.h>



static char const * eg_get_log_color(int level)
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
static char const * eg_get_log_title(int level)
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


enum eg_log_channel
{
	EG_LOG_CHANNEL_STDOUT,
	EG_LOG_CHANNEL_STDERR,
	EG_LOG_CHANNEL_COUNT
};
#define EG_LOG_RING_CAPACITY 64
static ck_ring_t g_ring[EG_LOG_CHANNEL_COUNT];
static ck_ring_buffer_t * g_ringbuf[EG_LOG_CHANNEL_COUNT];
static struct eg_memory_pool pool;



static void enqueue_blocking(enum eg_log_channel channel, struct eg_memory_entry * entry)
{
	ck_backoff_t backoff = CK_BACKOFF_INITIALIZER;
	bool success;
enqueue:
	success = ck_ring_enqueue_mpmc(g_ring + channel, g_ringbuf[channel], (void*)entry);
	if (CK_CC_UNLIKELY(success == false))
	{
		//ck_pr_stall();
		ecs_os_sleep(0, backoff);
		ck_backoff_eb(&backoff);
		goto enqueue;
	}
}


static struct eg_memory_entry * dequeue_blocking(enum eg_log_channel channel)
{
	struct eg_memory_entry * entry;
	ck_backoff_t backoff = CK_BACKOFF_INITIALIZER;
	bool success;
enqueue:
	success = ck_ring_dequeue_mpmc(g_ring + channel, g_ringbuf[channel], (void*)&entry);
	if(success == false)
	{
		//ck_pr_stall();
		ecs_os_sleep(0, backoff);
		ck_backoff_eb(&backoff);
		goto enqueue;
	}
	return entry;
}






static void eg_log_msg(int32_t level, const char *file, int32_t line, const char *msg)
{
	char const * color = eg_get_log_color(level);
	char const * text = eg_get_log_title(level);
	int len = strlen(msg);
	struct eg_memory_entry * entry = eg_memory_pool_get(&pool, len);
	eg_memory_pool_assert(&pool, entry);
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

	enum eg_log_channel channel;
	if (level < 0)
	{
		sprintf(buf, "%s%s"ECS_NORMAL": %s%s:%d: %s\n", color, text, indent, file, line, msg);
		channel = EG_LOG_CHANNEL_STDERR;
	}
	else
	{
		sprintf(buf, "%s%s"ECS_NORMAL":%s %s\n", color, text, indent, msg);
		channel = EG_LOG_CHANNEL_STDOUT;
	}
	enqueue_blocking(channel, entry);

}








static bool consume_print(enum eg_log_channel channel)
{
	struct eg_memory_entry * entry;
	bool success;
	// TODO: Print a batch of messages.
	success = ck_ring_dequeue_mpmc(g_ring + channel, g_ringbuf[channel], (void*)&entry);
	if (success == true)
	{
		eg_memory_pool_assert(&pool, entry);
		char * buf = (char*)entry->memory;
		//printf("Q%i: %s\n", q, buf);
		//printf("q=%i, reuses=%i, mallocs=%i, id=%i", q, pool.amount_reuse, pool.amount_malloc, entry->id);
		//printf("%s\n", buf);
		switch (channel)
		{
		case EG_LOG_CHANNEL_STDOUT:
			fputs(buf, stdout);
			break;
		case EG_LOG_CHANNEL_STDERR:
			fputs(buf, stderr);
			break;
		default:
			break;
		}
		eg_memory_pool_reclaim(&pool, entry);
	}
	return success;
}


//static int g_stall_count = 0;
static void * eg_log_printer_thread(void * arg)
{
	ck_backoff_t backoff = CK_BACKOFF_INITIALIZER;
	while(1)
	{
		bool success = consume_print(EG_LOG_CHANNEL_STDERR) || consume_print(EG_LOG_CHANNEL_STDOUT);
		if (success == true)
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
	for (int i = 0; i < EG_LOG_CHANNEL_COUNT; ++i)
	{
		ck_ring_init(g_ring + i, EG_LOG_RING_CAPACITY);
		g_ringbuf[i] = ecs_os_calloc(sizeof(ck_ring_buffer_t) * EG_LOG_RING_CAPACITY);
	}
	eg_memory_pool_init(&pool);

	ecs_os_api.log_ = eg_log_msg;
	ecs_os_thread_t t1 = ecs_os_thread_new(eg_log_printer_thread, NULL);
}

