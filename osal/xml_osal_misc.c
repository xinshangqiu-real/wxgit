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


xss_result_t xss_task_sleep(uint32_t timeout_us)
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

xss_result_t xss_get_boottime(uint64_t *boottime)
{
    struct timespec ts;

    while (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {}
    *boottime = ((long)ts.tv_sec) * 1000000;
    *boottime += (ts.tv_nsec / 1000);

    return SUCCESS;
}
