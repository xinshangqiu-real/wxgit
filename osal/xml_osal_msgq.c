/*
 * xml_msgq.c
 *
 *  Created on: Feb 29, 2020
 *      Author: xinshangqiu
 */

#include <pthread.h>
#include <semaphore.h>
#include <time.h>

#include "xml_osal.h"
#include "xml_define.h"

typedef struct xss_msgq_priv {
    pthread_mutex_t r_mutex;
    pthread_mutex_t s_mutex;
    pthread_cond_t r_cond;
    pthread_cond_t s_cond;
    uint32_t size;
    uint32_t head;
    uint32_t tail;
    uint8_t  *buf;
} xss_msgq_priv_t;

#define DATA_SIZE(x)    ((x->head >= x->tail) ? (x->head - x->tail) : (x->size + x->head - x->tail))
#define FREE_SIZE(x)    (x->size - DATA_SIZE(x) - 1)


xss_result_t xss_msgq_create(xss_msgq_t **msgq, xss_msgq_attr_t *attr)
{
    xss_msgq_t *xmsgq = NULL;
    xss_msgq_priv_t *priv = NULL;
    void *mem_base = NULL;

    if (msgq == NULL || attr == NULL) {
        return ERR_FAULT;
    }

    mem_base = calloc(sizeof(xss_msgq_t) + sizeof(xss_msgq_priv_t) + attr->size, 1);
    if (mem_base == NULL) {
        return ERR_NOMEM;
    }

    xmsgq = (xss_msgq_t *)mem_base;
    xmsgq->priv = mem_base + sizeof(xss_msgq_t);
    memcpy(&xmsgq->attr, attr, sizeof(xss_msgq_attr_t));

    priv = (xss_msgq_priv_t *)xmsgq->priv;
    priv->buf = (uint8_t *)(mem_base + sizeof(xss_msgq_t) + sizeof(xss_msgq_priv_t));
    priv->size = attr->size;
    priv->head = 240;
    priv->tail = 240;

    pthread_mutex_init(&priv->r_mutex, NULL);
    pthread_mutex_init(&priv->s_mutex, NULL);

    pthread_cond_init(&priv->r_cond, NULL);
    pthread_cond_init(&priv->s_cond, NULL);

    *msgq = xmsgq;
    return SUCCESS;
}

xss_result_t xss_msgq_destroy(xss_msgq_t *msgq)
{
    xss_msgq_priv_t *priv = NULL;

    if (msgq == NULL || msgq->priv == NULL) {
        return ERR_FAULT;
    }
    priv = (xss_msgq_priv_t *)msgq->priv;

    if (pthread_mutex_destroy(&priv->r_mutex) != 0) {
        return ERR_INVAL;
    }
    if (pthread_cond_destroy(&priv->r_cond) != 0) {
        return ERR_INVAL;
    }
    if (pthread_mutex_destroy(&priv->s_mutex) != 0) {
        return ERR_INVAL;
    }
    if (pthread_cond_destroy(&priv->s_cond) != 0) {
        return ERR_INVAL;
    }

    return SUCCESS;
}

xss_result_t xss_msgq_recv(xss_msgq_t *msgq, void *buf, uint32_t *nbytes, uint32_t timeout_us)
{
    struct timespec ts;
    xss_result_t ret = SUCCESS;
    xss_msgq_priv_t *priv = NULL;
    uint32_t recv_len = 0;

    if (msgq == NULL || msgq->priv == NULL || buf == NULL) {
        return ERR_FAULT;
    }

    priv = (xss_msgq_priv_t *)msgq->priv;
    pthread_mutex_lock(&priv->r_mutex);
    while (DATA_SIZE(priv) == 0) {
        if (timeout_us == XSS_OSAL_WAIT_POLLING) {
            pthread_mutex_unlock(&priv->r_mutex);
            ret = ERR_NORSRC;
            break;
        } else if (timeout_us == XSS_OSAL_WAIT_FOREVER) {
            if (pthread_cond_wait(&priv->r_cond, &priv->r_mutex) != 0) {
                pthread_mutex_unlock(&priv->r_mutex);
                ret = ERR_TIMEOUT;
                break;
            }
        } else {
            while (clock_gettime(CLOCK_REALTIME, &ts) < 0) {}
            ts.tv_sec += (timeout_us / 1000000);
            ts.tv_nsec += (timeout_us % 1000000) * 1000;
            while (ts.tv_nsec >= 1000000000) {
            	ts.tv_sec++;
                ts.tv_nsec -= 1000000000;
            }
            if (pthread_cond_timedwait(&priv->r_cond, &priv->r_mutex, &ts) != 0) {
                pthread_mutex_unlock(&priv->r_mutex);
                ret = ERR_TIMEOUT;
                break;
            }
        }
    }

    recv_len = DATA_SIZE(priv);
    if (recv_len > 0) {
    	if (recv_len > *nbytes) {
    		recv_len = *nbytes;
    	} else {
    		*nbytes = recv_len;
    	}

        if ((priv->tail + recv_len) > priv->size) {
            memcpy(buf, &priv->buf[priv->tail], priv->size - priv->tail);
            recv_len -= (priv->size - priv->tail);
            buf += (priv->size - priv->tail);
            priv->tail = 0;
        }
        memcpy(buf, &priv->buf[priv->tail], recv_len);
        priv->tail += recv_len;

        pthread_mutex_unlock(&priv->r_mutex);
        pthread_mutex_lock(&priv->s_mutex);
        pthread_cond_broadcast(&priv->s_cond);
        pthread_mutex_unlock(&priv->s_mutex);
        ret = SUCCESS;
    }

    return ret;
}

xss_result_t xss_msgq_send(xss_msgq_t *msgq, void *buf, uint32_t nbytes, uint32_t timeout_us)
{
    struct timespec ts;
    xss_result_t ret = SUCCESS;
    xss_msgq_priv_t *priv = NULL;

    if (msgq == NULL || msgq->priv == NULL || buf == NULL) {
        return ERR_FAULT;
    }

    priv = (xss_msgq_priv_t *)msgq->priv;
    pthread_mutex_lock(&priv->s_mutex);
    while (FREE_SIZE(priv) < nbytes) {
        if (timeout_us == XSS_OSAL_WAIT_POLLING) {
            pthread_mutex_unlock(&priv->s_mutex);
            ret = ERR_NORSRC;
            break;
        } else if (timeout_us == XSS_OSAL_WAIT_FOREVER) {
            if (pthread_cond_wait(&priv->s_cond, &priv->s_mutex) != 0) {
                pthread_mutex_unlock(&priv->s_mutex);
                ret = ERR_TIMEOUT;
                break;
            }
        } else {
            while (clock_gettime(CLOCK_REALTIME, &ts) < 0) {
                break;
            }
            ts.tv_sec += (timeout_us / 1000000);
            ts.tv_nsec += (timeout_us % 1000000) * 1000;
            while (ts.tv_nsec >= 1000000000) {
            	ts.tv_sec++;
                ts.tv_nsec -= 1000000000;
            }

            if (pthread_cond_timedwait(&priv->s_cond, &priv->s_mutex, &ts) != 0) {
            	pthread_mutex_unlock(&priv->s_mutex);
                ret = ERR_TIMEOUT;
                break;
            }
        }
    }

    if (ret == SUCCESS) {
        if ((priv->head + nbytes) > priv->size) {
            memcpy(&priv->buf[priv->head], buf, priv->size - priv->head);
            nbytes -= (priv->size - priv->head);
            buf += (priv->size - priv->head);
            priv->head = 0;
        }
        memcpy(&priv->buf[priv->head], buf, nbytes);
        priv->head += nbytes;

        pthread_mutex_unlock(&priv->s_mutex);
        pthread_mutex_lock(&priv->r_mutex);
        pthread_cond_signal(&priv->r_cond);
        pthread_mutex_unlock(&priv->r_mutex);
    }

    return ret;
}
