/// Copyright © Alexander Kaluzhnyy

#include "json.h"
#include "log.h"
#include <ctype.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

const char JSON_NULL[] = "null";
const char JSON_TRUE[] = "true";
const char JSON_FALSE[] = "false";
const char JSON_NUMBER[] = "number";
const char JSON_STRING[] = "string";
const char JSON_ARRAY[] = "array";
const char JSON_OBJECT[] = "object";

typedef struct json_t json_t;

typedef enum json_type_t {
    JSON_TYPE_NULL,
    JSON_TYPE_FALSE,
    JSON_TYPE_TRUE,
    JSON_TYPE_NUMBER,
    JSON_TYPE_STRING,
    JSON_TYPE_ARRAY,
    JSON_TYPE_OBJECT,
    JSON_TYPE_SIZE
} json_type_t;

typedef struct json_t json_t;

#define bitssz(size, bits) ((1 << bits) > size) ? bits
#define bit_requared(e) bitssz(e, 1) \
    : bitssz(e, 2)                   \
    : bitssz(e, 3)                   \
    : bitssz(e, 4)                   \
    : bitssz(e, 5)                   \
    : bitssz(e, 6)                   \
    : bitssz(e, 7)                   \
    : bitssz(e, 8)                   \
    : -1

typedef struct json_str_t {
    unsigned refcnt;
    char* data;
} json_str_t;

typedef struct json_arr_t {
    unsigned refcnt;
    json_t** data;
    size_t size;
} json_arr_t;

typedef struct json_t {
    json_type_t type : bit_requared(JSON_TYPE_SIZE);
    unsigned have_root : 1;
    union {
        struct {
            unsigned refcnt;
            char str[];
        } str;
        struct {
            unsigned size;
            json_t* nodes[];
        } arr;
    };
} json_t;

json_t node_null = { JSON_TYPE_NULL, 0, { { 0 } } };
json_t node_true = { JSON_TYPE_TRUE, 0, { { 0 } } };
json_t node_false = { JSON_TYPE_FALSE, 0, { { 0 } } };

static unsigned json_refcnt(const json_t* self)
{
    switch (self->type) {
    case JSON_TYPE_NUMBER:
    case JSON_TYPE_STRING:
        return self->str.refcnt;
    default:
        break;
    }
    return 0;
}

#define INSERT_PARAM_BEFORE(format, param, format2, ...) format format2, param __VA_OPT__(, ) __VA_ARGS__

#define CHECK_FUNC(call, ...) ({                                                        \
    typeof(call) CHECK_FUNC_result = (call);                                            \
    if (CHECK_FUNC_result == NULL) {                                                    \
        char CHECK_FUNC_function[] = #call;                                             \
        *strchr(CHECK_FUNC_function, '(') = 0;                                          \
        log_error_msg(INSERT_PARAM_BEFORE("%s", CHECK_FUNC_function, ":" __VA_ARGS__)); \
        return NULL;                                                                    \
    }                                                                                   \
    CHECK_FUNC_result;                                                                  \
})

#define CHECK_FUNC_ERRNO(call) ({                       \
    CHECK_FUNC(call, "%s(%i)", strerror(errno), errno); \
})

#define CALLOC(elemments, size) ({             \
    CHECK_FUNC_ERRNO(calloc(elemments, size)); \
})

#define REALLOC(source, new_size) ({             \
    CHECK_FUNC_ERRNO(realloc(source, new_size)); \
})
#define CHECK_NULL(value, ...) ({             \
    if (value == NULL) {                      \
        log_error_msg(#value " is NULL");     \
        return __VA_OPT__(__VA_ARGS__;) NULL; \
    }                                         \
})

#define FREE(value) ({ \
    free(value);       \
    (value) = NULL;    \
})

#define CLEANUP(cleaner) __attribute__((cleanup(cleaner)))
#define UNCLEANUP(ptr) ({        \
    typeof(ptr) tmp_##ptr = ptr; \
    ptr = NULL;                  \
    tmp_##ptr;                   \
})

#define CHECK_PPTR(pptr, ...) ({             \
    CHECK_NULL(pptr, ##__VA_ARGS__);         \
    typeof(*pptr) pptr##_value = *pptr;      \
    CHECK_NULL(pptr##_value, ##__VA_ARGS__); \
    pptr##_value;                            \
})

////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// INPUT
////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const char* symbol_str(char c)
{
    static char holder[] = "0x00";
    switch (c) {
    case 0x20 ... 0x7E:
        sprintf(holder, "'%c'", c);
        break;
    default:
        sprintf(holder, "0x%02X", 0xFF & c);
        break;
    }
    return holder;
}

typedef struct reader_t {
    json_getc_t getc;
    void* data;
    char current;
    struct {
        char* data;
        size_t stored;
        size_t size;
    } tmp;
} reader_t;

static const char* reader_put2s(reader_t* reader, char c)
{
    if (reader->tmp.stored == reader->tmp.size) {
        reader->tmp.data = REALLOC(reader->tmp.data, reader->tmp.size + 2);
        reader->tmp.size++;
        log_debug_msg("increase tmp to %zu", reader->tmp.size);
    }
    reader->tmp.data[reader->tmp.stored++] = c;
    reader->tmp.data[reader->tmp.stored] = 0;
    return reader->tmp.data;
}
#define READER_PUT2S(reader, c) ({                                   \
    log_debug_msg("push:%s", symbol_str(c));                         \
    CHECK_FUNC(reader_put2s(reader, c), "symbol:%s", symbol_str(c)); \
})
static const char* reader_reset_s(reader_t* reader)
{
    if (reader->tmp.size) {
        reader->tmp.data[0] = 0;
        reader->tmp.stored = 0;
    } else {
        READER_PUT2S(reader, 0);
        return reader_reset_s(reader);
    }
    return reader->tmp.data;
}

#define READER_RESET_S(reader) ({       \
    CHECK_FUNC(reader_reset_s(reader)); \
})

static const char* reader_get_s(reader_t* reader)
{
    return reader->tmp.data;
}

static reader_t reader_init(char (*getc)(void*), void* data)
{
    log_trace_func();
    reader_t self;
    memset(&self, 0, sizeof(self));
    self.data = data;
    self.getc = getc;
    self.current = getc(data);
    return self;
}

static void reader_cleanup(reader_t* reader)
{
    free(reader->tmp.data);
}

static char get_c(reader_t* self)
{
    log_trace_func();
    self->current = self->getc(self->data);
    log_debug_msg("symbol:%s", symbol_str(self->current));
    return self->current;
}

static char cur_c(reader_t* self)
{
    return self->current;
}

static char json_get_c_file(FILE* file)
{
    int symb = fgetc(file);
    if (symb == EOF) {
        log_debug_msg("EOF:%s(%i)", strerror(errno), errno);
        symb = 0;
    }
    return (char)symb;
}

static char json_get_c_str(const char** str)
{
    char ret = **str;
    (*str)++;
    return ret;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// GRAPH
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define JSON_FORMAT(node) #node ":%p{%s}{root:%u}{size:%zu}", (node), type2str((node)->type), json_refcnt(node), json_size(&node)

#define json_tmp_t json_t CLEANUP(json_deinit)

static json_t* json_init_from_value_internal(json_type_t type, const char* value_str);

static const char* types_str[]
    = {
          [JSON_TYPE_NULL] = JSON_NULL,
          [JSON_TYPE_FALSE] = JSON_FALSE,
          [JSON_TYPE_TRUE] = JSON_TRUE,
          [JSON_TYPE_NUMBER] = JSON_NUMBER,
          [JSON_TYPE_STRING] = JSON_STRING,
          [JSON_TYPE_ARRAY] = JSON_ARRAY,
          [JSON_TYPE_OBJECT] = JSON_OBJECT
      };

static const char* type2str(json_type_t type)
{
    return types_str[type];
}

static const json_type_t* str2type(const char* type_str)
{
    static json_type_t types[] = {
        [JSON_TYPE_NULL] = JSON_TYPE_NULL,
        [JSON_TYPE_FALSE] = JSON_TYPE_FALSE,
        [JSON_TYPE_TRUE] = JSON_TYPE_TRUE,
        [JSON_TYPE_NUMBER] = JSON_TYPE_NUMBER,
        [JSON_TYPE_STRING] = JSON_TYPE_STRING,
        [JSON_TYPE_ARRAY] = JSON_TYPE_ARRAY,
        [JSON_TYPE_OBJECT] = JSON_TYPE_OBJECT
    };
    for (size_t i = 0; i < sizeof(types_str) / sizeof(types_str[0]); i++) {
        if (strcmp(type_str, types_str[i]) == 0) {
            return &types[i];
        }
    }
    log_error_msg("Wrong type string '%s'", type_str);
    return NULL;
}

static void json_deinit_(json_t** self_ptr)
{
    log_trace_func();
    json_t* self = CHECK_PPTR(self_ptr, ;);
    log_debug_msg(JSON_FORMAT(self));
    switch (self->type) {
    case JSON_TYPE_NULL:
    case JSON_TYPE_FALSE:
    case JSON_TYPE_TRUE:
        log_debug_msg("base type %s: deinit not requared", type2str(self->type));
        *self_ptr = NULL;
        return;
    case JSON_TYPE_STRING:
    case JSON_TYPE_NUMBER:
        if (self->str.refcnt > 1) {
            self->str.refcnt--;
            log_debug_msg("refcnt: %zu", self->str.refcnt);
            return;
        }
        break;
    default:
        for (size_t id = 0; id < self->arr.size; id++) {
            json_deinit_(&(self->arr.nodes[id]));
        }
        break;
    }
    FREE(self);
    *self_ptr = NULL;
}

void json_deinit(json_t** self_ptr)
{
    log_trace_func();
    json_t* self = CHECK_PPTR(self_ptr, ;);
    log_debug_msg(JSON_FORMAT(self));
    if (self->have_root) {
        log_debug_msg("node have root. Deinit not requared");
        return;
    }
    json_deinit_(self_ptr);
}

json_t* json_copy(json_t** self_ptr)
{
    log_trace_func();
    json_t* self = CHECK_PPTR(self_ptr);
    log_debug_msg(JSON_FORMAT(self));
    switch (self->type) {
    case JSON_TYPE_NULL:
    case JSON_TYPE_TRUE:
    case JSON_TYPE_FALSE:
        return (json_t*)self;
    case JSON_TYPE_STRING:
    case JSON_TYPE_NUMBER:
        ((json_t*)self)->str.refcnt++;
        return (json_t*)self;
    default:
        break;
    }
    json_tmp_t* new = CALLOC(1, self->arr.size * sizeof(typeof(self->arr.nodes[0])) + sizeof(json_t));
    *new = *self;
    for (size_t i = 0; i < self->arr.size; i++) {
        new->arr.nodes[i] = CHECK_FUNC(json_copy(&self->arr.nodes[i]));
        new->arr.nodes[i]->have_root = 1;
    }
    return UNCLEANUP(new);
}

static void json_set_f(json_t** self_ptr, json_t** elem_ptr, size_t id)
{
    log_trace_func();
    json_t* self = *self_ptr;
    json_t* elem = *elem_ptr;
    log_debug_msg(JSON_FORMAT(self));
    log_debug_msg(JSON_FORMAT(elem));
    log_debug_msg("id:%zu", id);
    json_t* old = self->arr.nodes[id];
    self->arr.nodes[id] = elem;
    self->arr.nodes[id]->have_root = 1;
    json_deinit(&old);
}

const char* json_get_type(json_t** self_ptr)
{
    json_t* self = CHECK_PPTR(self_ptr);
    return type2str(self->type);
}

const char* json_get_str(json_t** self_ptr)
{
    log_trace_func();
    json_t* self = CHECK_PPTR(self_ptr);
    log_debug_msg(JSON_FORMAT(self));
    switch (self->type) {
    case JSON_TYPE_NULL:
    case JSON_TYPE_TRUE:
    case JSON_TYPE_FALSE:
        return type2str(self->type);
    case JSON_TYPE_NUMBER:
    case JSON_TYPE_STRING:
        return self->str.str;
    default:
        log_error_msg("not supported for %s type", type2str(self->type));
        return NULL;
    }
}

size_t json_size(json_t** self_ptr)
{
    json_t* self = CHECK_PPTR(self_ptr, 0);
    switch (self->type) {
    case JSON_TYPE_ARRAY:
        return self->arr.size;
    case JSON_TYPE_OBJECT:
        return self->arr.size / 2;
    default:
        break;
    }
    return 0;
}

static json_t** json_get_by_id_(json_t* self, size_t id)
{
    if (id < self->arr.size) {
        log_debug_msg("return " JSON_FORMAT(self->arr.nodes[id]));
        return &(self->arr.nodes[id]);
    }
    log_error_msg("index %zu out of range %zu", id, self->arr.size / (self->type == JSON_TYPE_OBJECT ? 2 : 1));
    return NULL;
}

json_t** json_get_by_id(json_t** self_ptr, size_t id)
{
    log_trace_func();
    json_t* self = CHECK_PPTR(self_ptr);
    log_debug_msg(JSON_FORMAT(self));
    log_debug_msg("id:%zu", id);
    switch (self->type) {
    case JSON_TYPE_OBJECT:
        id = id * 2 + 1;
        break;
    case JSON_TYPE_ARRAY:
        break;
    default:
        log_error_msg("not supported for %s type", type2str(self->type));
        return NULL;
    }
    return CHECK_FUNC(json_get_by_id_(self, id));
}

const char* json_key(json_t** self_ptr, size_t id)
{
    log_trace_func();
    json_t* self = CHECK_PPTR(self_ptr);
    log_debug_msg(JSON_FORMAT(self));
    log_debug_msg("id:%zu", id);
    switch (self->type) {
    case JSON_TYPE_OBJECT:
        id = id * 2;
        return (*CHECK_FUNC(json_get_by_id_(self, id)))->str.str;
    default:
        log_error_msg("not supported for %s type", type2str(self->type));
        return NULL;
    }
}

json_t** json_get_by_key(json_t** self_ptr, const char* key)
{
    log_trace_func();
    json_t* self = CHECK_PPTR(self_ptr);
    CHECK_NULL(key);
    log_debug_msg(JSON_FORMAT(self));
    log_debug_msg("key:%s", key);
    switch (self->type) {
    case JSON_TYPE_OBJECT:
        for (size_t i = 0; i < self->arr.size; i += 2) {
            if (strcmp(key, self->arr.nodes[i]->str.str) == 0) {
                return &(self->arr.nodes[i + 1]);
            }
        }
        log_error_msg("key '%s' not fround", key);
        return NULL;
    default:
        log_error_msg("not supported for %s type", type2str(self->type));
        return NULL;
    }
}

static json_t** json_check_circular_ref(json_t** self_ptr, json_t** elem_ptr)
{
    log_trace_func();
    json_t* self = *self_ptr;
    json_t* elem = *elem_ptr;
    log_debug_msg(JSON_FORMAT(self));
    log_debug_msg(JSON_FORMAT(elem));
    switch (elem->type) {
    case JSON_TYPE_ARRAY:
    case JSON_TYPE_OBJECT: {
        if (elem == self) {
            return NULL;
        }
        for (size_t id = 0; id < elem->arr.size; id++) {
            CHECK_FUNC(json_check_circular_ref(self_ptr, &(elem->arr.nodes[id])));
        }
        break;
    }
    default:
        break;
    }
    return elem_ptr;
}
static json_t** json_elem_copy(json_t** self_ptr, json_t** elem_ptr, int check_circular)
{
    log_trace_func();
    json_t* self = *self_ptr;
    json_t* elem = *elem_ptr;
    log_debug_msg(JSON_FORMAT(self));
    log_debug_msg(JSON_FORMAT(elem));
    log_debug_msg("check_circular:%s", check_circular ? JSON_TRUE : JSON_FALSE);
    if (self->have_root || (check_circular && json_check_circular_ref(self_ptr, elem_ptr) == NULL)) {
        *elem_ptr = json_copy(elem_ptr);
    }
    return elem_ptr;
}

static json_t** json_set_by_id_(json_t** self_ptr, json_t** elem_ptr, size_t id, int check_circular)
{
    log_trace_func();
    json_t* self = *self_ptr;
    json_t* elem = *elem_ptr;
    log_debug_msg(JSON_FORMAT(self));
    log_debug_msg(JSON_FORMAT(elem));
    log_debug_msg("id:%zu", id);
    elem_ptr = json_elem_copy(self_ptr, elem_ptr, check_circular);
    elem = *elem_ptr;
    log_debug_msg(JSON_FORMAT(elem));
    if (id == self->arr.size) {
        log_debug_msg("increase array size to %zu", self->arr.size + 1);
        self = REALLOC(self, (self->arr.size + 1) * sizeof(typeof(self->arr.nodes[0])) + sizeof(json_t));
        self->arr.nodes[self->arr.size++] = &node_null;
        *self_ptr = self;
    }
    json_set_f(self_ptr, elem_ptr, id);
    return self_ptr;
}

json_t** json_set_by_id(json_t** self_ptr, json_t** elem_ptr, size_t id)
{
    log_trace_func();
    json_t* self = CHECK_PPTR(self_ptr);
    json_t* elem = CHECK_PPTR(elem_ptr);
    log_debug_msg(JSON_FORMAT(self));
    log_debug_msg(JSON_FORMAT(elem));
    log_debug_msg("id:%zu", id);
    switch (self->type) {
    case JSON_TYPE_OBJECT:
        id = id * 2 + 1;
        if (id >= self->arr.size) {
            log_error_msg("id %zu out of range %zu", id / 2, self->arr.size / 2);
            return NULL;
        }
        break;
    case JSON_TYPE_ARRAY:
        if (id > self->arr.size) {
            log_error_msg("id %zu out of range %zu", id, self->arr.size);
            return NULL;
        }
        break;
    default:
        log_error_msg("not supported for %s type", type2str(self->type));
        return NULL;
    }

    self_ptr = CHECK_FUNC(json_set_by_id_(self_ptr, elem_ptr, id, 1));
    return self_ptr;
}

static void json_remove_last(json_t* self)
{
    log_debug_msg(JSON_FORMAT(self));
    json_deinit(&(self->arr.nodes[self->arr.size - 1]));
    self->arr.size--;
}

json_t** json_set_by_key(json_t** self_ptr, json_t** elem_ptr, const char* key)
{
    log_trace_func();
    CHECK_NULL(key);
    json_t* self = CHECK_PPTR(self_ptr);
    json_t* elem = CHECK_PPTR(elem_ptr);
    log_debug_msg(JSON_FORMAT(self));
    log_debug_msg(JSON_FORMAT(elem));
    log_debug_msg("key:%s", key);
    switch (self->type) {
    case JSON_TYPE_OBJECT: {
        break;
    }
    default:
        log_error_msg("not supported for %s type", type2str(self->type));
        return NULL;
    }
    for (size_t id = 0; id < self->arr.size; id += 2) {
        if (strcmp(key, self->arr.nodes[id]->str.str) == 0) {
            return CHECK_FUNC(json_set_by_id(self_ptr, elem_ptr, id / 2));
        }
    }
    json_tmp_t* new_key = CHECK_FUNC(json_init_from_value_internal(JSON_TYPE_STRING, key));
    CHECK_FUNC(json_set_by_id_(self_ptr, &new_key, self->arr.size, 0));
    if (json_set_by_id_(self_ptr, elem_ptr, self->arr.size, 1)) {
        log_error_msg("Can't set new key");
        json_remove_last(*self_ptr);
        return NULL;
    }
    return self_ptr;
}

json_t** json_set(json_t** self_ptr, json_t** elem_ptr)
{
    log_trace_func();
    json_t* self = CHECK_PPTR(self_ptr);
    CHECK_PPTR(elem_ptr);
    unsigned have_root = self->have_root;
    *self_ptr = *json_elem_copy(self_ptr, elem_ptr, 1);
    json_deinit_(&self);
    (*self_ptr)->have_root = have_root ? 1 : 0;
    return self_ptr;
}

static json_t* json_init_from_value_internal(json_type_t type, const char* value_str)
{
    log_trace_func();
    log_debug_msg("type:%s", type2str(type));
    json_tmp_t* self = NULL;
    switch (type) {
    case JSON_TYPE_NULL:
        return (json_t*)&node_null;
    case JSON_TYPE_FALSE:
        return (json_t*)&node_false;
    case JSON_TYPE_TRUE:
        return (json_t*)&node_true;
    case JSON_TYPE_NUMBER:
    case JSON_TYPE_STRING:
        self = CALLOC(1, sizeof(json_str_t) + strlen(value_str) + 1);
        self->type = type;
        strcpy(self->str.str, value_str);
        self->str.refcnt = 1;
        break;
    default:
        self = CALLOC(1, sizeof(json_t));
        self->type = type;
        break;
    }
    return UNCLEANUP(self);
}

static const char* parse_number(reader_t* reader);

static const char* check_number(const char* str)
{
    log_trace_func();
    log_debug_msg("check:'%s'", str);
    const char* iterator = str;
    reader_t CLEANUP(reader_cleanup) reader = reader_init((json_getc_t)json_get_c_str, &iterator);
    CHECK_FUNC(parse_number(&reader));
    iterator--;
    if (iterator[0] != 0) {
        log_debug_msg("expta symbols('%s') in number string:'%s'", &iterator[0], str);
        return NULL;
    }
    return str;
}

json_t* json_init_from_value(const char* type_str, const char* value_str)
{
    log_trace_func();
    CHECK_NULL(type_str);
    log_debug_msg("type_str:'%s'", type_str);
    const json_type_t* type = CHECK_FUNC(str2type(type_str));
    switch (*type) {
    case JSON_TYPE_NUMBER: {
        value_str = value_str ? value_str : "0";
        CHECK_FUNC(check_number(value_str));
    } break;
    case JSON_TYPE_STRING: {
        value_str = value_str ? value_str : "";
    }
    default:
        break;
    }
    return CHECK_FUNC(json_init_from_value_internal(*type, value_str));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// PARSER
////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define ARRAY_BEGIN '['
#define ARRAY_END ']'
#define OBJECT_BEGIN '{'
#define OBJECT_END '}'
#define OBJECT_DIV ':'
#define COMMA ','

#define UNEXPECTED_SYMBOL(reader) ({                                  \
    log_error_msg("Unexpected symbol %s", symbol_str(cur_c(reader))); \
})
#define json_tmp_t json_t CLEANUP(json_deinit)

static int json_cinset(char c, const char* set)
{
    log_trace_func();
    for (size_t i = 0; set[i]; i++) {
        if (c == set[i]) {
            return 1;
        }
    }
    return 0;
}

static int json_cdec2num(char c)
{
    log_trace_func();
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    return -1;
}

static int json_chex2num(char c)
{
    log_trace_func();
    log_debug_msg("char: %s", symbol_str(c));
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
    return json_cdec2num(c);
}

static void skipspaces(reader_t* reader)
{
    log_trace_func();
    if (isspace(cur_c(reader))) {
        do {
        } while (isspace(get_c(reader)));
    }
}

static int expect_token(reader_t* reader, char c)
{
    log_trace_func();
    skipspaces(reader);
    if (cur_c(reader) != c) {
        log_debug_msg("%s not found", symbol_str(c));
        return 0;
    }
    log_debug_msg("%s found", symbol_str(c));
    return 1;
}

#define PARSE_DIGIT_SEQ(reader)                          \
    ({                                                   \
        if (json_cdec2num(cur_c(reader)) >= 0) {         \
            do {                                         \
                READER_PUT2S(reader, cur_c(reader));     \
            } while (json_cdec2num(get_c(reader)) >= 0); \
        } else {                                         \
            UNEXPECTED_SYMBOL(reader);                   \
            return NULL;                                 \
        }                                                \
    })

#define MASK(size, mask) ({                                          \
    char buffer[] = #mask;                                           \
    _Static_assert(sizeof(buffer) == size + 1, "Wrong mask length"); \
    unsigned long long mask_val = 0b##mask;                          \
    mask_val;                                                        \
})

#define UTF8_SEQ_VAL_BITS 6

static json_t* json_parse_string(reader_t* reader)
{
    log_trace_func();
    log_debug_msg("parse %s", JSON_STRING);
    if (expect_token(reader, '"') == 0) {
        UNEXPECTED_SYMBOL(reader);
        return NULL;
    }
    READER_RESET_S(reader);
    for (char symbol = get_c(reader); symbol != '"'; symbol = get_c(reader)) {
        switch (symbol) {
        case '\\': {
            switch (get_c(reader)) {
            case '"':
            case '\\':
            case '/':
                symbol = cur_c(reader);
                break;
            case 'b':
                symbol = '\b';
                break;
            case 'f':
                symbol = '\f';
                break;
            case 'n':
                symbol = '\n';
                break;
            case 'r':
                symbol = '\r';
                break;
            case 't':
                symbol = '\t';
                break;
            case 'u': {
                log_debug_msg("parse utf symbol");
                unsigned result = 0;
                for (size_t i = 0; i < 4; i++) {
                    int intermediate = json_chex2num(get_c(reader));
                    if (intermediate < 0) {
                        UNEXPECTED_SYMBOL(reader);
                        return NULL;
                    }
                    result = result * 16 + (unsigned)intermediate;
                }
                const unsigned char byte_1b_mask = (unsigned char)MASK(CHAR_BIT, 01111111);
                log_debug_msg("symbol code:0x%04x", result);
                if (result <= byte_1b_mask) {
                    log_debug_msg("symbol have 1 byte");
                    symbol = (char)result;
                    break;
                } else {
                    log_debug_msg("symbol have 2 bytes");
                    const unsigned char seq_val_mask = (1 << UTF8_SEQ_VAL_BITS) - 1;
                    const unsigned char seq_val_head = (unsigned char)MASK(CHAR_BIT, 10000000);
                    const unsigned char byte_2b_mask = (unsigned char)MASK(CHAR_BIT, 00011111);
                    const unsigned char byte_2b_head = (unsigned char)MASK(CHAR_BIT, 11000000);
                    symbol = (char)((result & seq_val_mask) | seq_val_head);
                    result >>= UTF8_SEQ_VAL_BITS;
                    if (result <= byte_2b_mask) {
                        READER_PUT2S(reader, (char)(result | byte_2b_head));
                        break;
                    } else {
                        log_debug_msg("symbol have 3 bytes");
                        const unsigned char byte_3b_head = (unsigned char)MASK(CHAR_BIT, 11100000);
                        char midle = (char)((result & seq_val_mask) | seq_val_head);
                        result >>= UTF8_SEQ_VAL_BITS;
                        READER_PUT2S(reader, (char)(result | byte_3b_head));
                        READER_PUT2S(reader, midle);
                    }
                }
                break;
            }
            default:
                UNEXPECTED_SYMBOL(reader);
                return NULL;
            }
            READER_PUT2S(reader, symbol);
            break;
        }
        case '\0': {
            UNEXPECTED_SYMBOL(reader);
            return NULL;
        }
        default:
            READER_PUT2S(reader, symbol);
        }
    }
    return CHECK_FUNC(json_init_from_value_internal(JSON_TYPE_STRING, reader_get_s(reader)));
}

static const char* parse_number(reader_t* reader)
{
    log_trace_func();
    READER_RESET_S(reader);
    if (cur_c(reader) == '-') {
        READER_PUT2S(reader, cur_c(reader));
        get_c(reader);
    }
    if (cur_c(reader) == '0') {
        READER_PUT2S(reader, cur_c(reader));
        get_c(reader);
    } else {
        PARSE_DIGIT_SEQ(reader);
    }
    if (cur_c(reader) == '.') {
        log_debug_msg("Parse fractional part");
        READER_PUT2S(reader, cur_c(reader));
        get_c(reader);
        PARSE_DIGIT_SEQ(reader);
    }
    if (json_cinset(cur_c(reader), "eE")) {
        log_debug_msg("Parse exponent part");
        READER_PUT2S(reader, cur_c(reader));
        if (json_cinset(get_c(reader), "+-")) {
            READER_PUT2S(reader, cur_c(reader));
            get_c(reader);
        }
        PARSE_DIGIT_SEQ(reader);
    }
    return reader_get_s(reader);
}

static json_t* json_parse(reader_t* reader)
{
    log_trace_func();
    skipspaces(reader);
    char symb = cur_c(reader);
    json_tmp_t* value = NULL;
    switch (symb) {
    case ARRAY_BEGIN: {
        log_debug_msg("parse %s", JSON_ARRAY);
        get_c(reader);
        value = CHECK_FUNC(json_init_from_value(JSON_ARRAY, NULL));
        if (expect_token(reader, ARRAY_END)) {
            break;
        }
        do {
            json_tmp_t* elem = CHECK_FUNC(json_parse(reader));
            if (elem->type != JSON_TYPE_NUMBER) {
                get_c(reader);
            }
            CHECK_FUNC(json_set_by_id_(&value, &elem, value->arr.size, 0));
        } while (expect_token(reader, COMMA) && (get_c(reader) || 1));
        if (expect_token(reader, ARRAY_END) == 0) {
            UNEXPECTED_SYMBOL(reader);
            return NULL;
        }
        break;
    }
    case OBJECT_BEGIN: {
        log_debug_msg("parse %s", JSON_OBJECT);
        get_c(reader);
        value = CHECK_FUNC(json_init_from_value_internal(JSON_TYPE_OBJECT, NULL));
        log_debug_msg(JSON_FORMAT(value));
        if (expect_token(reader, OBJECT_END)) {
            break;
        }
        do {
            json_tmp_t* key = CHECK_FUNC(json_parse_string(reader));
            CHECK_FUNC(json_set_by_id_(&value, &key, value->arr.size, 0));
            get_c(reader);
            if (expect_token(reader, OBJECT_DIV) == 0) {
                UNEXPECTED_SYMBOL(reader);
                return NULL;
            }
            get_c(reader);
            json_tmp_t* elem = CHECK_FUNC(json_parse(reader));
            if (elem->type != JSON_TYPE_NUMBER) {
                get_c(reader);
            }
            CHECK_FUNC(json_set_by_id_(&value, &elem, value->arr.size, 0));
        } while (expect_token(reader, COMMA) && (get_c(reader) || 1));

        if (expect_token(reader, OBJECT_END) == 0) {
            UNEXPECTED_SYMBOL(reader);
            return NULL;
        }
        break;
    }
    case '"': {
        value = CHECK_FUNC(json_parse_string(reader));
        break;
    }
    case 't':
    case 'f':
    case 'n': {
        const char* types[] = {
            ['t'] = JSON_TRUE,
            ['f'] = JSON_FALSE,
            ['n'] = JSON_NULL,
        };
        const char* type = types[(unsigned)symb];
        log_debug_msg("parse %s", type);
        for (size_t idx = 1; type[idx] != '\0'; idx++) {
            get_c(reader);
            if (type[idx] != cur_c(reader)) {
                UNEXPECTED_SYMBOL(reader);
                return NULL;
            }
        }
        value = CHECK_FUNC(json_init_from_value(type, NULL));
        break;
    }
    default: {
        log_debug_msg("parse %s", JSON_NUMBER);
        value = CHECK_FUNC(json_init_from_value_internal(JSON_TYPE_NUMBER, CHECK_FUNC(parse_number(reader))));
        break;
    }
    }
    log_debug_msg("parsing success:" JSON_FORMAT(value));
    return UNCLEANUP(value);
}

json_t* json_init_from(json_getc_t getc, void* data)
{
    log_trace_func();
    CHECK_NULL(getc);
    reader_t reader CLEANUP(reader_cleanup) = reader_init(getc, data);
    json_t* self = CHECK_FUNC(json_parse(&reader));
    return self;
}

json_t* json_init_from_file(FILE* file)
{
    log_trace_func();
    CHECK_NULL(file);
    return CHECK_FUNC(json_init_from((json_getc_t)json_get_c_file, file));
}

json_t* json_init_from_str(const char* str, const char** endptr)
{
    log_trace_func();
    if (endptr != NULL) {
        *endptr = NULL;
    } else {
        log_debug_msg("endptr is NULL");
    }
    CHECK_NULL(str);
    log_debug_msg("parse:\n%s", str);
    json_t* self = json_init_from((json_getc_t)json_get_c_str, &str);
    if (endptr != NULL) {
        if (self == NULL || json_get_type(&self) == JSON_NUMBER) {
            str--;
        }
        log_debug_msg("%p", str);
        *endptr = str;
        log_debug_msg("endptr:'%s'", *endptr);
    } else {
        log_debug_msg("endptr is NULL");
    }
    if (self == NULL) {
        log_error_msg("parsing error!");
    }
    return self;
}