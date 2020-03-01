#include <pthread.h>
#include <time.h>
#include <sys/prctl.h>
#include <errno.h>
#include <signal.h>

#include "xml_define.h"
#include "xml_osal.h"


typedef struct xss_task_priv {
	pthread_t thread;
} xss_task_priv_t;


static void *task_thread(void *arg)
{
	xss_task_t *task = (xss_task_t *)arg;

	prctl(PR_SET_NAME, task->attr.name);
	task->attr.entry(task->attr.param);

	/* Detached mode only */
	if (task->attr.detached) {
		free(task);
	}

	return NULL;
}

xss_result_t xss_task_create(xss_task_t **task, xss_task_attr_t *attr)
{
	xss_task_t *xtask = NULL;
	xss_task_priv_t *priv = NULL;
	pthread_attr_t pthread_attr;
	struct sched_param sched_param;
	void *mem_base = NULL;

	if (task == NULL || attr == NULL) {
		return ERR_FAULT;
	}

	mem_base = calloc(sizeof(xss_task_t) + sizeof(xss_task_priv_t), 1);
	if (mem_base == NULL) {
		return ERR_NOMEM;
	}

	xtask = (xss_task_t *)mem_base;
	memcpy(&xtask->attr, attr, sizeof(xss_task_attr_t));
	xtask->priv = mem_base + sizeof(xss_task_t);
	priv = (xss_task_priv_t *)xtask->priv;

	pthread_attr_init(&pthread_attr);
	if (attr->detached) {
		pthread_attr_setdetachstate(&pthread_attr, PTHREAD_CREATE_DETACHED);
	} else {
		pthread_attr_setdetachstate(&pthread_attr, PTHREAD_CREATE_JOINABLE);
	}

	pthread_attr_setstacksize(&pthread_attr, attr->stacksize);
	pthread_attr_getschedparam(&pthread_attr, &sched_param);
	sched_param.sched_priority = attr->priority;
	pthread_attr_setschedparam(&pthread_attr, &sched_param);
	if (attr->priority) {
		pthread_attr_setschedpolicy(&pthread_attr, SCHED_RR);
	} else {
		pthread_attr_setschedpolicy(&pthread_attr, SCHED_OTHER);
	}
	pthread_create(&priv->thread, &pthread_attr, task_thread, xtask);
	pthread_attr_destroy(&pthread_attr);
	*task = xtask;

	return SUCCESS;
}

xss_result_t xss_task_destroy(xss_task_t *task)
{
	xss_task_priv_t *priv = NULL;

	if (task == NULL || task->priv == NULL) {
		return ERR_FAULT;
	}

	priv = (xss_task_priv_t *)task->priv;

	/* After pthread_cancel() call, thread will exit later */
	if (pthread_cancel(priv->thread) != 0) {
		return ERR_BUSY;
	}

	/*
	 * Thread in detached state,
	 * Resource willbe  recycled after exit automatically
	 * So, we need to recycle resource in joinable state
	 * */
	if (!task->attr.detached) {
		if (pthread_join(priv->thread, NULL) < 0) {
			return ERR_FAILURE;
		}
		free(task);
	}

	return SUCCESS;
}

xss_result_t xss_task_setprio(xss_task_t *task, uint32_t priority)
{
	xss_task_priv_t *priv = NULL;
	pthread_t thread;

	if (task == NULL || task->priv == NULL) {
		thread = pthread_self();
	} else {
		priv = (xss_task_priv_t *)task->priv;
		thread = priv->thread;
	}

	return (pthread_setschedprio(thread, priority) != 0) ? ERR_FAILURE : SUCCESS;
}

xss_result_t xss_task_is_exit(xss_task_t *task)
{
	xss_task_priv_t *priv = NULL;
	int32_t ret = 0;

	if (task == NULL || task->priv == NULL) {
		return ERR_FAULT;
	}

	priv = (xss_task_priv_t *)task->priv;
	ret = pthread_kill(priv->thread, 0);
	if (ret == ESRCH) {
		/* Thread has been exit, or not exist */
		return ERR_FAILURE;
	} else if ( ret == EINVAL) {
		/* Signal invalid */
		return ERR_INVAL;
	}
	/* Thread is running */
	return SUCCESS;
}
