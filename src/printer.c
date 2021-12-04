/// Copyright © Alexander Kaluzhnyy

#include "json.h"
#include "json_printer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "log.h"
#include <errno.h>

typedef struct writer_t {
    size_t indent;
    ssize_t indent_num;
    ssize_t print_cnt;
    json_putchar_t putchar;
    void* data;
} writer_t;

static int put_c(writer_t* self, char c)
{
    self->print_cnt++;
    return self->putchar(c, self->data);
}

static int writer_putc_nothing(char c, size_t* value)
{
    (void)c;
    (void)value;
    return 0;
}

static int writer_putc_s(char c, char** symbol)
{
    **symbol = c;
    *symbol += 1;
    return 0;
}

static int writer_putc_f(char c, FILE* file)
{
    return fputc(c, file) >= 0 ? 0 : -1;
}

#define HANDLE_ERROR(call, ...) ({  \
    if ((call) != 0) {              \
        log_debug_msg(__VA_ARGS__); \
        return -1;                  \
    }                               \
    0;                              \
})

#define HANDLE_NULL_ERROR(call, ...) ({ \
    typeof(call) ret = (call);          \
    if (ret == NULL) {                  \
        log_debug_msg(__VA_ARGS__);     \
        return -1;                      \
    }                                   \
    ret;                                \
})

#define PUT_C(writer, c) HANDLE_ERROR(put_c(writer, c), "can't put 0x%02x", c);

static int put_s(writer_t* writer, const char* s)
{
    log_trace_func();
    for (size_t i = 0; s[i] != '\0'; i++) {
        PUT_C(writer, s[i]);
    }
    return 0;
}
#define PUT_S(writer, s) HANDLE_ERROR(put_s(writer, s), "can't put '%s'", s)

static int put_json_s(writer_t* writer, const char* str)
{
    log_trace_func();
    PUT_C(writer, '"');
    for (size_t i = 0; str[i] != '\0'; i++) {
        switch (str[i]) {
        case '"':
        case '\\':
            PUT_C(writer, '\\');
        default:
            break;
        }
        PUT_C(writer, str[i]);
    }
    return PUT_C(writer, '"');
}

#define PUT_JSON_S(writer, s) HANDLE_ERROR(put_json_s(writer, s), "can't put '%s'", s)

static int put_indent(writer_t* writer, int change)
{
    log_trace_func();
    if (writer->indent == 0) {
        return 0;
    }
    PUT_C(writer, '\n');
    writer->indent_num += change;
    for (ssize_t i = 0; i < writer->indent_num; i++) {
        for (size_t j = 0; j < writer->indent; j++) {
            PUT_C(writer, ' ');
        }
    }
    return 0;
}
#define PUT_INDENT(writer) HANDLE_ERROR(put_indent(writer, 0), "can't put indent")
#define PUT_INDENT_ADD(writer) HANDLE_ERROR(put_indent(writer, 1), "can't put indent")
#define PUT_INDENT_SUB(writer) HANDLE_ERROR(put_indent(writer, -1), "can't put indent")

#define JSON_GET_STR(self) HANDLE_NULL_ERROR(json_get_str(self), "json_get_str() return NULL")
#define JSON_KEY(self, id) HANDLE_NULL_ERROR(json_key(self, id), "json_key() return NULL")
#define JSON_GET_BY_ID(self, id) HANDLE_NULL_ERROR(json_get_by_id(self, id), "json_get_by_id() return NULL")

static int json_print_internal(json_t* self, writer_t* writer)
{
    log_trace_func();
    const char* type = json_get_type(self);
    log_debug_msg("Value type is '%s'", type);
    if (type != JSON_OBJECT && type != JSON_ARRAY) {
        log_debug_msg("Object not container. Get only value");
        const char* value = JSON_GET_STR(self);
        if (type == JSON_STRING) {
            return PUT_JSON_S(writer, value);
        }
        return PUT_S(writer, value);
    } else {
        const char* border = type == JSON_OBJECT ? "{}" : "[]";
        PUT_C(writer, border[0]);
        PUT_INDENT_ADD(writer);
        for (size_t i = 0; i < json_size(self); i++) {
            if (i != 0) {
                PUT_C(writer, ',');
            }
            PUT_INDENT(writer);
            if (type == JSON_OBJECT) {
                PUT_JSON_S(writer, JSON_KEY(self, i));
                PUT_C(writer, ':');
            }
            json_t* node = JSON_GET_BY_ID(self, i);
            if (json_print_internal(node, writer) != 0) {
                log_error_msg("json_print_internal() return error");
                return -1;
            }
        }
        PUT_INDENT_SUB(writer);
        PUT_C(writer, border[1]);
    }
    return 0;
}

ssize_t json_uniprint(json_t* self, size_t indent, json_putchar_t putchar, void* data)
{
    log_trace_func();
    writer_t writer;
    memset(&writer, 0, sizeof(writer));
    writer.data = data;
    writer.putchar = putchar;
    writer.indent = indent;
    if (json_print_internal(self, &writer) != 0) {
        log_error_msg("print error!");
        return -1;
    }
    return writer.print_cnt;
}
ssize_t json_fprint(json_t* self, size_t indent, FILE* file);

char* json_sprint(json_t* self, size_t indent)
{
    log_trace_func();
    if (self == NULL) {
        log_debug_msg("self is NULL");
        return NULL;
    }
    ssize_t size = json_uniprint(self, indent, (json_putchar_t)writer_putc_nothing, NULL);
    if (size < 0) {
        log_error_msg("can't calculate size");
        return NULL;
    }
    char* str = calloc((size_t)(size + 1), sizeof(char));
    if (str == NULL) {
        log_error_msg("calloc(): %s(%i)", strerror(errno), errno);
        return NULL;
    }

    char* iterator = str;
    if (json_uniprint(self, indent, (json_putchar_t)writer_putc_s, &iterator) < 0) {
        log_error_msg("can't print value");
        free(str);
        return NULL;
    }
    return str;
}

ssize_t json_fprint(json_t* self, size_t indent, FILE* file)
{
    log_trace_func();
    return json_uniprint(self, indent, (json_putchar_t)writer_putc_f, file);
}
