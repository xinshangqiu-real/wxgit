///*
// * xml_msg.c
// *
// *  Created on: Mar 3, 2020
// *      Author: xinshangqiu
// */
//
//#include "xml_define.h"
//#include "xml_log.h"
//#include "xml_osal.h"
//
//#include "xml_msg.h"
//
//#define MB_LOGF(...)        XSS_LOGF(XSS_MODULE_MB, __VA_ARGS__)
//#define MB_LOGE(...)        XSS_LOGE(XSS_MODULE_MB, __VA_ARGS__)
//#define MB_LOGW(...)        XSS_LOGW(XSS_MODULE_MB, __VA_ARGS__)
//#define MB_LOGI(...)        XSS_LOGI(XSS_MODULE_MB, __VA_ARGS__)
//#define MB_LOGD(...)        XSS_LOGD(XSS_MODULE_MB, __VA_ARGS__)
//#define MB_LOGV(...)        XSS_LOGV(XSS_MODULE_MB, __VA_ARGS__)
//
//
//typedef struct xss_msg_client_priv {
//    xss_osal_task_t     *hub_task;
//    xss_osal_msgq_t     *hub_msgq;
//} xss_msg_client_priv_t;
//
//
//static xss_result_t msg_handle(xss_msg_client_t *client, xss_msg_t *msg)
//{
//    return SUCCESS;
//}
//
//static void msg_hub_task(void *arg)
//{
//    xss_msg_client_t *client;
//    xss_msg_client_priv_t *priv;
//    xss_result_t ret = SUCCESS;
//    uint32_t recv_len;
//    xss_msg_t msg;
//    uint8_t buffer[XSS_MSG_PAYLOAD_MAXLEN];
//
//    client = (xss_msg_client_t *)arg;
//    priv = (xss_msg_client_priv_t *)client->priv;
//    msg.data = buffer;
//
//    while (1) {
//        recv_len = sizeof(xss_msg_t);
//        ret = xss_osal_msgq_recv(priv->hub_msgq, &msg, &recv_len, XSS_OSAL_WAIT_FOREVER);
//        if (ret != SUCCESS) {
//            MB_LOGE("xss_osal_msgq_recv failed:%d\n");
//            continue;
//        }
//
//        XSS_ASSERT(recv_len != sizeof(xss_msg_t),
//                "xss_osal_msgq_recv need %d but return %d\n",
//                recv_len, sizeof(xss_msg_t));
//
//        if (msg.len) {
//            recv_len = msg.len;
//            ret = xss_osal_msgq_recv(priv->hub_msgq, msg.data, &recv_len, XSS_OSAL_WAIT_FOREVER);
//            XSS_ASSERT(ret != SUCCESS, "xss_osal_msgq_recv failed:%d\n", ret);
//            XSS_ASSERT(recv_len != msg.len, "xss_osal_msgq_recv need %d but return %d\n", recv_len, msg.len);
//        }
//
//        ret = msg_handle(client, msg);
//        if (ret != SUCCESS) {
//            MB_LOGE("msg_handle failed:%d\n");
//        }
//    }
//}
//
//xss_result_t xss_msg_init(xss_msg_client_t **client, xss_msg_client_attr_t *attr)
//{
//    xss_result_t ret = SUCCESS;
//    xss_osal_task_attr_t task_attr;
//    xss_osal_msgq_attr_t msgq_attr;
//    xss_msg_client_t *mclient = NULL;
//    xss_msg_client_priv_t *priv = NULL;
//    void *mem_base = NULL;
//
//    mem_base = calloc(1, sizeof(xss_msg_client_t) + sizeof(xss_msg_client_priv_t));
//    if (mem_base == NULL) {
//        MB_LOGE("no mem\n");
//        return ERR_NOMEM;
//    }
//
//    mclient = (xss_msg_client_t *)mem_base;
//    mclient->name = attr->name;
//    memcpy(&mclient->attr, attr, sizeof(xss_msg_client_attr_t));
//    mclient->priv = (xss_msg_client_priv_t *)(mem_base + sizeof(xss_msg_client_priv_t));
//    priv = mclient->priv;
//
//    /*
//     * message bus port configure
//     * */
//    ret = msg_bus_port_cfg(priv, attr->port_cfg_tbl);
//    if (ret != SUCCESS) {
//        US_LOGE("xss_osal_msgq_create(%s) failed:%d\n", msgq_attr.name, ret);
//        goto init_failed;
//    }
//
//    msgq_attr.name = "message bus msgq";
//    msgq_attr.size = 0x100000;                  /* 1MB bytes */
//    ret = xss_osal_msgq_create(&priv->hub_msgq, &msgq_attr);
//    if (ret != SUCCESS) {
//        US_LOGE("xss_osal_msgq_create(%s) failed:%d\n", msgq_attr.name, ret);
//        goto init_failed;
//    }
//
//    task_attr.name = "message bus task";
//    task_attr.param = (void *)client;
//    task_attr.entry = msg_hub_task;
//    task_attr.detached = false;
//    task_attr.priority = XSS_OSAL_PRIO_HIGH;
//    task_attr.stacksize = XSS_OSAL_STACKSIZE_DEFAULT;
//    ret = xss_osal_task_create(&priv->hub_task, &task_attr);
//    if (ret != SUCCESS) {
//        US_LOGE("xss_osal_task_create(%s) failed:%d\n", task_attr.name, ret);
//        goto init_failed;
//    }
//
//    *client = mclient;
//    return SUCCESS;
//
//    init_failed:
//    if (priv->hub_msgq) {
//        ret = xss_osal_msgq_destroy(priv->hub_msgq);
//        if (ret != SUCCESS) {
//            US_LOGE("xss_osal_msgq_destroy() failed:%d\n", ret);
//        }
//    }
//    free(mem_base);
//    return ret;
//}
