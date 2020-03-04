/*
 * us_camera.c
 *
 *  Created on: Mar 1, 2020
 *      Author: xinshangqiu
 */
#include <us_common.h>
#include <us_state.h>
#include "xml_define.h"
#include "xml_log.h"
#include "xml_osal.h"
#include "xml_hal.h"
#include "xml_hal_cam.h"


void camera_task(void *arg)
{
    xss_result_t ret = SUCCESS;
    us_info_t *us_info = NULL;
    xss_stream_buf_t *cam_buf = NULL;

    us_info = (us_info_t *)arg;

    while (1) {
        xss_osal_task_cancelpoint();
        ret = hal_cam_pop(us_info->cam_dev, &cam_buf);
        if (ret != SUCCESS) {
            US_LOGE("xss_hal_pop() failed:%d\n", ret);
            continue;
        }

        ret = hal_cam_push(us_info->cam_dev, cam_buf);
        if (ret != SUCCESS) {
            US_LOGE("xss_hal_push() failed:%d\n", ret);
        }
    }
}

xss_result_t us_cam_init(us_info_t *us_info)
{
    xss_result_t ret = SUCCESS;
    xss_osal_task_attr_t task_attr;

    ret = xss_hal_open("/dev/video0", &us_info->cam_dev, &us_info->cam_cfg);
    if (ret != SUCCESS) {
        US_LOGE("xss_hal_open(/dev/video0) failed:%d\n", ret);
        return ret;
    }

    ret = hal_cam_start(us_info->cam_dev);
    if (ret != SUCCESS) {
        US_LOGE("xss_hal_start() failed:%d\n", ret);
        return ret;
    }

    task_attr.name = "camera task";
    task_attr.param = (void *)us_info;
    task_attr.entry = camera_task;
    task_attr.detached = true;
    task_attr.priority = XSS_OSAL_PRIO_HIGH;
    task_attr.stacksize = XSS_OSAL_STACKSIZE_DEFAULT;
    ret = xss_osal_task_create(&us_info->cam_task, &task_attr);
    if (ret != SUCCESS) {
        US_LOGE("xss_osal_task_create(%s) failed:%s\n", task_attr.name, ret);
        return ret;
    }

    return SUCCESS;
}

xss_result_t us_cam_exit(us_info_t *us_info)
{
    xss_result_t ret = SUCCESS;

    ret = xss_osal_task_destroy(us_info->cam_task);
    if (ret != SUCCESS) {
        US_LOGE("xss_osal_task_destroy() failed:%s\n", ret);
        return ret;
    }

    ret = hal_cam_stop(us_info->cam_dev);
    if (ret != SUCCESS) {
        US_LOGE("xss_hal_stop() failed:%d\n", ret);
        return ret;
    }

    ret = xss_hal_close(us_info->cam_dev);
    if (ret != SUCCESS) {
        US_LOGE("xss_hal_close() failed:%d\n", ret);
        return ret;
    }

    return SUCCESS;
}
