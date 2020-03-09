/*
 * linux_cam.h
 *
 *  Created on: Mar 1, 2020
 *      Author: xinshangqiu
 */

#ifndef XML_HAL_CAM_H_
#define XML_HAL_CAM_H_

#include "xml_define.h"

typedef struct xss_hal_cam_cfg {
    uint32_t width;
    uint32_t height;
    uint32_t format;
    uint32_t frame_rate;
    uint32_t buf_num;
} xss_hal_cam_cfg_t;

#define XSS_CAM_FMT_MJPEG       0x01
#define XSS_CAM_FMT_YUYV        0x02

xss_result_t hal_cam_start(void *dev);
xss_result_t hal_cam_pop(void *dev, xss_stream_buf_t **buf);
xss_result_t hal_cam_push(void *dev, xss_stream_buf_t *buf);
xss_result_t hal_cam_stop(void *dev);


#endif /* XML_HAL_CAM_H_ */
