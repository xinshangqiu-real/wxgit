#ifndef __XML_MSGQ_H_
#define __XML_MSGQ_H_

#include "xml_define.h"

/*
 * wait timeout macro
 * */
#define XSS_OSAL_WAIT_POLLING            (uint32_t)(0)
#define XSS_OSAL_WAIT_FOREVER            (uint32_t)(-1)

/*
 * timer repeat timer macro
 * */
#define XSS_OSAL_REPEAT_FOREVER            (uint32_t)(0)

/*
 * task or timer priority in thread schedule
 * */
#define XSS_OSAL_PRIO_DEFAULT            0
#define XSS_OSAL_PRIO_NORMAL            5
#define XSS_OSAL_PRIO_LOW                10
#define XSS_OSAL_PRIO_MID               15
#define XSS_OSAL_PRIO_HIGH              20
#define XSS_OSAL_PRIO_TIML              25
#define XSS_OSAL_PRIO_TIMM                30
#define XSS_OSAL_PRIO_TIMH                35
#define XSS_OSAL_PRIO_INT                50
#define XSS_OSAL_PRIO_HIGHEST              99

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

/*
 * message queue's attribute
 * */
typedef struct xss_osal_msgq_attr {
    char *name;
    uint32_t size;
} xss_osal_msgq_attr_t;

/*
 * message queue's structure
 * */
typedef struct xss_osal_msgq {
    xss_osal_msgq_attr_t attr;
    void *priv;
} xss_osal_msgq_t;

/*
 * Create a meassge queue
 * @param msgq
 *             A sencodary pointer, use is to store the created msgq's memory address.
 * @param attr
 *             Location of message queue's attribute
 * @return
 *             On success, xss_osal_msgq_create() returns SUCCESS.
 * */
xss_result_t xss_osal_msgq_create(xss_osal_msgq_t **msgq, xss_osal_msgq_attr_t *attr);

/*
 * Destroy a message queue
 * @param msgq
 *             A pointer to message queue, which will be destroyed at later.
 * @return
 *             On success, xss_osal_msgq_destroy() return SUCCESS.
 * */
xss_result_t xss_osal_msgq_destroy(xss_osal_msgq_t *msgq);

/*
 * Attempts to recv up to *nbytes bytes form message queue.
 * When message queue has no data, the recv would be blocked.
 * @param msgq
 * @param buf
 * @param nbytes
 * @param timeout_us
 * @return
 *             On success, xss_osal_msgq_recv() returns SUCCESS.
 * @notes
 *             After xss_osal_msgq_recv() returned, check nbytes's value.
 *             It could be changed.
 * */
xss_result_t xss_osal_msgq_recv(xss_osal_msgq_t *msgq, void *buf, uint32_t *nbytes, uint32_t timeout_us);

/*
 * Attempts to send up to nbytes bytes to message queue.
 * When message queue has no free space, the send would be blocked.
 * @param msgq
 * @param buf
 * @param nbytes
 * @param timeout_us
 * @return
 *             On success, xss_osal_msgq_send() returns SUCCESS.
 * */
xss_result_t xss_osal_msgq_send(xss_osal_msgq_t *msgq, void *buf, uint32_t nbytes, uint32_t timeout_us);

/*
 ============================================================================
 Description : mutex functions
 ============================================================================
 */
typedef struct xss_osal_mutex_attr {
    char *name;
} xss_osal_mutex_attr_t;

typedef struct xss_osal_mutex {
    xss_osal_mutex_attr_t attr;
    void *priv;
} xss_osal_mutex_t;

/*
 * Create a mutex
 * @param mutex
 *             A sencodary pointer, use is to store the created mutex's memory address.
 * @param attr
 *             Location of mutex's attribute
 * @return
 *             On success, xss_osal_mutex_create() returns SUCCESS.
 * */
xss_result_t xss_osal_mutex_create(xss_osal_mutex_t **mutex, xss_osal_mutex_attr_t *attr);

/*
 * Destroy a mutex queue
 * @param mutex
 *             A pointer to mutex, which will be destroyed at later.
 * @return
 *             On success, xss_osal_mutex_destroy() return SUCCESS.
 * */
xss_result_t xss_osal_mutex_destroy(xss_osal_mutex_t *mutex);
xss_result_t xss_osal_mutex_unlock(xss_osal_mutex_t *mutex);
xss_result_t xss_osal_mutex_lock(xss_osal_mutex_t *mutex, uint32_t timeout_us);

/*
 ============================================================================
 Description : semaphore functions
 ============================================================================
 */
typedef struct xss_osal_sema_attr {
    char *name;
    uint32_t init_value;
} xss_osal_sema_attr_t;

typedef struct xss_osal_sema {
    xss_osal_sema_attr_t attr;
    void *priv;
} xss_osal_sema_t;

/*
 * Create a semaphore
 * @param mutex
 *             A sencodary pointer, use is to store the created semaphore's memory address.
 * @param attr
 *             Location of semaphore's attribute
 * @return
 *             On success, xss_osal_sema_create() returns SUCCESS.
 * */
xss_result_t xss_osal_sema_create(xss_osal_sema_t **sema, xss_osal_sema_attr_t *attr);

/*
 * Destroy a mutex
 * @param mutex
 *             A pointer to mutex, which will be destroyed at later.
 * @return
 *             On success, xss_osal_mutex_destroy() return SUCCESS.
 * */
xss_result_t xss_osal_sema_destroy(xss_osal_sema_t *sema);
xss_result_t xss_osal_sema_post(xss_osal_sema_t *sema);
xss_result_t xss_osal_sema_wait(xss_osal_sema_t *sema, uint32_t timeout_us);

/*
 ============================================================================
 Description : task functions
 ============================================================================
 */
typedef struct xss_osal_task_attr {
    char *name;
    void *param;
    void (*entry)(void *);
    int32_t stacksize;
    int32_t priority;
    int32_t detached;
} xss_osal_task_attr_t;

typedef struct xss_osal_task {
    xss_osal_task_attr_t attr;
    void *priv;
} xss_osal_task_t;

/*
 * Create a task
 * @param msgq
 *             A sencodary pointer, use is to store the created task's memory address.
 * @param attr
 *             Location of task's attribute
 * @return
 *             On success, xss_osal_task_create() returns SUCCESS.
 * */
xss_result_t xss_osal_task_create(xss_osal_task_t **task, xss_osal_task_attr_t *attr);

/*
 * Destroy a task
 * @param task
 *             A pointer to task, which will be destroyed at later.
 * @return
 *             On success, xss_osal_task_destroy() return SUCCESS.
 * */
xss_result_t xss_osal_task_destroy(xss_osal_task_t *task);
xss_result_t xss_osal_task_setprio(xss_osal_task_t *task, uint32_t priority);
xss_result_t xss_osal_task_is_exit(xss_osal_task_t *task);
xss_result_t xss_osal_task_cancelpoint(void);

/*
 ============================================================================
 Description : timer functions
 ============================================================================
 */
typedef struct xss_osal_timer_attr {
    char *name;
    void *param;
    void (*entry)(void *);
    uint32_t priority;
    uint32_t timeout_us;
    uint32_t latency_us;
    uint32_t repeat_cnt;
} xss_osal_timer_attr_t;

typedef struct xss_osal_timer {
    xss_osal_timer_attr_t attr;
    void *priv;
} xss_osal_timer_t;


/*
 * Create a timer
 * @param msgq
 *             A sencodary pointer, use is to store the created timer's memory address.
 * @param attr
 *             Location of timer's attribute
 * @return
 *          On success, xss_osal_timer_create() returns SUCCESS.
 * */
xss_result_t xss_osal_timer_create(xss_osal_timer_t **timer, xss_osal_timer_attr_t *attr);

/*
 * Destroy a timer
 * @param timer
 *             A pointer to timer, which will be destroyed at later.
 * @return
 *             On success, xss_osal_timer_destroy() return SUCCESS.
 * */
xss_result_t xss_osal_timer_destroy(xss_osal_timer_t *timer);
xss_result_t xss_osal_timer_restart(xss_osal_timer_t *timer);
xss_result_t xss_osal_timer_suspend(xss_osal_timer_t *timer);
xss_result_t xss_osal_timer_resume(xss_osal_timer_t *timer);
xss_result_t xss_osal_timer_update(xss_osal_timer_t *timer, uint32_t timeout_us);

/*
 ============================================================================
 Description : misc functions
 ============================================================================
 */
xss_result_t xss_osal_sleep(uint32_t timeout_us);
xss_result_t xss_osal_get_boottime(uint64_t *boottime_us);
xss_result_t xss_osal_get_walltime(uint64_t *walltime_us);

#endif
