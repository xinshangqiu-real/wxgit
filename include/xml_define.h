#ifndef __XML_DEFINE_H_
#define __XML_DEFINE_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>


/*
 * return value
 * */

typedef int32_t xss_result_t;

#define SUCCESS                         0
#define ERR_FAILURE                     -1001    /* Common failure*/
#define ERR_PERM                        -1002    /* Operation not permitted */
#define ERR_AGAIN                       -1003    /* Try again */
#define ERR_BUSY                        -1004    /* Device or resource is busy */
#define ERR_ACCESS                      -1005    /* Permission denied  */
#define ERR_FAULT                       -1006    /* Bad adress */
#define ERR_TIMEOUT                     -1007    /* Timeout */
#define ERR_NORSRC                      -1008    /* No resource */
#define ERR_NOMEM                       -1009    /* No memory */
#define ERR_INVAL                       -1010    /* Invalid argument */

#define LEN_1024                        1024
#define LEN_512                         512
#define LEN_256                         256
#define LEN_128                         128
#define LEN_64                          64
#define LEN_32                          32
#define LEN_16                          16

/*
 * xml module list
 * */
#define XSS_MODULE_HAL                  0x11
#define XSS_MODULE_CAM                  0x12        /* camera hal module */
#define XSS_MODULE_US                   0x13        /* up stream module*/
#define XSS_MODULE_DS                   0x14        /* down stream module */
#define XSS_MODULE_MB                   0x15        /* message bus */
#define XSS_MODULE_UNKNOW               0xFF        /* unknow module */



/*
 * misc
 * */
typedef struct xss_stream_buf {
    uint32_t totl_len;
    uint32_t data_len;
    uint8_t *data;
} xss_stream_buf_t;

#endif
