/*
 * xml_file.c
 *
 *  Created on: Mar 6, 2020
 *      Author: xinshangqiu
 */
#include <sys/stat.h>

#include "xml_file.h"


xss_result_t xss_file_exist(const char *name)
{
    return (name == NULL) ? ERR_FAULT : ((access(name , F_OK) == 0) ? SUCCESS : ERR_NORSRC);
}

xss_result_t xss_file_size(const char *name, uint32_t *nbytes)
{
    struct stat file_stat;

    if (stat(name, &file_stat) != 0) {
        return ERR_PERM;
    }

    *nbytes = file_stat.st_size;

    return SUCCESS;
}

