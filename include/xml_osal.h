#ifndef __XML_MSGQ_H_
#define __XML_MSGQ_H_

#include "xml_define.h"

/*
 * wait timeout macro
 * */
#define XSS_OSAL_WAIT_POLLING			(uint32_t)(0)
#define XSS_OSAL_WAIT_FOREVER			(uint32_t)(-1)

/*
 * timer repeat timer macro
 * */
#define XSS_OSAL_REPEAT_FOREVER			(uint32_t)(0)

/*
 * task or timer priority in thread schedule
 * */
#define XSS_OSAL_PRIO_DEFAULT			0
#define XSS_OSAL_PRIO_NORMAL            5
#define XSS_OSAL_PRIO_LOW				10
#define XSS_OSAL_PRIO_MID               15
#define XSS_OSAL_PRIO_HIGH              20
#define XSS_OSAL_PRIO_TIML				25
#define XSS_OSAL_PRIO_TIMM				30
#define XSS_OSAL_PRIO_TIMH				35
#define XSS_OSAL_PRIO_INT				50
#define XSS_OSAL_PRIO_HIGHEST          	99

/*
 * stack size for task or timer
 * Set stacksize to 0, linux system will use deault stack size
 * */
#define XSS_OSAL_STACKSIZE_DEFAULT      0


/*
 ============================================================================
 Description : msgq functions
 ============================================================================
 */

typedef struct xss_msgq_attr {
	char *name;
	uint32_t size;
} xss_msgq_attr_t;

typedef struct xss_msgq {
	xss_msgq_attr_t attr;
	void *priv;
} xss_msgq_t;


xss_result_t xss_msgq_create(xss_msgq_t **msgq, xss_msgq_attr_t *attr);
xss_result_t xss_msgq_destroy(xss_msgq_t *msgq);
xss_result_t xss_msgq_recv(xss_msgq_t *msgq, void *buf, uint32_t *nbytes, uint32_t timeout_us);
xss_result_t xss_msgq_send(xss_msgq_t *msgq, void *buf, uint32_t nbytes, uint32_t timeout_us);

/*
 ============================================================================
 Description : mutex functions
 ============================================================================
 */
typedef struct xss_mutex_attr {
    char *name;
} xss_mutex_attr_t;

typedef struct xss_mutex {
    xss_mutex_attr_t attr;
    void *priv;
} xss_mutex_t;

xss_result_t xss_mutex_create(xss_mutex_t **mutex, xss_mutex_attr_t *attr);
xss_result_t xss_mutex_destroy(xss_mutex_t *mutex);
xss_result_t xss_mutex_unlock(xss_mutex_t *mutex);
xss_result_t xss_mutex_lock(xss_mutex_t *mutex, uint32_t timeout_us);

/*
 ============================================================================
 Description : semaphore functions
 ============================================================================
 */
typedef struct xss_sema_attr {
    char *name;
    uint32_t init_value;
} xss_sema_attr_t;

typedef struct xss_sema {
    xss_sema_attr_t attr;
    void *priv;
} xss_sema_t;

xss_result_t xss_sema_create(xss_sema_t **sema, xss_sema_attr_t *attr);
xss_result_t xss_sema_destroy(xss_sema_t *sema);
xss_result_t xss_sema_post(xss_sema_t *sema);
xss_result_t xss_sema_wait(xss_sema_t *sema, uint32_t timeout_us);

/*
 ============================================================================
 Description : task functions
 ============================================================================
 */
typedef struct xss_task_attr {
    char *name;
    void *param;
    void (*entry)(void *);
    int32_t stacksize;
    int32_t priority;
    int32_t detached;
} xss_task_attr_t;

typedef struct xss_task {
    xss_task_attr_t attr;
    void *priv;
} xss_task_t;

xss_result_t xss_task_create(xss_task_t **task, xss_task_attr_t *attr);
xss_result_t xss_task_destroy(xss_task_t *task);
xss_result_t xss_task_setprio(xss_task_t *task, uint32_t priority);
xss_result_t xss_task_is_exit(xss_task_t *task);
xss_result_t xss_task_sleep(uint32_t timeout_us);
xss_result_t xss_osal_get_boottime(uint64_t *boottime);

/*
 ============================================================================
 Description : timer functions
 ============================================================================
 */
typedef struct xss_timer_attr {
	char *name;
	void *param;
	void (*entry)(void *);
	uint32_t priority;
	uint32_t timeout_us;
	uint32_t latency_us;
	uint32_t repeat_cnt;
} xss_timer_attr_t;

typedef struct xss_timer {
	xss_timer_attr_t attr;
	void *priv;
} xss_timer_t;


xss_result_t xss_timer_create(xss_timer_t **timer, xss_timer_attr_t *attr);
xss_result_t xss_timer_destroy(xss_timer_t *timer);
xss_result_t xss_timer_restart(xss_timer_t *timer);
xss_result_t xss_timer_suspend(xss_timer_t *timer);
xss_result_t xss_timer_resume(xss_timer_t *timer);
xss_result_t xss_timer_update(xss_timer_t *timer, uint32_t timeout_us);

/*
 ============================================================================
 Description : misc functions
 ============================================================================
 */
#endif
