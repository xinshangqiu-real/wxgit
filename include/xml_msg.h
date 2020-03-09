///*
// * xml_msg.h
// *
// *  Created on: Mar 3, 2020
// *      Author: xinshangqiu
// */
//
//#ifndef XML_MSG_H_
//#define XML_MSG_H_
//
//#define XSS_MSG_SOF         0x55
//#define XSS_MSG_PAYLOAD_MAXLEN      1024
//
//typedef struct xss_msg {
//    uint8_t src;
//    uint8_t dst;
//    uint8_t len;
//    uint8_t crc;
//    uint8_t *data;
//} xss_msg_t;
//
////typedef struct xss_msg {
////    uint8_t sof;
////    uint8_t len;
////    uint8_t seq;
////    uint8_t src;
////    uint8_t dst;
////    uint8_t msg;
////    uint8_t crc_h;
////    uint8_t crc_l;
////    uint8_t data[0];
////} xss_msg_t;
//
//typedef
//typedef struct xss_msg_payload {
//
//} xss_msg_payload_t;
//
//typedef struct xss_msg_port_cfg_tbl {
//
//} xss_msg_port_cfg_tbl_t;
//
//typedef struct xss_msg_callback_tbl {
//
//} xss_msg_callback_tbl_t;
//
//typedef struct xss_msg_client_attr {
//    char *name;
//    xss_msg_port_cfg_tbl_t  *port_cfg_tbl;
//    xss_msg_callback_tbl_t  *callback_tbl;
//} xss_msg_client_attr_t;
//
//typedef struct xss_msg_client {
//    char *name;
//    xss_msg_client_attr_t attr;
//    void *priv;
//} xss_msg_client_t;
//
//
//xss_result_t xss_msg_init(xss_msg_client_t **client, xss_msg_client_attr_t *attr);
//xss_result_t xss_msg_send(xss_msg_client_t *client, xss_msg_t *msg);
//xss_result_t xss_msg_recv(xss_msg_client_t *client, xss_msg_t *msg);
//xss_result_t xss_msg_exit(xss_msg_client_t *client);
//
//
//#endif /* XML_MSG_H_ */
