/*
 * xml_osal_misc.c
 *
 *  Created on: Mar 1, 2020
 *      Author: xinshangqiu
 */

#include <pthread.h>
#include <time.h>

#include "xml_define.h"
#include "xml_osal.h"


xss_result_t xss_osal_sleep(uint32_t timeout_us)
{
    struct timespec ts;

    while (clock_gettime(CLOCK_MONOTONIC, &ts) < 0) {}
    ts.tv_sec += (timeout_us / 1000000);
    ts.tv_nsec += (timeout_us % 1000000) * 1000;
    while (ts.tv_nsec >= 1000000000) {
        ts.tv_sec++;
        ts.tv_nsec -= 1000000000;
    }

    return clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &ts, NULL);
}

xss_result_t xss_osal_get_boottime(uint64_t *boottime_us)
{
    struct timespec ts;

    while (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {}
    *boottime_us = ((long)ts.tv_sec) * 1000000;
    *boottime_us += (ts.tv_nsec / 1000);

    return SUCCESS;
}

xss_result_t xss_osal_get_walltime(uint64_t *walltime_us)
{
    struct timespec ts;

    while (clock_gettime(CLOCK_REALTIME, &ts) != 0) {}
    *walltime_us = ((long)ts.tv_sec) * 1000000;
    *walltime_us += (ts.tv_nsec / 1000);

    return SUCCESS;
}
