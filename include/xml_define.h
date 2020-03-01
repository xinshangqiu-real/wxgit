#ifndef __XML_DEFINE_H_
#define __XML_DEFINE_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>


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


#endif
