#include <stdarg.h>
#include <sys/time.h>
#include <time.h>


xss_result_t xss_log(const char *func, int32_t func_line, int32_t module, const char *fmt, ...)
{
    char in_buffer[LEN_1024];
    struct timeval tv;
    struct tm *tm;

    gettimeofday(&tv, NULL);
    tm = localtime(&tv.tv_sec);

    va_list args;
    va_start(args, fmt);
    vsnprintf(in_buffer, sizeof(in_buffer) - 1, fmt, args);
    va_end(args);

    printf("%4d/%02d/%02d %02d:%02d:%02d.%03d [%32s:%4d] M%02x %s", 
            tm->tm_year+1900, tm->tm_mon+1, tm->tm_mday, 
            tm->tm_hour, tm->tm_min, tm->tm_sec,(int)(tv.tv_usec/1000),
            func, func_line, module, in_buffer);

    return SUCCESS;
}
