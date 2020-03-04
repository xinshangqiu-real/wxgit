/*
 * xml_hal.h
 *
 *  Created on: Mar 1, 2020
 *      Author: xinshangqiu
 */

#ifndef XML_HAL_H_
#define XML_HAL_H_

#include <fcntl.h>

#include "xml_define.h"
#include "xml_log.h"


typedef struct xss_hal_dev {
    char *name;
    xss_result_t (*attach)(void *dev);
    xss_result_t (*detach)(void *dev);
    void *priv;                        /* Point to xss_hal_drv_t */
} xss_hal_dev_t;

typedef struct xss_hal_drv {
    xss_result_t (*open)(void *dev, void *cfg);
    xss_result_t (*ioctl)(void *dev, void *ctl);
    xss_result_t (*write)(void *dev, void *buf, uint32_t nbytes, uint32_t *ret_len);
    xss_result_t (*read)(void *dev, void *buf, uint32_t nbytes, uint32_t *ret_len);
    xss_result_t (*close)(void *dev);
    void *dev;
    void *priv;
} xss_hal_drv_t;

xss_result_t xss_hal_open(char *name, xss_hal_dev_t **dev, void *cfg);
xss_result_t xss_hal_write(xss_hal_dev_t *dev, void *buf, uint32_t nbytes, uint32_t *ret_len);
xss_result_t xss_hal_read(xss_hal_dev_t *dev, void *buf, uint32_t nbytes, uint32_t *ret_len);
xss_result_t xss_hal_ioctl(xss_hal_dev_t *dev, void *ctl);
xss_result_t xss_hal_close(xss_hal_dev_t *dev);

#endif /* XML_HAL_H_ */
