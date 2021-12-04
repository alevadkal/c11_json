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
    JSON_TYPE_NULL = 0,
    JSON_TYPE_FALSE = 1,
    JSON_TYPE_TRUE = 2,
    JSON_TYPE_NUMBER = 3,
    JSON_TYPE_STRING = 4,
    JSON_TYPE_ARRAY = 5,
    JSON_TYPE_OBJECT = 6,
} json_type_t;

typedef struct json_t json_t;

typedef struct json_t {
    json_type_t type;
    unsigned refcnt;
    union {
        char* str;
        struct {
            json_t** data;
            unsigned size;
        };
    };
} json_t;

#define LOGABLE 1

#define INSERT_PARAM_BEFORE(format, param, format2, ...) format format2, param __VA_OPT__(, ) __VA_ARGS__

#define CHECK_FUNC(call, ...) ({                                                            \
    typeof(call) CHECK_FUNC_result = (call);                                                \
    if (CHECK_FUNC_result == NULL) {                                                        \
        if (LOGABLE) {                                                                      \
            char CHECK_FUNC_function[] = #call;                                             \
            *strchr(CHECK_FUNC_function, '(') = 0;                                          \
            log_error_msg(INSERT_PARAM_BEFORE("%s", CHECK_FUNC_function, ":" __VA_ARGS__)); \
        }                                                                                   \
        return NULL;                                                                        \
    }                                                                                       \
    CHECK_FUNC_result;                                                                      \
})

#define CHECK_FUNC_ERRNO(call) ({                       \
    CHECK_FUNC(call, "%s(%i)", strerror(errno), errno); \
})

#define MALLOC(size) ({                \
    CHECK_FUNC_ERRNO(calloc(1, size)); \
})

#define CALLOC(elemments, size) ({             \
    CHECK_FUNC_ERRNO(calloc(elemments, size)); \
})

#define STRDUP(str) ({             \
    CHECK_FUNC_ERRNO(strdup(str)); \
})

#define REALLOC(source, new_size) ({             \
    CHECK_FUNC_ERRNO(realloc(source, new_size)); \
})

#define FREE(ptr) ({ \
    free(ptr);       \
    ptr = NULL;      \
})

#define CHECK(rule) ({                    \
    if (!(rule)) {                        \
        log_error_msg(#rule " is false"); \
        return NULL;                      \
    }                                     \
})

#define CHECK_NULL(value) ({              \
    if (value == NULL) {                  \
        log_error_msg(#value " is NULL"); \
        return NULL;                      \
    }                                     \
})

#define CLEANUP(cleaner) __attribute__((cleanup(cleaner##_cleanup)))
#define UNCLEANUP(ptr) ({                  \
    typeof(ptr) get_uncleanable_ptr = ptr; \
    ptr = NULL;                            \
    get_uncleanable_ptr;                   \
})

#define MAKE_CLEANUP_IMPL(cleaner, type) \
    void cleaner##_cleanup(type* ptr)    \
    {                                    \
        if (*ptr == NULL) {              \
            return;                      \
        }                                \
        cleaner(*ptr);                   \
    }

static MAKE_CLEANUP_IMPL(json_deinit, json_t*);

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
    char (*getc)(void*);
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

#define JSON_FORMAT(node) #node ":%p{%u}{%s}", node, json_get_refcnt(node), type2str(node->type)
#define json_tmp_t json_t CLEANUP(json_deinit)

static json_type_t types[]
    = {
          [JSON_TYPE_NULL] = JSON_TYPE_NULL,
          [JSON_TYPE_FALSE] = JSON_TYPE_FALSE,
          [JSON_TYPE_TRUE] = JSON_TYPE_TRUE,
          [JSON_TYPE_NUMBER] = JSON_TYPE_NUMBER,
          [JSON_TYPE_STRING] = JSON_TYPE_STRING,
          [JSON_TYPE_ARRAY] = JSON_TYPE_ARRAY,
          [JSON_TYPE_OBJECT] = JSON_TYPE_OBJECT
      };

static const char* types_str[] = {
    [JSON_TYPE_NULL] = JSON_NULL,
    [JSON_TYPE_FALSE] = JSON_FALSE,
    [JSON_TYPE_TRUE] = JSON_TRUE,
    [JSON_TYPE_NUMBER] = JSON_NUMBER,
    [JSON_TYPE_STRING] = JSON_STRING,
    [JSON_TYPE_ARRAY] = JSON_ARRAY,
    [JSON_TYPE_OBJECT] = JSON_OBJECT
};

void json_deinit(json_t* self);

static const char* type2str(json_type_t type)
{
    return types_str[type];
}

static const json_type_t* str2type(const char* type_str)
{
    for (size_t i = 0; i < sizeof(types_str) / sizeof(types_str[0]); i++) {
        if (strcmp(type_str, types_str[i]) == 0) {
            return &types[i];
        }
    }
    log_error_msg("Wrong type string '%s'", type_str);
    return NULL;
}

static size_t json_get_refcnt(json_t* self)
{
    switch (self->type) {
    case JSON_TYPE_NULL:
    case JSON_TYPE_TRUE:
    case JSON_TYPE_FALSE:
        return 1;
    default:
        return self->refcnt;
    }
}

void json_deinit(json_t* self)
{
    log_trace_func();
    if (self == NULL) {
        log_debug_msg("self is NULL");
        return;
    }
    log_debug_msg(JSON_FORMAT(self));
    switch (self->type) {
    case JSON_TYPE_NULL:
    case JSON_TYPE_FALSE:
    case JSON_TYPE_TRUE:
        break;
    default: {
        self->refcnt--;
        if (self->refcnt) {
            log_debug_msg(JSON_FORMAT(self));
            return;
        }
    }
    }
    log_debug_msg("Clean resources");
    switch (self->type) {
    case JSON_TYPE_NUMBER:
    case JSON_TYPE_STRING:
        FREE(self->str);
        self->str = NULL;
        break;
    case JSON_TYPE_ARRAY:
    case JSON_TYPE_OBJECT:
        if (self->data == NULL) {
            log_error_msg("self.data is NULL");
            break;
        }
        for (size_t i = 0; i < self->size; i++) {
            json_deinit(self->data[i]);
            self->data[i] = NULL;
        }
        FREE(self->data);
        break;
    default:
        return;
    }
    memset(self, 0, sizeof(*self));
    FREE(self);
    return;
}

static json_t* json_increase_refcnt(json_t* self)
{
    log_trace_func();
    log_debug_msg(JSON_FORMAT(self));
    switch (self->type) {
    case JSON_TYPE_NULL:
    case JSON_TYPE_TRUE:
    case JSON_TYPE_FALSE:
        break;
    default:
        self->refcnt++;
    }
    return self;
}

static json_t* json_copy(json_t* self)
{
    CHECK_NULL(self);
    log_debug_msg(JSON_FORMAT(self));
    json_tmp_t* new = NULL;
    switch (self->type) {
    case JSON_TYPE_NULL:
    case JSON_TYPE_TRUE:
    case JSON_TYPE_FALSE:
        break;
    default:
        new = CALLOC(1, sizeof(*self));
        break;
    }
    switch (self->type) {
    case JSON_TYPE_NUMBER:
    case JSON_TYPE_STRING:
        new->str = STRDUP(self->str);
        break;
    default:
        new->data = CALLOC(self->size, sizeof(typeof(self->data[0])));
        new->size = self->size;
        for (size_t i = 0; i < self->size; i++) {
            new->data[i] = json_increase_refcnt(self->data[i]);
        }
    };
    new->type = self->type;
    return json_increase_refcnt(UNCLEANUP(new));
}

const char* json_get_type(json_t* self)
{
    log_trace_func();
    CHECK_NULL(self);
    log_debug_msg(JSON_FORMAT(self));
    return type2str(self->type);
}

const char* json_get_str(json_t* self)
{
    log_trace_func();
    CHECK_NULL(self);
    log_debug_msg(JSON_FORMAT(self));
    switch (self->type) {
    case JSON_TYPE_NULL:
    case JSON_TYPE_TRUE:
    case JSON_TYPE_FALSE:
        return type2str(self->type);
    case JSON_TYPE_NUMBER:
    case JSON_TYPE_STRING:
        return self->str;
    default:
        log_error_msg("not supported for %s type", type2str(self->type));
        return NULL;
    }
}

size_t json_size(json_t* self)
{
    log_trace_func();
    if (self == NULL) {
        log_error_msg("self is NULL");
        return 0;
    }
    log_debug_msg(JSON_FORMAT(self));
    size_t ret = 0;
    switch (self->type) {
    case JSON_TYPE_ARRAY:
        ret = self->size;
        break;
    case JSON_TYPE_OBJECT:
        ret = self->size / 2;
        break;
    default:
        log_error_msg("not supported for %s type", type2str(self->type));
        return 0;
    }
    log_debug_msg("ret:%zu", ret);
    return ret;
}

json_t* json_get_by_id(json_t* self, size_t id)
{
    log_trace_func();
    CHECK_NULL(self);
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
    if (id < self->size) {
        log_debug_msg(JSON_FORMAT(self->data[id]));
        return self->data[id];
    }
    log_error_msg("index %zu out of range %zu", id, self->size / (self->type == JSON_TYPE_OBJECT ? 2 : 1));
    return NULL;
}

const char* json_key(json_t* self, size_t id)
{
    log_trace_func();
    CHECK_NULL(self);
    log_debug_msg(JSON_FORMAT(self));
    log_debug_msg("id:%zu", id);
    switch (self->type) {
    case JSON_TYPE_OBJECT:
        id = id * 2;
        if (id < self->size) {
            return self->data[id]->str;
        }
        log_error_msg("index %zu out of range %zu", id / 2, self->size / 2);
        return NULL;
    default:
        log_error_msg("not supported for %s type", type2str(self->type));
        return NULL;
    }
    log_error_msg("index %zu out of range %zu", id, self->size / 2);
    return NULL;
}

json_t* json_get_by_key(json_t* self, const char* key)
{
    log_trace_func();
    CHECK_NULL(self);
    CHECK_NULL(key);
    log_debug_msg(JSON_FORMAT(self));
    log_debug_msg("key:%s", key);
    switch (self->type) {
    case JSON_TYPE_OBJECT:
        for (size_t i = 0; i < self->size; i += 2) {
            if (strcmp(key, self->data[i]->str) == 0) {
                return self->data[i + 1];
            }
        }
        log_error_msg("key '%s' not fround", key);
        return NULL;
    default:
        log_error_msg("not supported for %s type", type2str(self->type));
        return NULL;
    }
}

static json_t* json_set_by_id_internal(json_t* self, json_t* elem, size_t id)
{
    log_trace_func();
    if (id == self->size) {
        self->data = REALLOC(self->data, (self->size + 1) * sizeof(typeof(self->data[0])));
        self->data[self->size] = NULL;
        self->size++;
    }
    json_t* old = self->data[id];
    self->data[id] = json_increase_refcnt(elem);
    json_deinit(old);
    return self;
}

json_t* json_set_by_id(json_t** self_ptr, json_t* elem, size_t id)
{
    log_trace_func();
    CHECK_NULL(self_ptr);
    CHECK_NULL(elem);
    json_t* self = *self_ptr;
    CHECK_NULL(self);
    log_debug_msg(JSON_FORMAT(self));
    log_debug_msg(JSON_FORMAT(elem));
    log_debug_msg("id:%zu", id);
    switch (self->type) {
    case JSON_TYPE_OBJECT:
        id = id * 2 + 1;
        if (id >= self->size) {
            log_error_msg("id %zu out of range %zu", id / 2, self->size / 2);
            return NULL;
        }
        break;
    case JSON_TYPE_ARRAY:
        if (id > self->size) {
            log_error_msg("id %zu out of range %zu", id, self->size);
            return NULL;
        }
        break;
    default:
        log_error_msg("not supported for %s type", type2str(self->type));
        return NULL;
    }
    log_debug_msg("real id:%zu", id);
    if (self->refcnt > 1) {
        log_debug_msg("refcnt more than 1, cow requared");
        json_tmp_t* new = CHECK_FUNC(json_copy(self));
        CHECK_FUNC(json_set_by_id_internal(new, elem, id));
        json_deinit(self);
        self = UNCLEANUP(new);
    } else if (self == elem) {
        log_debug_msg("cirsular ref, cow requared");
        json_tmp_t* new = CHECK_FUNC(json_copy(self));
        CHECK_FUNC(json_set_by_id_internal(new, elem, id));
        self = UNCLEANUP(new);
    } else {
        log_debug_msg("regular set");
        self = CHECK_FUNC(json_set_by_id_internal(self, elem, id));
    }
    *self_ptr = self;
    return self;
}

static json_t* json_insert_by_key(json_t* self, json_t* elem, const char* key)
{
    log_trace_func();
    log_debug_msg(JSON_FORMAT(self));
    log_debug_msg(JSON_FORMAT(elem));
    log_debug_msg("key:%s", key);
    json_tmp_t* key_obj = CHECK_FUNC(json_init_from_value(JSON_STRING, key));
    log_debug_msg(JSON_FORMAT(key_obj));
    self->data = REALLOC(self->data, (self->size + 1) * 2 * sizeof(typeof(self->data[0])));
    self->data[self->size++] = NULL;
    self->data[self->size++] = NULL;
    self->data[self->size - 2] = UNCLEANUP(key_obj);
    self->data[self->size - 1] = json_increase_refcnt(elem);
    return self;
}

json_t* json_set_by_key(json_t** self_ptr, json_t* elem, const char* key)
{
    log_trace_func();
    CHECK_NULL(self_ptr);
    CHECK_NULL(elem);
    CHECK_NULL(key);
    json_t* self = *self_ptr;
    CHECK_NULL(self);
    log_debug_msg(JSON_FORMAT(self));
    log_debug_msg(JSON_FORMAT(elem));
    log_debug_msg("key:%s", key);
    switch (self->type) {
    case JSON_TYPE_OBJECT:
        for (size_t id = 0; id < self->size; id += 2) {
            if (strcmp(key, self->data[id]->str) == 0) {
                return CHECK_FUNC(json_set_by_id(self_ptr, elem, id / 2));
            }
        }
        if (self->refcnt > 1) {
            json_tmp_t* new = CHECK_FUNC(json_copy(self));
            CHECK_FUNC(json_insert_by_key(new, elem, key));
            json_deinit(self);
            self = UNCLEANUP(new);
        } else if (self == elem) {
            json_tmp_t* new = CHECK_FUNC(json_copy(self));
            CHECK_FUNC(json_insert_by_key(new, elem, key));
            self = UNCLEANUP(new);
        } else {
            CHECK_FUNC(json_insert_by_key(self, elem, key));
        }
        break;
    default:
        log_error_msg("not supported for %s type", type2str(self->type));
        return NULL;
    }
    *self_ptr = self;
    return self;
}

static json_t* json_init_from_value_internal(json_type_t type, const char* value_str)
{
    log_trace_func();
    json_tmp_t* self = NULL;
    switch (type) {
    case JSON_TYPE_NULL:
    case JSON_TYPE_FALSE:
    case JSON_TYPE_TRUE:
        self = (json_t*)&types[type];
        log_debug_msg(JSON_FORMAT(self));
        return UNCLEANUP(self);
    default:
        break;
    }
    self = MALLOC(sizeof(json_t));
    self->type = type;
    self->refcnt = 1;
    switch (self->type) {
    case JSON_TYPE_NUMBER:
    case JSON_TYPE_STRING:
        self->str = STRDUP(value_str);
        log_debug_msg("value:%s", self->str);
        break;
    default:
        break;
    }
    log_debug_msg(JSON_FORMAT(self));
    return UNCLEANUP(self);
}

static const char* parse_number(reader_t* reader);

static const char* check_number(const char* str)
{
    log_trace_func();
    log_debug_msg("check:'%s'", str);
    const char* iterator = str;
    reader_t CLEANUP(reader) reader = reader_init((json_getc_t)json_get_c_str, &iterator);
    CHECK_FUNC(parse_number(&reader));
    iterator--;
    CHECK(iterator[0] == 0);
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

static json_t* json_push(json_t** self, json_t* value)
{
    log_trace_func();
    return json_set_by_id(self, value, json_size(*self));
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
            if (json_get_type(elem) != JSON_NUMBER) {
                get_c(reader);
            }
            value = CHECK_FUNC(json_push(&value, elem));
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
        value = CHECK_FUNC(json_init_from_value(JSON_OBJECT, NULL));
        if (expect_token(reader, OBJECT_END)) {
            break;
        }
        do {
            json_tmp_t* key = CHECK_FUNC(json_parse_string(reader));
            get_c(reader);
            if (expect_token(reader, OBJECT_DIV) == 0) {
                UNEXPECTED_SYMBOL(reader);
                return NULL;
            }
            get_c(reader);
            json_tmp_t* elem = CHECK_FUNC(json_parse(reader));
            if (json_get_type(elem) != JSON_NUMBER) {
                get_c(reader);
            }
            value = CHECK_FUNC(json_set_by_key(&value, elem, json_get_str(key)));

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
    log_debug_msg("parsing success");
    return UNCLEANUP(value);
}

json_t* json_init_from(json_getc_t getc, void* data)
{
    log_trace_func();
    CHECK_NULL(getc);
    reader_t reader CLEANUP(reader) = reader_init(getc, data);
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
        if (self == NULL || json_get_type(self) == JSON_NUMBER) {
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