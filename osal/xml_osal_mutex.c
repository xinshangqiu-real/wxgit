#include <time.h>
#include <semaphore.h>

#include "xml_define.h"
#include "xml_osal.h"


typedef struct xss_osal_mutex_priv {
    sem_t sema;
} xss_osal_mutex_priv_t;


xss_result_t xss_osal_mutex_create(xss_osal_mutex_t **mutex, xss_osal_mutex_attr_t *attr)
{
    xss_osal_mutex_t *xmutex = NULL;
    xss_osal_mutex_priv_t *priv = NULL;
    void *mem_base = NULL;

    mem_base = calloc(sizeof(xss_osal_mutex_t) + sizeof(xss_osal_mutex_priv_t), 1);
    if (mem_base == NULL) {
        return ERR_NOMEM;
    }

    xmutex = (xss_osal_mutex_t *)mem_base;
    xmutex->priv = mem_base + sizeof(xss_osal_mutex_t);
    memcpy(&xmutex->attr, attr, sizeof(xss_osal_mutex_attr_t));
    priv = (xss_osal_mutex_priv_t *)xmutex->priv;
    sem_init(&priv->sema, 0, 1);

    *mutex = xmutex;

    return SUCCESS;
}
xss_result_t xss_osal_mutex_destroy(xss_osal_mutex_t *mutex)
{
    xss_osal_mutex_priv_t *priv = NULL;

    if (mutex == NULL || mutex->priv) {
        return ERR_FAULT;
    }

    priv = (xss_osal_mutex_priv_t *)mutex->priv;
    sem_destroy(&priv->sema);
    free(mutex);

    return SUCCESS;
}

xss_result_t xss_osal_mutex_unlock(xss_osal_mutex_t *mutex)
{
    xss_osal_mutex_priv_t *priv = NULL;

    if (mutex == NULL || mutex->priv == NULL) {
        return ERR_FAULT;
    }

    priv = (xss_osal_mutex_priv_t *)mutex->priv;
    sem_post(&priv->sema);

    return SUCCESS;
}

xss_result_t xss_osal_mutex_lock(xss_osal_mutex_t *mutex, uint32_t timeout_us)
{
    struct timespec ts;
    xss_osal_mutex_priv_t *priv = NULL;

    if (mutex == NULL || mutex->priv == NULL) {
        return ERR_FAULT;
    }

    priv = (xss_osal_mutex_priv_t *)mutex->priv;
    if (timeout_us == XSS_OSAL_WAIT_POLLING) {
        if (sem_trywait(&priv->sema) != 0) {
            return ERR_TIMEOUT;
        }
    } else if (timeout_us == XSS_OSAL_WAIT_FOREVER) {
        if (sem_wait(&priv->sema) != 0) {
            return ERR_FAILURE;
        }
    } else  {
        while (clock_gettime(CLOCK_REALTIME, &ts) < 0) {}
        ts.tv_sec += (timeout_us / 1000000);
        ts.tv_nsec += (timeout_us % 1000000) * 1000;
        while (ts.tv_nsec >= 1000000000) {
            ts.tv_sec++;
            ts.tv_nsec -= 1000000000;
        }
        if (sem_timedwait(&priv->sema, &ts) != 0) {
            return ERR_TIMEOUT;
        }
    }

    return SUCCESS;
}
