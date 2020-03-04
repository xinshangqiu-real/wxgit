/*
 * hal_cam.c
 *
 *  Created on: Mar 1, 2020
 *      Author: xinshangqiu
 */
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include "xml_define.h"
#include "xml_log.h"
#include "xml_hal.h"

#include "xml_hal_cam.h"

#define CAM_LOGF(...)        XSS_LOGF(XSS_MODULE_CAM, __VA_ARGS__)
#define CAM_LOGE(...)        XSS_LOGE(XSS_MODULE_CAM, __VA_ARGS__)
#define CAM_LOGW(...)        XSS_LOGW(XSS_MODULE_CAM, __VA_ARGS__)
#define CAM_LOGI(...)        XSS_LOGI(XSS_MODULE_CAM, __VA_ARGS__)
#define CAM_LOGD(...)        XSS_LOGD(XSS_MODULE_CAM, __VA_ARGS__)
#define CAM_LOGV(...)        XSS_LOGV(XSS_MODULE_CAM, __VA_ARGS__)

typedef struct xss_hal_cam {
    char *name;
    uint32_t fd;
    xss_stream_buf_t *buf;
    xss_hal_cam_cfg_t cfg;
} xss_hal_cam_priv_t;



static uint32_t _v4l2_fmt(uint32_t format)
{
    switch (format) {
        case XSS_CAM_FMT_MJPEG:
            return V4L2_PIX_FMT_MJPEG;
        case XSS_CAM_FMT_YUYV:
            return V4L2_PIX_FMT_YUYV;
        default:
            break;
    }

    return 0;
}


static xss_result_t _query_ctrl(xss_hal_cam_priv_t *cam_priv)
{
    int32_t i = 0, j = 0;
    struct v4l2_queryctrl qctrl;
    struct v4l2_control ctrl;
    struct v4l2_querymenu menu;
    struct v4l2_fmtdesc fmtd;

    fmtd.index = 0;
    fmtd.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    while (ioctl(cam_priv->fd, VIDIOC_ENUM_FMT, &fmtd) == 0) {
        CAM_LOGI("video format:%s\n", fmtd.description);
        fmtd.index++;
    }

    for (i = V4L2_CID_BASE; i < V4L2_CID_LASTP1; i++) {
        qctrl.id = i;
        if (ioctl(cam_priv->fd, VIDIOC_QUERYCTRL, &qctrl) != 0) {
            continue;
        }

        ctrl.id = qctrl.id;
        if (ioctl(cam_priv->fd, VIDIOC_G_CTRL, &ctrl) != 0) {
            CAM_LOGE("get v4l2_cid:0x%x value failed\n", ctrl.id);
        }

        CAM_LOGI("v4l2_cid:0x%x name:%-32s min:%5d max:%5d value:%5d step:%d default_value:%4d\n",
                  qctrl.id, qctrl.name, qctrl.minimum, qctrl.maximum, ctrl.value, qctrl.step, qctrl.default_value);
        if (qctrl.type == V4L2_CTRL_TYPE_MENU) {
            for (j = qctrl.minimum; j <= qctrl.maximum; j++) {
                menu.id = qctrl.id;
                menu.index = j;
                if (ioctl(cam_priv->fd, VIDIOC_QUERYMENU, &menu) == 0) {
                    CAM_LOGI("\t\t\t%d:%s\n", menu.index, menu.name);
                }
            }
        }
    }

    return SUCCESS;
}

static xss_result_t _query_cap(xss_hal_cam_priv_t *cam_priv)
{
    struct v4l2_capability cap;

    memset(&cap, 0, sizeof(struct v4l2_capability));
    if (ioctl(cam_priv->fd, VIDIOC_QUERYCAP, &cap) != 0) {
        CAM_LOGE("device %s: querycap failed:%s\n", cam_priv->name, strerror(errno));
        return ERR_FAILURE;
    }

    CAM_LOGI("driver:%s\n", cap.driver);
    CAM_LOGI("card:%s\n", cap.card);
    CAM_LOGI("bus info:%s\n", cap.bus_info);
    CAM_LOGI("version:%x\n", cap.version);
    CAM_LOGI("capabilities:0x%x\n", cap.capabilities);
    CAM_LOGI("device_caps:0x%x\n", cap.device_caps);
    return SUCCESS;
}

static xss_result_t _set_format(xss_hal_cam_priv_t *cam_priv, uint32_t width, uint32_t height, uint32_t format)
{
    struct v4l2_format fmtd;

    fmtd.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmtd.fmt.pix.pixelformat = _v4l2_fmt(format);
    if (ioctl(cam_priv->fd, VIDIOC_TRY_FMT, &fmtd) != 0) {
        CAM_LOGE("device %s: not supports format:0x%x\n", cam_priv->name, format);
        return ERR_INVAL;
    }

    fmtd.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmtd.fmt.pix.pixelformat = _v4l2_fmt(format);
    fmtd.fmt.pix.width = width;
    fmtd.fmt.pix.height = height;
    fmtd.fmt.pix.field = V4L2_FIELD_INTERLACED;
    if (ioctl(cam_priv->fd, VIDIOC_S_FMT, &fmtd) != 0) {
        CAM_LOGE("device %s: set format failed:%s", cam_priv->name, strerror(errno));
        return ERR_INVAL;
    }

    if (ioctl(cam_priv->fd, VIDIOC_G_FMT, &fmtd) == 0) {
        CAM_LOGI("device :%s width:%d height:%d format:%c%c%c%c\n", cam_priv->name,
                fmtd.fmt.pix.width, fmtd.fmt.pix.height,
                (fmtd.fmt.pix.pixelformat) & 0xFF,
                (fmtd.fmt.pix.pixelformat>>8) & 0xFF,
                (fmtd.fmt.pix.pixelformat>>16) & 0xFF,
                (fmtd.fmt.pix.pixelformat>>24) & 0xFF);
    }

    return SUCCESS;
}

static xss_result_t _request_buf(xss_hal_cam_priv_t *cam_priv, uint32_t buf_num)
{
    uint32_t buf_idx, i;
    struct v4l2_buffer v4l2_buf;
    struct v4l2_requestbuffers req;

    req.count = buf_num;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    if (ioctl(cam_priv->fd, VIDIOC_REQBUFS, &req) != 0) {
        CAM_LOGE("device :%s request buffer failed:%s\n", cam_priv->name, strerror(errno));
        return ERR_PERM;
    }

    cam_priv->buf = calloc(buf_num, sizeof(xss_stream_buf_t));
    if (cam_priv->buf == NULL) {
        CAM_LOGE("no mem\n");
        return ERR_NOMEM;
    }

    for (buf_idx = 0; buf_idx < buf_num; buf_idx++) {
        v4l2_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        v4l2_buf.memory = V4L2_MEMORY_MMAP;
        v4l2_buf.index = buf_idx;
        if (ioctl(cam_priv->fd, VIDIOC_QUERYBUF, &v4l2_buf) != 0) {
            CAM_LOGE("device :%s querybuf failed:%s\n", cam_priv->name, strerror(errno));
            goto qbuf_failed;
        }

        cam_priv->buf[buf_idx].totl_len = v4l2_buf.length;
        cam_priv->buf[buf_idx].data = mmap(NULL, v4l2_buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, cam_priv->fd, v4l2_buf.m.offset);
        if (cam_priv->buf[buf_idx].data == MAP_FAILED) {
            cam_priv->buf[buf_idx].data = NULL;
            CAM_LOGE("device :%s mmap failed:%s\n", cam_priv->name, strerror(errno));
            goto mmap_failed;
        }

        if (ioctl(cam_priv->fd, VIDIOC_QBUF, &v4l2_buf) != 0) {
            CAM_LOGE("device :%s qbuf failed:%s\n", cam_priv->name, strerror(errno));
            goto qbuf_failed;
        }
    }
    return SUCCESS;

    qbuf_failed:
    for (i=0;i<buf_idx;i++) {
        v4l2_buf.index = i;
        ioctl(cam_priv->fd, VIDIOC_DQBUF, &v4l2_buf);
    }

    mmap_failed:
    for (i=0;i<buf_idx;i++) {
        if (cam_priv->buf[buf_idx].data != NULL) {
            munmap(cam_priv->buf[buf_idx].data, cam_priv->buf[buf_idx].totl_len);
        }
    }

    return ERR_FAILURE;
}

static xss_result_t _set_frame_rate(xss_hal_cam_priv_t *cam_priv, uint32_t fps)
{
    struct v4l2_streamparm stream_parm;

    memset(&stream_parm, 0, sizeof(struct v4l2_streamparm));
    stream_parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    stream_parm.parm.capture.timeperframe.denominator = fps;
    stream_parm.parm.capture.timeperframe.numerator = 1;
    if (ioctl(cam_priv->fd, VIDIOC_S_PARM, &stream_parm) != 0) {
        CAM_LOGE("device :%s set frame rate failed:%s\n", cam_priv->name, strerror(errno));
        return ERR_INVAL;
    }

    return SUCCESS;
}

static xss_result_t hal_cam_open(void *dev, void *cfg)
{
    xss_result_t ret = SUCCESS;
    xss_hal_dev_t *hal_dev = NULL;
    xss_hal_drv_t *hal_drv = NULL;
    xss_hal_cam_priv_t *cam_priv = NULL;
    xss_hal_cam_cfg_t *cam_cfg = NULL;

    if ((!dev) || (!cfg)) {
        CAM_LOGE("dev or cfg is null\n");
        return ERR_FAULT;
    }

    hal_dev = (xss_hal_dev_t *)dev;
    cam_cfg = (xss_hal_cam_cfg_t *)cfg;
    if (!hal_dev->priv) {
        CAM_LOGE("hal_dev->priv is null\n");
        return ERR_FAULT;
    }

    hal_drv = (xss_hal_drv_t *)hal_dev->priv;
    if (!hal_drv->priv) {
        CAM_LOGE("hal_drv->priv is null\n");
        return ERR_FAULT;
    }

    cam_priv = (xss_hal_cam_priv_t *)hal_drv->priv;
    cam_priv->fd = open(cam_priv->name, O_RDWR);
    if (cam_priv->fd < 0) {
        CAM_LOGE("open(%s) failed:%s\n", cam_priv->name, strerror(errno));
        return ERR_NORSRC;
    }

    ret = _query_ctrl(cam_priv);
    if (ret != SUCCESS) {
        CAM_LOGE("device %s: set_fmt failed:%d\n", cam_priv->name, ret);
    }

    ret = _query_cap(cam_priv);
    if (ret != SUCCESS) {
        CAM_LOGE("device %s: set_fmt failed:%d\n", cam_priv->name, ret);
    }

    ret = _set_format(cam_priv, cam_cfg->width, cam_cfg->height, cam_cfg->format);
    if (ret != SUCCESS) {
        CAM_LOGE("device %s: set_fmt failed:%d\n", cam_priv->name, ret);
        goto open_failed;
    }

    ret = _set_frame_rate(cam_priv, cam_cfg->frame_rate);
    if (ret != SUCCESS) {
        CAM_LOGE("device %s: set_fmt failed:%d\n", cam_priv->name, ret);
        goto open_failed;
    }

    ret = _request_buf(cam_priv, cam_cfg->buf_num);
    if (ret != SUCCESS) {
        CAM_LOGE("device %s: req_buf failed:%d\n", cam_priv->name, ret);
        goto open_failed;
    }

    memcpy(&cam_priv->cfg, cam_cfg, sizeof(xss_hal_cam_cfg_t));
    return SUCCESS;

    open_failed:
    if (cam_priv->fd) {
        close(cam_priv->fd);
    }
    return ret;
}

static xss_result_t hal_cam_ioctl(void *dev, void *ctl)
{
    return SUCCESS;
}

static xss_result_t hal_cam_close(void *dev)
{
    xss_hal_dev_t *hal_dev = (xss_hal_dev_t *)dev;
    xss_hal_drv_t *hal_drv = hal_dev->priv;
    xss_hal_cam_priv_t *cam_priv = hal_drv->priv;
    uint32_t i;

    for (i = 0; i < cam_priv->cfg.buf_num; i++) {
        if (cam_priv->buf[i].data != NULL) {
            munmap(cam_priv->buf[i].data, cam_priv->buf->totl_len);
        }
    }

    if (cam_priv->buf) {
        free(cam_priv->buf);
    }
    close(cam_priv->fd);

    return SUCCESS;
}

xss_result_t hal_cam_start(void *dev)
{
    xss_hal_dev_t *hal_dev = NULL;
    xss_hal_drv_t *hal_drv = NULL;
    xss_hal_cam_priv_t *cam_priv = NULL;
    enum v4l2_buf_type type;

    if (!dev) {
        CAM_LOGE("dev or cfg is null\n");
        return ERR_FAULT;
    }

    hal_dev = (xss_hal_dev_t *)dev;
    if (!hal_dev->priv) {
        CAM_LOGE("hal_dev->priv is null\n");
        return ERR_FAULT;
    }

    hal_drv = (xss_hal_drv_t *)hal_dev->priv;
    if (!hal_drv->priv) {
        CAM_LOGE("hal_drv->priv is null\n");
        return ERR_FAULT;
    }

    cam_priv = (xss_hal_cam_priv_t *)hal_drv->priv;
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(cam_priv->fd, VIDIOC_STREAMON, &type) != 0) {
        CAM_LOGE("device :%s streamon failed:%s\n", cam_priv->name, strerror(errno));
        return ERR_PERM;
    }

    return SUCCESS;
}

xss_result_t hal_cam_stop(void *dev)
{
    xss_hal_dev_t *hal_dev = NULL;
    xss_hal_drv_t *hal_drv = NULL;
    xss_hal_cam_priv_t *cam_priv = NULL;
    enum v4l2_buf_type type;

    if (!dev) {
        CAM_LOGE("dev or cfg is null\n");
        return ERR_FAULT;
    }

    hal_dev = (xss_hal_dev_t *)dev;
    if (!hal_dev->priv) {
        CAM_LOGE("hal_dev->priv is null\n");
        return ERR_FAULT;
    }

    hal_drv = (xss_hal_drv_t *)hal_dev->priv;
    if (!hal_drv->priv) {
        CAM_LOGE("hal_drv->priv is null\n");
        return ERR_FAULT;
    }

    cam_priv = (xss_hal_cam_priv_t *)hal_drv->priv;
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (ioctl(cam_priv->fd, VIDIOC_STREAMOFF, &type) != 0) {
        CAM_LOGE("device :%s streamoff failed:%s\n", cam_priv->name, strerror(errno));
        return ERR_PERM;
    }

    return SUCCESS;
}

xss_result_t hal_cam_pop(void *dev, xss_stream_buf_t **buf)
{
    xss_hal_dev_t *hal_dev = NULL;
    xss_hal_drv_t *hal_drv = NULL;
    xss_hal_cam_priv_t *cam_priv = NULL;
    struct v4l2_buffer v4l2_buf;

    if (!dev) {
        CAM_LOGE("dev or cfg is null\n");
        return ERR_FAULT;
    }

    hal_dev = (xss_hal_dev_t *)dev;
    if (!hal_dev->priv) {
        CAM_LOGE("hal_dev->priv is null\n");
        return ERR_FAULT;
    }

    hal_drv = (xss_hal_drv_t *)hal_dev->priv;
    if (!hal_drv->priv) {
        CAM_LOGE("hal_drv->priv is null\n");
        return ERR_FAULT;
    }

    cam_priv = (xss_hal_cam_priv_t *)hal_drv->priv;
    memset(&v4l2_buf, 0, sizeof(struct v4l2_buffer));
    v4l2_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    v4l2_buf.memory = V4L2_MEMORY_MMAP;
    if (ioctl (cam_priv->fd, VIDIOC_DQBUF, &v4l2_buf) != 0) {
        CAM_LOGE("device :%s dqbuf failed:%s\n", cam_priv->name, strerror(errno));
        return ERR_FAILURE;
    }
    cam_priv->buf[v4l2_buf.index].data_len = v4l2_buf.bytesused;
    *buf = &cam_priv->buf[v4l2_buf.index];

    return SUCCESS;
}

xss_result_t hal_cam_push(void *dev, xss_stream_buf_t *buf)
{
    xss_hal_dev_t *hal_dev = NULL;
    xss_hal_drv_t *hal_drv = NULL;
    xss_hal_cam_priv_t *cam_priv = NULL;
    struct v4l2_buffer v4l2_buf;
    uint32_t i;

    if (!dev) {
        CAM_LOGE("dev or cfg is null\n");
        return ERR_FAULT;
    }

    hal_dev = (xss_hal_dev_t *)dev;
    if (!hal_dev->priv) {
        CAM_LOGE("hal_dev->priv is null\n");
        return ERR_FAULT;
    }

    hal_drv = (xss_hal_drv_t *)hal_dev->priv;
    if (!hal_drv->priv) {
        CAM_LOGE("hal_drv->priv is null\n");
        return ERR_FAULT;
    }

    cam_priv = (xss_hal_cam_priv_t *)hal_drv->priv;
    for (i = 0; i < cam_priv->cfg.buf_num; i++) {
        if (buf == &cam_priv->buf[i]) {
            v4l2_buf.index = i;
            v4l2_buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            v4l2_buf.memory = V4L2_MEMORY_MMAP;
            if (ioctl (cam_priv->fd, VIDIOC_QBUF, &v4l2_buf) != 0) {
                CAM_LOGE("device :%s dqbuf failed:%s\n", cam_priv->name, strerror(errno));
                return ERR_FAILURE;
            }

            return SUCCESS;
        }
    }

    return ERR_FAILURE;
}

xss_result_t hal_cam_attach(void *dev)
{
    xss_hal_dev_t *hal_dev = (xss_hal_dev_t *)dev;
    xss_hal_drv_t *hal_drv = (xss_hal_drv_t *)hal_dev->priv;
    xss_hal_cam_priv_t *cam_priv = NULL;

    cam_priv = calloc(sizeof(xss_hal_cam_priv_t), 1);
    if (cam_priv == NULL) {
        CAM_LOGE("no mem\n");
        return ERR_NOMEM;
    }
    cam_priv->name = hal_dev->name;

    hal_drv->priv = cam_priv;
    hal_drv->open = hal_cam_open;
    hal_drv->write = NULL;
    hal_drv->read = NULL;
    hal_drv->ioctl = hal_cam_ioctl;
    hal_drv->close = hal_cam_close;

    return SUCCESS;
}

xss_result_t hal_cam_detach(void *dev)
{
    xss_hal_dev_t *hal_dev = (xss_hal_dev_t *)dev;
    xss_hal_drv_t *hal_drv = (xss_hal_drv_t *)hal_dev->priv;

    if (hal_drv->priv) {
        free(hal_drv->priv);
    }

    hal_drv->open = NULL;
    hal_drv->write = NULL;
    hal_drv->read = NULL;
    hal_drv->ioctl = NULL;
    hal_drv->close = NULL;

    return SUCCESS;
}
