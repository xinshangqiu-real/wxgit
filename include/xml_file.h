/*
 * xml_file.h
 *
 *  Created on: Mar 6, 2020
 *      Author: xinshangqiu
 */

#ifndef XML_FILE_H_
#define XML_FILE_H_

#include "xml_define.h"


xss_result_t xss_file_exist(const char *name);
xss_result_t xss_file_size(const char *name, uint32_t *nbytes);

#endif /* XML_FILE_H_ */
