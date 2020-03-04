/*
 * es_common.h
 *
 *  Created on: Mar 2, 2020
 *      Author: xinshangqiu
 */

#ifndef US_COMMON_H_
#define US_COMMON_H_

#include "xml_define.h"
#include "xml_log.h"


#define US_LOGF(...)        XSS_LOGF(XSS_MODULE_US, __VA_ARGS__)
#define US_LOGE(...)        XSS_LOGE(XSS_MODULE_US, __VA_ARGS__)
#define US_LOGW(...)        XSS_LOGW(XSS_MODULE_US, __VA_ARGS__)
#define US_LOGI(...)        XSS_LOGI(XSS_MODULE_US, __VA_ARGS__)
#define US_LOGD(...)        XSS_LOGD(XSS_MODULE_US, __VA_ARGS__)
#define US_LOGV(...)        XSS_LOGV(XSS_MODULE_US, __VA_ARGS__)


#endif /* US_COMMON_H_ */
