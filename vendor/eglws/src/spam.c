#include "spam.h"



static void * thread_spam(void *d)
{
	struct per_vhost_data__minimal *vhd = (struct per_vhost_data__minimal *)d;
	struct msg amsg;
	int len = 128;
	int index = 1;
	int whoami = 0;

	for (int n = 0; n < (int)LWS_ARRAY_SIZE(vhd->spam_pthread); n++) {
		if (pthread_equal(pthread_self(), vhd->spam_pthread[n])) {
			whoami = n + 1;
		}
	}

	do {
		// don't generate output if nobody connected
		if (!vhd->pss_list) {
			goto wait;
		}

		pthread_mutex_lock(&vhd->lock_ring);

		{
			// only create if space in ringbuffer
			int n = (int)lws_ring_get_count_free_elements(vhd->ring);
			if (!n) {
				lwsl_user("dropping!\n");
				goto wait_unlock;
			}
		}

		amsg.payload = malloc((unsigned int)(LWS_PRE + len));
		if (!amsg.payload) {
			lwsl_user("OOM: dropping\n");
			goto wait_unlock;
		}

		{
			int n = lws_snprintf((char *)amsg.payload + LWS_PRE, (unsigned int)len, "%s: tid: %d, msg: %d", vhd->config, whoami, index++);
			amsg.len = (unsigned int)n;
			n = (int)lws_ring_insert(vhd->ring, &amsg, 1);
			if (n != 1) {
				__minimal_destroy_message(&amsg);
				lwsl_user("dropping!\n");
			} else {
				// This will cause a LWS_CALLBACK_EVENT_WAIT_CANCELLED in the lws service thread context.
				lws_cancel_service(vhd->context);
			}
		}

		printf("lws_ring_insert %li\n", lws_ring_get_count_waiting_elements(vhd->ring, NULL));

wait_unlock:
		pthread_mutex_unlock(&vhd->lock_ring);

wait:
		usleep(1000*1000*2);

	} while (!vhd->spam_finished);

	lwsl_notice("thread_spam %d exiting\n", whoami);

	pthread_exit(NULL);

	return NULL;
}


int spam_fini(struct per_vhost_data__minimal *vhd)
{
	void *retval;
	vhd->spam_finished = 1;
	for (int n = 0; n < (int)LWS_ARRAY_SIZE(vhd->spam_pthread); n++) {
		if (vhd->spam_pthread[n]) {
			pthread_join(vhd->spam_pthread[n], &retval);
		}
	}
	return 0;
}


int spam_start(struct per_vhost_data__minimal *vhd)
{
	for (int n = 0; n < (int)LWS_ARRAY_SIZE(vhd->spam_pthread); n++) {
		if (pthread_create(&vhd->spam_pthread[n], NULL, thread_spam, vhd)) {
			lwsl_err("thread creation failed\n");
			spam_fini(vhd);
			return -1;
		}
	}
}

