/*
 * xml_timer.c
 *
 *  Created on: Feb 29, 2020
 *      Author: xinshangqiu
 */
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <sys/prctl.h>
#include <errno.h>

#include "xml_define.h"
#include "xml_osal.h"

typedef struct xss_osal_timer_priv {
    pthread_t thread;                /* Pthread insatnce */
    bool suspend;                    /* Timer need suspend or not */
    uint32_t repeat_cnt;            /* Timer run times count */
    struct timespec wkup_ts;        /* Which time point, timer will be wake up */
    pthread_mutex_t mutex;            /* Protect private data */
    sem_t    resume_sema;            /* Resume semaphore */
} xss_osal_timer_priv_t;

static void *timer_thread(void *arg)
{
    xss_osal_timer_t *timer = (xss_osal_timer_t *)arg;
    xss_osal_timer_priv_t *priv = (xss_osal_timer_priv_t *)timer->priv;
    xss_osal_timer_attr_t *attr = &timer->attr;

    prctl(PR_SET_NAME, timer->attr.name);

    if (attr->latency_us) {
        pthread_mutex_lock(&priv->mutex);
        while (clock_gettime(CLOCK_MONOTONIC, &priv->wkup_ts) < 0) {}
        priv->wkup_ts.tv_sec += (attr->latency_us / 1000000);
        priv->wkup_ts.tv_nsec += (attr->latency_us % 1000000) * 1000;
        while (priv->wkup_ts.tv_nsec >= 1000000000) {
            priv->wkup_ts.tv_sec++;
            priv->wkup_ts.tv_nsec -= 1000000000;
        }
        pthread_mutex_unlock(&priv->mutex);

        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &priv->wkup_ts, NULL);
    }

    while ((attr->repeat_cnt == XSS_OSAL_REPEAT_FOREVER) || (priv->repeat_cnt < attr->repeat_cnt)) {
        if (priv->suspend) {
            sem_wait(&priv->resume_sema);
        }

        pthread_mutex_lock(&priv->mutex);
        priv->repeat_cnt++;
        while (clock_gettime(CLOCK_MONOTONIC, &priv->wkup_ts) < 0) {}
        priv->wkup_ts.tv_sec += (attr->timeout_us / 1000000);
        priv->wkup_ts.tv_nsec += (attr->timeout_us % 1000000) * 1000;
        while (priv->wkup_ts.tv_nsec >= 1000000000) {
            priv->wkup_ts.tv_sec++;
            priv->wkup_ts.tv_nsec -= 1000000000;
        }
        pthread_mutex_unlock(&priv->mutex);

        if (attr->entry) {
            attr->entry(attr->param);
        }

        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &priv->wkup_ts, NULL);
    }

    return NULL;
}

xss_result_t xss_osal_timer_create(xss_osal_timer_t **timer, xss_osal_timer_attr_t *attr)
{
    xss_osal_timer_t *xtimer = NULL;
    xss_osal_timer_priv_t *priv = NULL;
    pthread_attr_t pthread_attr;
    struct sched_param sched_param;
    void *mem_base = NULL;

    if (timer == NULL || attr == NULL) {
        return ERR_FAULT;
    }

    mem_base = calloc(sizeof(xss_osal_timer_t) + sizeof(xss_osal_timer_priv_t), 1);
    if (mem_base == NULL) {
        return ERR_NOMEM;
    }

    xtimer = (xss_osal_timer_t *)mem_base;
    memcpy(&xtimer->attr, attr, sizeof(xss_osal_timer_attr_t));
    xtimer->priv = mem_base + sizeof(xss_osal_timer_t);
    priv = (xss_osal_timer_priv_t *)xtimer->priv;

    sem_init(&priv->resume_sema, 0, 0);
    pthread_mutex_init(&priv->mutex, NULL);

    pthread_attr_init(&pthread_attr);
    pthread_attr_setdetachstate(&pthread_attr, PTHREAD_CREATE_JOINABLE);
    pthread_attr_setstacksize(&pthread_attr, XSS_OSAL_STACKSIZE_DEFAULT);
    pthread_attr_getschedparam(&pthread_attr, &sched_param);
    sched_param.sched_priority = attr->priority;
    pthread_attr_setschedparam(&pthread_attr, &sched_param);
    if (attr->priority) {
        pthread_attr_setschedpolicy(&pthread_attr, SCHED_RR);
    } else {
        pthread_attr_setschedpolicy(&pthread_attr, SCHED_OTHER);
    }
    pthread_create(&priv->thread, &pthread_attr, timer_thread, xtimer);
    pthread_attr_destroy(&pthread_attr);
    *timer = xtimer;

    return SUCCESS;
}

xss_result_t xss_osal_timer_destroy(xss_osal_timer_t *timer)
{
    xss_osal_timer_priv_t *priv = NULL;

    if (timer == NULL || timer->priv == NULL) {
        return ERR_FAULT;
    }

    priv = (xss_osal_timer_priv_t *)timer->priv;

    /* After pthread_cancel() call, thread will exit later */
    if (pthread_cancel(priv->thread) != 0) {
        return ERR_BUSY;
    }

    /* Wait timer's pthread exit */
    if (pthread_join(priv->thread, NULL) < 0) {
        return ERR_FAILURE;
    }

    pthread_mutex_destroy(&priv->mutex);
    sem_destroy(&priv->resume_sema);
    free(timer);

    return SUCCESS;
}

xss_result_t xss_osal_timer_suspend(xss_osal_timer_t *timer)
{
    xss_osal_timer_priv_t *priv = NULL;

    if (timer == NULL || timer->priv == NULL) {
        return ERR_FAULT;
    }

    priv = (xss_osal_timer_priv_t *)timer->priv;
    pthread_mutex_lock(&priv->mutex);
    priv->suspend = true;
    pthread_mutex_unlock(&priv->mutex);

    return SUCCESS;
}

xss_result_t xss_osal_timer_resume(xss_osal_timer_t *timer)
{
    xss_osal_timer_priv_t *priv = NULL;

    if (timer == NULL || timer->priv == NULL) {
        return ERR_FAULT;
    }

    priv = (xss_osal_timer_priv_t *)timer->priv;
    if (priv->suspend) {
        pthread_mutex_lock(&priv->mutex);
        priv->suspend = false;
        sem_post(&priv->resume_sema);
        pthread_mutex_unlock(&priv->mutex);
    }

    return SUCCESS;
}

xss_result_t xss_osal_timer_update(xss_osal_timer_t *timer, uint32_t timeout_us)
{
    xss_osal_timer_priv_t *priv = NULL;

    if (timer == NULL || timer->priv == NULL) {
        return ERR_FAULT;
    }

    priv = (xss_osal_timer_priv_t *)timer->priv;
    pthread_mutex_lock(&priv->mutex);
    timer->attr.timeout_us = timeout_us;
    pthread_mutex_unlock(&priv->mutex);

    return SUCCESS;
}
