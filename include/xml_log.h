#ifndef __XML_LOG_H_
#define __XML_LOG_H_

#include "xml_define.h"

xss_result_t xss_log(const char *func, int32_t func_line, int32_t module, const char *fmt, ...);

#define XSS_LOGF(module, fmt, ...)   xss_log(__FUNCTION__, __LINE__, module, "F: "fmt, ##__VA_ARGS__)
#define XSS_LOGE(module, fmt, ...)   xss_log(__FUNCTION__, __LINE__, module, "E: "fmt, ##__VA_ARGS__)
#define XSS_LOGW(module, fmt, ...)   xss_log(__FUNCTION__, __LINE__, module, "W: "fmt, ##__VA_ARGS__)
#define XSS_LOGI(module, fmt, ...)   xss_log(__FUNCTION__, __LINE__, module, "I: "fmt, ##__VA_ARGS__)
#define XSS_LOGD(module, fmt, ...)   xss_log(__FUNCTION__, __LINE__, module, "D: "fmt, ##__VA_ARGS__)
#define XSS_LOGV(module, fmt, ...)   xss_log(__FUNCTION__, __LINE__, module, "V: "fmt, ##__VA_ARGS__)


#endif
