/// Copyright © Alexander Kaluzhnyy

#ifndef LOG_H_INCLUDED
#define LOG_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

void log_msg_internal(const char* file, int line, const char* format, ...);
void log_trace_end(int*);
void log_trace_start(const char* function, const char* file, int line);

#define __LOG_FILE__ (&(strrchr("/" __FILE__, '/')[1]))

#define log_msg(format, ...) log_msg_internal(__LOG_FILE__, __LINE__, format, ##__VA_ARGS__)

#define log_error_msg(format, ...) log_msg("\e[31merror\e[0m:" format, ##__VA_ARGS__)

#define log_debug_msg(format, ...) log_msg("debug:" format, ##__VA_ARGS__)

#define log_trace_func()                                            \
    __attribute__((cleanup(log_trace_end))) int log_trace_func_var; \
    (void)log_trace_func_var;                                       \
    log_trace_start(__FUNCTION__, __LOG_FILE__, __LINE__)

#ifdef __cplusplus
}
#endif

#endif // LOG_H_INCLUDED
