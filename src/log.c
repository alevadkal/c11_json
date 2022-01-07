/// Copyright © Alexander Kaluzhnyy

#include "log.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

static size_t indent_cnt = 0;
static int state = 0;

static void log_indent(void)
{
    for (size_t i = 0; i < indent_cnt; i++) {
        putchar(' ');
        putchar(' ');
        putchar(' ');
        putchar(' ');
    }
}

void log_msg_internal(const char* file, int line, const char* format, ...)
{
    if (state == 0) {
        putchar('\n');
        state = 1;
    }
    log_indent();
    va_list args;
    printf("%s:%i:", file, line);
    va_start(args, format);
    vprintf(format, args);
    putchar('\n');
    state = 1;
    fflush(stdout);
    va_end(args);
}
void log_trace_end(int* unused)
{
    indent_cnt--;
    if (state) {
        log_indent();
    }
    putchar('}');
    putchar('\n');
    state = 1;
    fflush(stdout);
    (void)unused;
}
void log_trace_start(const char* function, const char* file, int line)
{
    if (state == 0) {
        putchar('\n');
    }
    log_indent();
    printf("%s(%s:%i){", function, file, line - 2);
    fflush(stdout);
    state = 0;
    indent_cnt++;
}
