#include <stdarg.h>
#include <sys/time.h>
#include <time.h>

#include "xml_define.h"
#include "xml_file.h"
#include "xml_list.h"
#include "xml_osal.h"

#define XSS_LOG_WAIT_TIMEOUT_US         500000

typedef struct xss_log_client_attr {
    char    *file_dire;
    char    *file_mask;
    uint32_t file_size;
    uint32_t file_nums;
    uint32_t page_size;
    uint32_t page_nums;
} xss_log_client_attr_t;

typedef struct xss_log_client {
    xss_log_client_attr_t attr;

    xss_stream_buf_t log_buf;

    xss_osal_mutex_t *lock;
    xss_osal_sema_t *sync_sema;
    xss_osal_task_t *sync_task;
    xss_osal_msgq_t *free_list;
    xss_osal_msgq_t *dirty_list;

    void *page_membase;

    FILE *file_fd;
    uint32_t file_size;

} xss_log_client_t;



static xss_result_t file_write(xss_log_client_t *client, void *ptr, uint32_t len)
{
    xss_result_t ret= SUCCESS;
    xss_log_client_attr_t *attr;
    uint32_t i;
    char old_file[LEN_256];
    char new_file[LEN_256];

    if (client == NULL) {
        return ERR_NORSRC;
    }

    attr = &client->attr;
    if (client->file_fd == NULL) {
        snprintf(new_file, sizeof(new_file) - 1, attr->file_mask, 0);
        client->file_fd = fopen(new_file, "a+");
    }

    if (client->file_fd != NULL) {
        ret = fwrite(ptr, 1, len, client->file_fd);
        if (ret < 0) {
            return ERR_FAILURE;
        }

        client->file_size += ret;
        if (client->file_size > attr->file_size) {
            client->file_size = 0;
            fclose(client->file_fd);
            client->file_fd = NULL;

            for (i = attr->file_nums ; i> 0; i--) {
                snprintf(old_file, sizeof(old_file) - 1, attr->file_mask, i-1);
                if (xss_file_exist(old_file) == SUCCESS) {
                    snprintf(new_file, sizeof(new_file) - 1, attr->file_mask, i);
                    rename(old_file, new_file);
                }
            }
            snprintf(new_file, sizeof(new_file) - 1, attr->file_mask, 0);
            client->file_fd = fopen(new_file, "a+");
        }
    }

    return ret;
}

static void log_sync_task(void *arg)
{
    xss_result_t ret = SUCCESS;
    xss_log_client_t *client = NULL;
    xss_stream_buf_t log_buf;
    uint32_t recv_len;

    client = (xss_log_client_t *)arg;
    recv_len = sizeof(xss_stream_buf_t);

    while (1) {
        ret = xss_osal_msgq_recv(client->dirty_list, &log_buf, &recv_len,XSS_OSAL_WAIT_FOREVER);
        if (ret != SUCCESS) {
            continue;
        }

        ret = file_write(client, log_buf.data, log_buf.data_len);
        if (ret != SUCCESS) {
            printf("file_write failed:%d\n", ret);
        }

        log_buf.data_len = 0;
        ret = xss_osal_msgq_send(client->dirty_list, &log_buf, recv_len, XSS_OSAL_WAIT_FOREVER);
        if (ret != SUCCESS) {
            continue;
        }
    }
}

static xss_log_client_attr_t xlog_attr = {
        .file_dire = "/tmp/xsq_us",
        .file_mask = "/tmp/xsq_us/run%02d.log",
        .file_size = 1000000,
        .file_nums = 5,
        .page_size = 4096,
        .page_nums = 64
};

xss_log_client_t *xlog_client = NULL;

xss_result_t xss_log_init(void )
{
    uint32_t i;
    xss_result_t ret = SUCCESS;
    xss_log_client_attr_t *attr = NULL;
    void *mem_base = NULL;
    uint32_t mem_offset = 0;
    xss_stream_buf_t page_buf;
    xss_osal_sema_attr_t sema_attr;
    xss_osal_mutex_attr_t mutex_attr;
    xss_osal_task_attr_t task_attr;
    xss_osal_msgq_attr_t msgq_attr;
    char cmd[LEN_256];

    xlog_client = calloc(1, sizeof(xss_log_client_t));
    if (xlog_client == NULL) {
        return ERR_NOMEM;
    }

    memcpy(&xlog_client->attr, &xlog_attr, sizeof(xss_log_client_attr_t));
    attr = &xlog_attr;

    /* mkdir for log storage */
    snprintf(cmd, sizeof(cmd) -1, "mkdir -p %s", attr->file_dire);
    system(cmd);

    msgq_attr.name = "free list msgq";
    msgq_attr.size = attr->page_nums * sizeof(xss_stream_buf_t);
    ret = xss_osal_msgq_create(&xlog_client->free_list, &msgq_attr);
    if (ret != SUCCESS) {
        printf("xss_osal_msgq_create failed:%d\n", ret);
        goto init_failed;
    }

    msgq_attr.name = "dirty list msgq";
    msgq_attr.size = attr->page_nums * sizeof(xss_stream_buf_t);
    ret = xss_osal_msgq_create(&xlog_client->dirty_list, &msgq_attr);
    if (ret != SUCCESS) {
        printf("xss_osal_msgq_create failed:%d\n", ret);
        goto init_failed;
    }

    mutex_attr.name = "log mutex lock";
    ret = xss_osal_mutex_create(&xlog_client->lock, &mutex_attr);
    if (ret != SUCCESS) {
        printf("xss_osal_mutex_create failed:%d\n", ret);
        goto init_failed;
    }

    sema_attr.name = "log sync semaphore";
    sema_attr.init_value = 0;
    ret = xss_osal_sema_create(&xlog_client->sync_sema,&sema_attr);
    if (ret != SUCCESS) {
        printf("xss_osal_sema_create failed:%d\n", ret);
        goto init_failed;
    }

    task_attr.name = "log sync task";
    task_attr.param = (void *)xlog_client;
    task_attr.entry = log_sync_task;
    task_attr.detached = false;
    task_attr.priority = XSS_OSAL_PRIO_DEFAULT;
    task_attr.stacksize = XSS_OSAL_STACKSIZE_DEFAULT;
    ret = xss_osal_task_create(&xlog_client->sync_task, &task_attr);
    if (ret != SUCCESS) {
        printf("xss_osal_task_create failed:%d\n", ret);
        goto init_failed;
    }

    mem_offset = 0;
    mem_base = malloc(attr->page_nums * attr->page_size);
    if (mem_base == NULL) {
        ret = ERR_NOMEM;
        printf("malloc failed:%d\n", ret);
        goto init_failed;
    }

    xlog_client->page_membase = mem_base;
    for ( i = 0; i < attr->page_nums; i++) {
        page_buf.totl_len = attr->page_size;
        page_buf.data_len = 0;
        page_buf.data = mem_base + mem_offset;
        mem_offset += attr->page_size;
        ret = xss_osal_msgq_send(xlog_client->free_list, &page_buf, sizeof(xss_stream_buf_t), XSS_OSAL_WAIT_FOREVER);
        if (ret != SUCCESS) {
            printf("xss_osal_msgq_send failed:%d\n", ret);
        }
    }

    return SUCCESS;

    init_failed:
    if (mem_base) {
        free(mem_base);
    }
    if (xlog_client->sync_task) {
        xss_osal_task_destroy(xlog_client->sync_task);
    }
    if (xlog_client->sync_sema) {
        xss_osal_sema_destroy(xlog_client->sync_sema);
    }
    if (xlog_client->lock) {
        xss_osal_mutex_destroy(xlog_client->lock);
    }
    if (xlog_client->free_list) {
        xss_osal_msgq_destroy(xlog_client->free_list);
    }
    if (xlog_client->dirty_list) {
        xss_osal_msgq_destroy(xlog_client->dirty_list);
    }
    if (mem_base) {
        free(mem_base);
    }
    free(xlog_client);
    xlog_client = NULL;

    return ret;
}

xss_result_t xss_log(const char *func, int32_t func_line, int32_t module, const char *fmt, ...)
{
    char in_buffer[LEN_1024];
    char out_buffer[LEN_1024];
    struct timeval tv;
    struct tm *tm;
    uint32_t ret_len, offset;
    xss_stream_buf_t *log_buf = NULL;
    xss_result_t ret = SUCCESS;

    gettimeofday(&tv, NULL);
    tm = localtime(&tv.tv_sec);

    va_list args;
    va_start(args, fmt);
    vsnprintf(in_buffer, sizeof(in_buffer) - 1, fmt, args);
    va_end(args);

    ret_len = snprintf(out_buffer, sizeof(out_buffer) -1,
            "%4d/%02d/%02d %02d:%02d:%02d.%03d [%20s:%4d] X%02x%s",
            tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday,
            tm->tm_hour, tm->tm_min, tm->tm_sec,(int)(tv.tv_usec/1000),
            func, func_line, module, in_buffer);

    if (xlog_client) {
        uint32_t recv_size = sizeof(xss_stream_buf_t);
        xss_osal_mutex_lock(xlog_client->lock, XSS_OSAL_WAIT_FOREVER);
        log_buf = &xlog_client->log_buf;
        if (log_buf->data == NULL) {
            ret = xss_osal_msgq_recv(xlog_client->free_list, &log_buf, &recv_size, XSS_LOG_WAIT_TIMEOUT_US);
            if (ret != SUCCESS) {
                printf("xss_osal_msgq_recv failed:%d\n", ret);
                goto log_exit;
            }
        }

        if (log_buf->data != NULL) {
            if ((ret_len + log_buf->data_len) >= log_buf->totl_len) {
                memcpy(log_buf->data, out_buffer, (log_buf->totl_len - log_buf->data_len));
                offset = log_buf->totl_len - log_buf->data_len;
                ret_len -= offset;
                log_buf->data_len += offset;
                ret = xss_osal_msgq_send(xlog_client->dirty_list, &log_buf, recv_size, XSS_LOG_WAIT_TIMEOUT_US);
                if (ret != SUCCESS) {
                    goto log_exit;
                }
                log_buf->data = NULL;
                ret = xss_osal_msgq_recv(xlog_client->free_list, &log_buf, &recv_size, XSS_LOG_WAIT_TIMEOUT_US);
                if (ret != SUCCESS) {
                    goto log_exit;
                }
            }

            if (log_buf->data != NULL && ret_len) {
                memcpy(log_buf->data, &out_buffer[offset], ret_len);
                log_buf->data_len += ret_len;
            }
        }

log_exit:
        xss_osal_mutex_unlock(xlog_client->lock);
    }

    return ret;
}
