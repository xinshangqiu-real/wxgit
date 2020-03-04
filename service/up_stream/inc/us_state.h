/*
 * es_entry.c
 *
 *  Created on: Mar 1, 2020
 *      Author: xinshangqiu
 */

#ifndef US_STATE_H_
#define US_STATE_H_

#include "xml_define.h"
#include "xml_log.h"
#include "xml_osal.h"
#include "xml_hal.h"
#include "xml_hal_cam.h"


typedef struct us_info {
    xss_hal_dev_t       *cam_dev;
    xss_hal_cam_cfg_t    cam_cfg;
    xss_osal_task_t     *cam_task;
} us_info_t;


xss_result_t us_cam_init(us_info_t *us_info);
xss_result_t us_cam_exit(us_info_t *us_info);

#endif /* ES_ENTRY_C_ */
