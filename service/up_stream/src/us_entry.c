/*
 * e_entry.c
 *
 *  Created on: Mar 1, 2020
 *      Author: xinshangqiu
 */

#include <us_common.h>
#include <us_state.h>
#include "xml_define.h"
#include "xml_log.h"



int main(int argc, char **argv)
{
    xss_result_t ret = SUCCESS;
    us_info_t *us_info = NULL;
    xss_hal_cam_cfg_t cam_cfg = {
        .width = 1280,
        .height = 720,
        .format = XSS_CAM_FMT_YUYV,
        .frame_rate = 30,
        .buf_num = 4
    };

    us_info = calloc(1, sizeof(us_info_t));
    if (us_info == NULL) {
        US_LOGF("no mem\n");
        return ERR_NOMEM;
    }

    us_info->cam_cfg.width = cam_cfg.width;
    us_info->cam_cfg.height = cam_cfg.height;
    us_info->cam_cfg.format = cam_cfg.format;
    us_info->cam_cfg.frame_rate = cam_cfg.frame_rate;
    us_info->cam_cfg.buf_num = cam_cfg.buf_num;

    ret = us_cam_init(us_info);
    if (ret != SUCCESS) {
        US_LOGE("us_cam_init failed:%d\n", ret);
        goto exit;
    }

    while (1) {
        US_LOGI("up stream service heartbeat\n");
        xss_osal_sleep(2000000);
    }

    exit:
    ret = us_cam_exit(us_info);
    if (ret != SUCCESS) {
        US_LOGE("us_cam_exit failed:%d\n", ret);
    }

}
