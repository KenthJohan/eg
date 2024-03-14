#include "spam.h"
#include "msg.h"


static void * thread_spam(void *d)
{
	eglws_vhd_t *vhd = (eglws_vhd_t *)d;
	int index = 1;
	int whoami = 0;
	for (int n = 0; n < (int)LWS_ARRAY_SIZE(vhd->spam_pthread); n++) {
		if (pthread_equal(pthread_self(), vhd->spam_pthread[n])) {
			whoami = n + 1;
		}
	}
	do {
		char buf[256];
		snprintf(buf, sizeof(buf), "%s: tid: %d, msg: %d", vhd->config, whoami, index++);
		int rc = eglws_vhd_send_text(vhd, buf);
		if(rc) {
			usleep(1000*1000*2);
		}
	} while (!vhd->spam_finished);
	lwsl_notice("thread_spam %d exiting\n", whoami);
	pthread_exit(NULL);
	return NULL;
}


int spam_fini(eglws_vhd_t *vhd)
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


int spam_start(eglws_vhd_t *vhd)
{
	for (int n = 0; n < (int)LWS_ARRAY_SIZE(vhd->spam_pthread); n++) {
		if (pthread_create(&vhd->spam_pthread[n], NULL, thread_spam, vhd)) {
			lwsl_err("thread creation failed\n");
			spam_fini(vhd);
			return -1;
		}
	}
	return 0;
}

