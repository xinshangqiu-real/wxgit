/*
 * hal_core.c
 *
 *  Created on: Mar 1, 2020
 *      Author: xinshangqiu
 */

#include <xml_hal_cam.h>
#include "xml_hal.h"
#include "xml_log.h"


#define HAL_LOGF(...)        XSS_LOGF(XSS_MODULE_HAL, __VA_ARGS__)
#define HAL_LOGE(...)        XSS_LOGE(XSS_MODULE_HAL, __VA_ARGS__)
#define HAL_LOGW(...)        XSS_LOGW(XSS_MODULE_HAL, __VA_ARGS__)
#define HAL_LOGI(...)        XSS_LOGI(XSS_MODULE_HAL, __VA_ARGS__)
#define HAL_LOGD(...)        XSS_LOGD(XSS_MODULE_HAL, __VA_ARGS__)
#define HAL_LOGV(...)        XSS_LOGV(XSS_MODULE_HAL, __VA_ARGS__)


typedef struct xss_hal_obj {
    const char *name;
    xss_result_t (*attach)(void *dev);
    xss_result_t (*detach)(void *dev);
} xss_hal_obj_t;

extern xss_result_t hal_cam_attach(void *dev);
extern xss_result_t hal_cam_detach(void *dev);

static xss_hal_obj_t obj_tbl[] = {
    { "/dev/video",        hal_cam_attach,        hal_cam_detach        }
};

xss_result_t _hal_attach(const char *name, xss_hal_dev_t *dev)
{
    uint32_t i = 0;

    for (i = 0; i < (sizeof(obj_tbl)/sizeof(xss_hal_obj_t)); i++) {
        if (strncmp(name, obj_tbl[i].name, strlen(obj_tbl[i].name)) == 0) {
            dev->attach = obj_tbl[i].attach;
            dev->detach = obj_tbl[i].detach;
            return SUCCESS;
        }
    }

    return ERR_NORSRC;
}

xss_result_t xss_hal_open(char *name, xss_hal_dev_t **dev, void *cfg)
{
    xss_result_t ret = SUCCESS;
    xss_hal_drv_t *hal_drv = NULL;
    xss_hal_dev_t *hal_dev = NULL;
    void *mem_base = NULL;

    mem_base = calloc(sizeof(xss_hal_dev_t) + sizeof(xss_hal_drv_t), 1);
    if (mem_base == NULL) {
        HAL_LOGE("no mem\n");
        return ERR_NOMEM;
    }

    hal_dev = (xss_hal_dev_t *)mem_base;
    hal_dev->priv = mem_base + sizeof(xss_hal_dev_t);
    hal_dev->name = name;

    ret = _hal_attach(name, hal_dev);
    if (ret != SUCCESS) {
        HAL_LOGE("_hal_probe(%s) failed:%d\n", name, ret);
        goto mem_free;
    }

    if (hal_dev->attach) {
        ret = hal_dev->attach(hal_dev);
        if (ret != SUCCESS) {
            HAL_LOGE("_hal_probe(%s) failed:%d\n", name, ret);
            goto mem_free;
        }
    } else {
        ret = ERR_PERM;
        goto mem_free;
    }

    hal_drv = (xss_hal_drv_t *)hal_dev->priv;
    hal_drv->dev = hal_dev;

    if (hal_drv->open) {
        ret = hal_drv->open(hal_dev, cfg);
        if (ret != SUCCESS) {
            HAL_LOGE("hal_drv->probe(%s) failed:%d\n", name, ret);
            goto mem_free;
        }
    }

    *dev = hal_dev;
    return ret;

    mem_free:
    free(mem_base);
    return ret;
}

xss_result_t xss_hal_ioctl(xss_hal_dev_t *dev, void *ctl)
{
    xss_hal_drv_t *drv = NULL;

    if (dev == NULL || ctl == NULL) {
        return ERR_FAULT;
    }

    drv = (xss_hal_drv_t *)dev->priv;
    if (drv->ioctl) {
        return drv->ioctl(dev, ctl);
    }

    return SUCCESS;
}

xss_result_t xss_hal_write(xss_hal_dev_t *dev, void *buf, uint32_t nbytes, uint32_t *ret_len)
{
    xss_hal_drv_t *drv = NULL;

    if (dev == NULL || buf == NULL) {
        return ERR_FAULT;
    }

    drv = (xss_hal_drv_t *)dev->priv;
    if (drv->write) {
        return drv->write(dev, buf, nbytes, ret_len);
    }

    return ERR_PERM;
}

xss_result_t xss_hal_read(xss_hal_dev_t *dev, void *buf, uint32_t nbytes, uint32_t *ret_len)
{
    xss_hal_drv_t *drv = NULL;

    if (dev == NULL || buf == NULL) {
        return ERR_FAULT;
    }

    drv = (xss_hal_drv_t *)dev->priv;
    if (drv->read) {
        return drv->read(dev, buf, nbytes, ret_len);
    }

    return ERR_PERM;
}

xss_result_t xss_hal_close(xss_hal_dev_t *dev)
{
    xss_result_t ret = SUCCESS;
    xss_hal_drv_t *drv = NULL;

    drv = (xss_hal_drv_t *)dev->priv;
    if (drv->close) {
        ret = drv->close(dev);
        if (ret != SUCCESS) {
            HAL_LOGE("drv->close(%s) failed:%d\n", dev->name, ret);
            return ret;
        }
    }

    if (dev->detach) {
        ret = dev->detach(dev);
        if (ret != SUCCESS) {
            HAL_LOGE("dev->detach(%s) failed:%d\n", dev->name, ret);
            return ret;
        }
    }

    free(dev);
    return ret;
}

