#include <time.h>
#include <semaphore.h>

#include "xml_define.h"
#include "xml_osal.h"


typedef struct xss_sema_priv {
    sem_t sema;
} xss_sema_priv_t;


xss_result_t xss_sema_create(xss_sema_t **sema, xss_sema_attr_t *attr)
{
    xss_sema_t *xsema = NULL;
    xss_sema_priv_t *priv = NULL;
    void *mem_base = NULL;

	if (sema == NULL || attr == NULL) {
		return ERR_FAULT;
	}

    mem_base = calloc(sizeof(xss_sema_t) + sizeof(xss_sema_priv_t), 1);
    if (mem_base == NULL) {
        return ERR_NOMEM;
    }

    xsema = (xss_sema_t *)mem_base;
    xsema->priv = mem_base + sizeof(xss_sema_t);
    memcpy(&xsema->attr, attr, sizeof(xss_sema_attr_t));
    priv = (xss_sema_priv_t *)xsema->priv;
    sem_init(&priv->sema, 0, attr->init_value);

    *sema = xsema;

    return SUCCESS;
}

xss_result_t xss_sema_destroy(xss_sema_t *sema)
{
	xss_sema_priv_t *priv = NULL;

    if (sema == NULL || sema->priv == NULL) {
        return ERR_FAULT;
    }

    priv = (xss_sema_priv_t *)sema->priv;
    sem_destroy(&priv->sema);
    free(sema);

    return SUCCESS;
}

xss_result_t xss_sema_post(xss_sema_t *sema)
{
	xss_sema_priv_t *priv = NULL;

    if (sema == NULL || sema->priv == NULL) {
        return ERR_FAULT;
    }

    priv = (xss_sema_priv_t *)sema->priv;
    sem_post(&priv->sema);

    return SUCCESS;
}

xss_result_t xss_sema_wait(xss_sema_t *sema, uint32_t timeout_us)
{
	struct timespec ts;
	xss_sema_priv_t *priv = NULL;

    if (sema == NULL || sema->priv == NULL) {
        return ERR_FAULT;
    }

    priv = (xss_sema_priv_t *)sema->priv;
    if (timeout_us == XSS_OSAL_WAIT_POLLING) {
		if (sem_trywait(&priv->sema) != 0) {
			return ERR_TIMEOUT;
		}
	} else if (timeout_us == XSS_OSAL_WAIT_FOREVER) {
		if (sem_wait(&priv->sema) != 0) {
			return ERR_FAILURE;
		}
	} else {
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
