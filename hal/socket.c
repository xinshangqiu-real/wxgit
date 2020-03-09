///*
// * socket.c
// *
// *  Created on: Mar 3, 2020
// *      Author: xinshangqiu
// */
//#include <sys/types.h>
//#include <sys/socket.h>
//#include <sys/un.h>
//
//
//#include "xml_define.h"
//#include "xml_log.h"
//
//#include "xml_osal.h"
//
//typedef struct xss_local_channel_cfg {
//    char *phy_port;
//} xss_local_channel_cfg_t;
//
//xss_result_t socket_init(void *arg)
//{
//    int32_t msg_fd;
//    xss_local_channel_cfg_t *cfg;
//    struct sockaddr_in client_addr;
//    struct sockaddr_un serv;
//
//    msg_fd = socket(AF_UNIX, SOCK_DGRAM);
//    if (msg_fd < 0) {
//        printf("socket failed:%s\n", strerror(errno));
//        return ERR_PERM;
//    }
//
//    unlink(cfg->phy_port);
//    bzero(&serv, sizeof(serv));
//    serv.sun_family = AF_LOCAL;
//    strcpy(serv.sun_path, cfg->phy_port);
//
//    bind(msg_fd, (struct sockaddr *)&serv, sizeof(serv));
//
//}
//xss_result_t socket_recv(void *);
//xss_result_t socket_send(void *);
//xss_result_t socket_exit(void *);
