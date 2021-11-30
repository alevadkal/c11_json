/// Copyright © Alexander Kaluzhnyy

#include "c11_json.h"
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>

const char JSON_TRUE[] = "true";
const char JSON_FALSE[] = "false";
const char JSON_NULL[] = "null";
const char JSON_BOOL[] = "bool";
const char JSON_NUMBER[] = "number";
const char JSON_STRING[] = "string";
const char JSON_ARRAY[] = "array";
const char JSON_OBJECT[] = "object";
const char json_pair[] = "pair";

typedef struct json_node_t json_node_t;

struct json_t {
    json_node_t* node;
};

typedef struct json_simple_t {
    char* value;
    size_t use_cnt;
    const char* type;
} json_simple_t;

typedef struct json_array_t {
    json_node_t** values;
    size_t use_cnt;
    const char* type;
    size_t size;
} json_array_t;

typedef struct json_object_t {
    json_node_t** values;
    size_t use_cnt;
    const char* type;
    size_t size;
    char** keys;
} json_object_t;

struct json_node_t {
    union {
        const char* value;
        json_simple_t simple;
        json_array_t array;
        json_object_t object;
    };
};

_Static_assert(offsetof(json_object_t, values) == offsetof(json_simple_t, value), "Wrong offset!");
_Static_assert(offsetof(json_object_t, values) == offsetof(json_array_t, values), "Wrong offset!");

_Static_assert(offsetof(json_object_t, use_cnt) == offsetof(json_simple_t, use_cnt), "Wrong offset!");
_Static_assert(offsetof(json_object_t, use_cnt) == offsetof(json_array_t, use_cnt), "Wrong offset!");

_Static_assert(offsetof(json_object_t, type) == offsetof(json_simple_t, type), "Wrong offset!");
_Static_assert(offsetof(json_object_t, type) == offsetof(json_array_t, type), "Wrong offset!");

_Static_assert(offsetof(json_object_t, size) == offsetof(json_array_t, size), "Wrong offset!");

static void json_node_deinit(json_node_t* self);

static int json_node_is_base_value(json_node_t* self)
{
    if (self->value == JSON_NULL || self->value == JSON_FALSE || self->value == JSON_TRUE) {
        return 1;
    }
    return 0;
}

static json_node_t* json_node_increase_use_cnt(json_node_t* self)
{
    if (!json_node_is_base_value(self)) {
        self->simple.use_cnt++;
    }
    return self;
}

static void json_node_decrease_use_cnt(json_node_t* self)
{
    if (json_node_is_base_value(self)) {
        return;
    }
    if (--self->simple.use_cnt == 0) {
        json_node_deinit(self);
    };
}

static json_t* json_create_holder(json_node_t* self)
{
    json_t* value = calloc(1, sizeof(json_t));
    json_node_increase_use_cnt(self);
    value->node = self;
    return value;
}

static int json_node_is_simple_value(json_node_t* self)
{
    if (self->value == JSON_NUMBER || self->value == JSON_STRING) {
        return 1;
    }
    return 0;
}

static json_node_t* json_node_copy(json_node_t* self)
{
    if (json_node_is_base_value(self)) {
        return self;
    }
    if (json_node_is_simple_value(self)) {
        json_simple_t* out = calloc(1, sizeof(json_simple_t));
        if (out == NULL) {
            return NULL;
        }
        *out = *(json_simple_t*)self;
        out->value = strdup(self->simple.value);
        if (out->value == NULL) {
            free(out);
            return NULL;
        }
        out->use_cnt++;
        return (json_node_t*)out;
    }
    size_t keys_element_size = sizeof(self->object.values[0]); // NOLINT clang-tidy(bugprone-sizeof-expression)
    size_t element_size = sizeof(self->array.values[0]); // NOLINT clang-tidy(bugprone-sizeof-expression)
    json_node_t** values = calloc(self->array.size, element_size);
    if (values == NULL) {
        return NULL;
    }
    json_node_t* out = calloc(1, self->simple.type == JSON_OBJECT ? sizeof(json_object_t) : sizeof(json_array_t));
    if (out == NULL) {
        free(values);
        return NULL;
    }
    if (self->simple.type == JSON_OBJECT) {
        char** keys = calloc(self->object.size, keys_element_size);
        for (size_t i = 0; i < self->object.size; i++) {
            keys[i] = strdup(self->object.keys[i]);
            if (keys[i] == NULL) {
                while (i--) {
                    free(keys[i]);
                }
                free(keys);
                free(values);
                free(out);
                return NULL;
            }
        }
        out->object.keys = keys;
    }
    for (size_t i = 0; i < self->array.size; i++) {
        values[i] = json_node_increase_use_cnt(self->array.values[i]);
    }
    out->array.values = values;
    out->simple.use_cnt++;
    return out;
}
static json_t* json_cow(json_t* value)
{
    json_node_t* self = value->node;
    if (json_node_is_base_value(self) || self->simple.use_cnt == 1) {
        return value;
    }
    json_node_t* new_node = json_node_copy(self);
    if (new_node == NULL) {
        return NULL;
    }
    json_node_decrease_use_cnt(self);
    value->node = new_node;
    return value;
}

static void json_node_deinit(json_node_t* self)
{
    if (json_node_is_base_value(self)) {
        return;
    }
    if (self->simple.type == JSON_OBJECT) {
        for (size_t i = 0; i < self->object.size; i++) {
            free(self->object.keys[i]);
        }
        free(self->object.keys);
        self->object.type = JSON_ARRAY;
    }
    if (self->simple.type == JSON_ARRAY) {
        for (size_t i = 0; i < self->array.size; i++) {
            json_node_decrease_use_cnt(self->array.values[i]);
        }
        free(self->array.values);
    }
    if (json_node_is_simple_value(self)) {
        free(self->simple.value);
    }
    free(self);
}

const char* json_get_type(json_t* value)
{
    if (value == NULL) {
        return NULL;
    }
    json_node_t* self = value->node;
    if (self->value == JSON_FALSE) {
        return JSON_BOOL;
    }
    if (self->value == JSON_TRUE) {
        return JSON_BOOL;
    }
    if (self->value == JSON_NULL) {
        return JSON_NULL;
    }
    return self->simple.type;
}
const char* json_get_value(json_t* value)
{
    if (value == NULL) {
        return NULL;
    }
    json_node_t* self = value->node;
    if (self->value == JSON_ARRAY || self->value == JSON_OBJECT) {
        return NULL;
    }
    if (self->value == JSON_TRUE) {
        return self->value;
    }
    if (self->value == JSON_FALSE) {
        return self->value;
    }
    if (self->value == JSON_NULL) {
        return self->value;
    }
    return self->simple.value;
}
size_t json_size(json_t* value)
{
    if (value == NULL) {
        return 0;
    }
    json_node_t* self = value->node;
    if (self->value == JSON_ARRAY || self->value == JSON_OBJECT) {
        return self->array.size;
    }
    return 0;
}
json_t* hide_json_get_node_by_id(json_t* value, size_t id)
{
    if (value == NULL) {
        return 0;
    }
    json_node_t* self = value->node;
    if (self->value != JSON_ARRAY || self->value != JSON_OBJECT) {
        return NULL;
    }
    if (self->array.size >= id) {
        return NULL;
    }
    return json_create_holder(self->array.values[id]);
}
json_t* hide_json_get_node_by_key(json_t* value, const char* key)
{
    if (value == NULL) {
        return 0;
    }
    json_node_t* self = value->node;
    if (self->value != JSON_OBJECT) {
        return NULL;
    }
    json_node_t* out = NULL;
    for (size_t id = 0; id < self->object.size; id++) {
        if (strcmp(self->object.keys[id], key) == 0) {
            out = self->object.values[id];
            break;
        }
    }
    return out ? json_create_holder(out) : NULL;
}

void json_deinit(json_t* self)
{
    if (self == NULL) {
        return;
    }
    json_node_decrease_use_cnt(self->node);
    free(self);
}

static json_node_t* avoid_circular_ref(json_node_t* self, json_node_t* value)
{
    if (json_node_is_base_value(value) || json_node_is_simple_value(value)) {
        return value;
    }
    if (value == self) {
        value = json_node_copy(value);
        if (value == NULL) {
            return NULL;
        }
        return value;
    }
    for (size_t i = 0; i < self->array.size; i++) {
        if (self->array.values[i] == value) {
            value = json_node_copy(value);
            if (value == NULL) {
                return NULL;
            }
            return value;
        }
    }
}

static json_node_t* json_node_set_value_by_id(json_node_t* self, json_node_t* value, size_t id)
{
    if (self->array.size == id) {
        size_t element_size = sizeof(self->array.values[0]); // NOLINT clang-tidy(bugprone-sizeof-expression)
        typeof(self->array.values) new_values = reallocarray(self->array.values, self->array.size + 1, element_size);
        if (new_values == NULL) {
            return NULL;
        }
        self->array.values = new_values;
        self->array.values[self->array.size++] = (json_node_t*)JSON_NULL;
    }
    json_node_decrease_use_cnt(self->array.values[id]);
    self->array.values[id] = value;
    json_node_increase_use_cnt(value);
    return self;
}

json_t* hide_json_set_node_by_id(json_t* value, json_t* element, size_t id)
{
    if (value == NULL) {
        return NULL;
    }
    json_node_t* self = value->node;
    if (element == NULL) {
        return NULL;
    }
    json_node_t* element_node = element->node;
    if (json_node_is_base_value(self) || json_node_is_simple_value(self)) {
        return NULL;
    }
    if (self->array.size > id) {
        return NULL;
    }
    if (self->array.size == id && self->array.type != JSON_ARRAY) {
        return NULL;
    }
    value = json_cow(value);
    if (value == NULL) {
        return NULL;
    }
    self = value->node;
    return json_node_set_value_by_id(self, element_node, id) ? value : NULL;
}
static json_t* json_push(json_t* self, json_t* value)
{
    return hide_json_set_node_by_id(self, value, json_size(self));
}

json_t* hide_json_set_node_by_key(json_t* value, json_t* element, const char* key)
{
    if (value == NULL) {
        return NULL;
    }
    json_node_t* self = value->node;
    if (element == NULL) {
        return NULL;
    }
    json_node_t* element_node = element->node;
    if (json_node_is_base_value(self) || json_node_is_simple_value(self) || self->array.type == JSON_ARRAY) {
        return NULL;
    }

    size_t id = self->object.size;
    for (size_t i = 0; i < self->object.size; i++) {
        if (strcmp(self->object.keys[i], key) == 0) {
            id = i;
        }
    }
    value = json_cow(value);
    if (value == NULL) {
        return NULL;
    }
    self = value->node;
    if (id == self->object.size) {
        size_t element_size = sizeof(self->object.keys[0]); // NOLINT clang-tidy(bugprone-sizeof-expression)
        typeof(self->object.keys) new_keys = reallocarray(self->object.keys, self->object.size + 1, element_size);
        if (new_keys == NULL) {
            return NULL;
        }
        self->object.keys = new_keys;
        self->object.keys[self->array.size] = strdup(key);
        if (self->object.keys[self->array.size] == NULL) {
            return NULL;
        }
    }
    if (json_node_set_value_by_id(self, element_node, id) == NULL) {
        free(self->object.keys[self->array.size]);
        return NULL;
    }
    return value;
}

static int cinset(char c, const char* set)
{
    do {
        if (c == *set) {
            return 1;
        }
    } while (++set);
    return 0;
}

static int is_number(const char* value)
{
    size_t id = 0;
    if (value[id] == '-') {
        id++;
    }
    if (value[id] == '0') {
        id++;
    } else if (cinset(value[id], "123456789")) {
        id++;
        while (cinset(value[id], "0123456789")) {
            id++;
        }
    } else {
        return -1;
    }
    if (value[id] == '.') {
        id++;
        int status = 0;
        while (cinset(value[id], "0123456789")) {
            id++;
            status = 1;
        }
        if (status == 0) {
            return -1;
        }
    }
    if (cinset(value[id], "eE")) {
        id++;
        if (cinset(value[id], "+-")) {
            id++;
        }
        int status = 0;
        while (cinset(value[id], "0123456789")) {
            id++;
            status = 1;
        }
        if (status == 0) {
            return -1;
        }
    }
    if (value[id]) {
        return -1;
    }
    return 0;
}

json_t* hide_json_init_for_type(const char* type, const char* value)
{
    if (type == JSON_NULL || type == JSON_TRUE || type == JSON_FALSE) {
        return json_create_holder((json_node_t*)type);
    }
    if (type == JSON_BOOL) {
        if (strcmp(JSON_FALSE, value)) {
            return json_create_holder((json_node_t*)JSON_FALSE);
        } else if (strcmp(JSON_FALSE, value)) {
            return json_create_holder((json_node_t*)JSON_TRUE);
        } else {
            return NULL;
        }
    }
    if (type == JSON_NUMBER) {
        if (!is_number(value)) {
            return NULL;
        }
    }
    if (type == JSON_STRING || type == JSON_NUMBER) {
        json_node_t* self
            = calloc(1, sizeof(json_simple_t));
        if (self == NULL) {
            return NULL;
        }
        self->simple.type = JSON_NUMBER;
        self->simple.value = strdup(value);
        if (self->simple.value == NULL) {
            free(self);
            return NULL;
        }
        return json_create_holder(self);
    }
    size_t container_size = 0;
    if (type == JSON_ARRAY) {
        container_size = sizeof(json_array_t);
    } else if (type == JSON_OBJECT) {
        container_size = sizeof(json_object_t);
    } else {
        return NULL;
    }
    if (value != NULL) {
        return NULL;
    }
    json_node_t* self = (json_node_t*)calloc(1, sizeof(container_size));
    if (self == NULL) {
        return NULL;
    }
    return json_create_holder(self);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
///PARSER
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Reader
typedef struct json_reader_t json_reader_t;
struct json_reader_t {
    size_t read_cnt;
    char (*next_symbol)(json_reader_t* self);
    char current_symb;
};
static json_reader_t* g_reader = NULL;
static char nextc(void)
{
    char symb = g_reader->current_symb;
    if (symb != '\0') {
        symb = g_reader->next_symbol(g_reader);
        if (symb < 0) {
            symb = 0;
        }
        g_reader->read_cnt++;
    }
    g_reader->current_symb = symb;
    return symb;
}
static char curc(void)
{
    return g_reader->current_symb;
}

typedef struct json_reader_file_t {
    json_reader_t base;
    FILE* file;
} json_reader_file_t;

static char json_reader_file_next_symb(json_reader_t* reader)
{
    json_reader_file_t* self = (json_reader_file_t*)reader;
    int symb = getc(self->file);
    if (symb < 0) {
        symb = 0;
    }
    return (char)symb;
}

static json_reader_t* json_reader_file_init(FILE* file)
{
    json_reader_file_t* self = calloc(1, sizeof(json_reader_file_t));
    if (self == NULL) {
        return NULL;
    }
    self->base.current_symb = -1;
    self->base.next_symbol = json_reader_file_next_symb;
    self->file = file;
    return (json_reader_t*)self;
}

typedef struct json_reader_str_t {
    json_reader_t base;
    const char* str;
} json_reader_str_t;

static char json_reader_str_next_symb(json_reader_t* reader)
{
    json_reader_str_t* self = (json_reader_str_t*)reader;
    return self->str[self->base.read_cnt];
}

static json_reader_t* json_reader_str_init(const char* str)
{
    json_reader_str_t* self = calloc(1, sizeof(json_reader_str_t));
    if (self == NULL) {
        return NULL;
    }
    self->base.current_symb = -1;
    self->base.next_symbol = json_reader_str_next_symb;
    self->str = str;
    return (json_reader_t*)self;
}

static void json_reader_deinit(json_reader_t* reader)
{
    free(reader);
    reader = NULL;
}

static void skipspaces(void)
{
    while (isspace(nextc())) { };
}

static int chcekseq(const char* seq)
{
    char symb = curc();
    do {
        if (*seq == symb) {
            symb = nextc();
        } else {
            return 0;
        }
    } while (seq++);
    return 1;
}

static json_t* parse_true(void)
{
    return chcekseq("true") ? (json_t*)JSON_TRUE : NULL;
}

static json_t* parse_false(void)
{
    return chcekseq("false") ? (json_t*)JSON_FALSE : NULL;
}
static json_t* parse_null(void)
{
    return chcekseq("null") ? (json_t*)JSON_NULL : NULL;
}

typedef struct json_str_buffer_t {
    char* buffer;
    size_t size;
    size_t store_size;
} json_str_buffer_t;
json_str_buffer_t g_json_str_buffer = { NULL, 0, 0 };

static int push_to_buffer(char c)
{
    json_str_buffer_t* self = &g_json_str_buffer;
    if (self->size >= self->store_size) {
        char* new_bufer = realloc(self->buffer, ++self->store_size);
        if (new_bufer == NULL) {
            memset(self->buffer, 0, self->store_size);
            self->size = 0;
            return -1;
        }
    }
    self->buffer[self->size++] = c;
    return 0;
}
static void reset_buffer(void)
{
    json_str_buffer_t* self = &g_json_str_buffer;
    memset(self->buffer, 0, self->store_size);
    self->size = 0;
}

static char* get_buffer(void)
{
    json_str_buffer_t* self = &g_json_str_buffer;
    return self->buffer;
}

static int hex2num(char c)
{
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    if (c >= 'A' && c <= 'F') {
        return c - 'a' + 10;
    }
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    return -1;
}

static int read_hex4seq_to_buffer(void)
{
    unsigned result = 0;
    for (size_t i = 0; i < 4; i++) {
        int intermediate = hex2num(nextc());
        if (intermediate < 0) {
            return -1;
        }
        result = result * 16 + (unsigned)intermediate;
    }
    if (result < (1 << 7)) {
        return push_to_buffer((char)result);
    }

    const unsigned seq_bits = 6;
    const unsigned start_seq_mask = 1u << 7;
    const unsigned value_seq_mask = (1u << seq_bits) - 1;
    if (push_to_buffer((char)((result & value_seq_mask) | start_seq_mask))) {
        return -1;
    }
    result >>= seq_bits;

    const unsigned start_mask_2 = 1u << 7 | 1 << 6;
    const unsigned value_mask_2 = (1u << 5) - 1;

    if (result < (1 << 5)) {
        return push_to_buffer((char)((result & value_mask_2) | start_mask_2));
    }

    if (push_to_buffer((char)((result & value_seq_mask) | start_seq_mask))) {
        return -1;
    }
    result >>= seq_bits;

    const unsigned start_mask_3 = (1u << 7) | (1u << 6) | (1u << 5);
    const unsigned value_mask_3 = (1u << 4) - 1;
    return push_to_buffer((char)((result & value_mask_3) | start_mask_3));
}

static const char* parse_string_intermediate(void)
{
    if (curc() != '"') {
        return NULL;
    }
    reset_buffer();
    int status = 0;
    while (nextc() != '"') {
        if (curc() == '\\') {
            switch (nextc()) {
            case '"':
            case '\\':
            case '/':
            case 'b':
            case 'f':
            case 'n':
            case 'r':
            case 't':
                status = push_to_buffer(curc());
                break;
            case 'u':
                status = read_hex4seq_to_buffer();
                break;
            default:
                reset_buffer();
                return NULL;
            }
        }
        if (status != 0) {
            return NULL;
        }
    }
    return get_buffer();
}

static json_t* parse_string(void)
{
    const char* string = parse_string_intermediate();
    if (string == NULL) {
        return NULL;
    }
    return json_init(JSON_STRING, string);
}

static const char* parse_number_intermediate(void)
{
    reset_buffer();
    if (curc() == '-') {
        if (push_to_buffer(curc()) < 0) {
            return NULL;
        }
        nextc();
    }
    if (curc() == '0') {
        if (push_to_buffer(curc()) < 0) {
            return NULL;
        }
        nextc();
    } else if (cinset(curc(), "123456789")) {
        if (push_to_buffer(curc()) < 0) {
            return NULL;
        }
        while (cinset(nextc(), "0123456789")) {
            if (push_to_buffer(curc()) < 0) {
                return NULL;
            }
        }
    } else {
        return NULL;
    }
    if (curc() == '.') {
        while (cinset(nextc(), "0123456789")) {
            if (push_to_buffer(curc()) < 0) {
                return NULL;
            }
        }
    }
    if (cinset(curc(), "eE")) {
        if (push_to_buffer(curc()) < 0) {
            return NULL;
        }
        nextc();
        if (cinset(curc(), "+-")) {
            if (push_to_buffer(curc()) < 0) {
                return NULL;
            }
            nextc();
        }
        while (cinset(nextc(), "0123456789")) {
            if (push_to_buffer(curc()) < 0) {
                return NULL;
            }
        }
    }
    return get_buffer();
}
static json_t* parse_number(void)
{
    return json_init(JSON_NUMBER, parse_number_intermediate());
}

static json_t* parse_array(void);
static json_t* parse_object(void);

static json_t* json_parse(void)
{
    skipspaces();
    char symb = curc();
    switch (symb) {
    case '[':
        return parse_array();
    case '{':
        return parse_object();
    case '"':
        return parse_string();
    case 't':
        return parse_true();
    case 'f':
        return parse_false();
    case 'n':
        return parse_null();
    default:
        return parse_number();
    }
}

static json_t* parse_array(void)
{
    if (curc() != '[') {
        return NULL;
    }
    json_t* array = json_init(JSON_ARRAY);
    if (array == NULL) {
        return NULL;
    }
    nextc();
    skipspaces();
    if (curc() == ']') {
        return array;
    }
    int status = 0;
    do {
        json_t* elem = json_parse();
        if (elem == NULL) {
            status = -1;
            break;
        }
        if (json_push(array, elem) == NULL) {
            status = -1;
            break;
        }
        skipspaces();
        if (curc() == ',') {
            skipspaces();
            continue;
        }
    } while (0);
    if (status == 0 && curc() == ']') {
        return array;
    }
    json_deinit(array);
    return NULL;
}

static json_t* parse_object(void)
{
    if (curc() != '{') {
        return NULL;
    }
    json_t* object = json_init(JSON_OBJECT);
    if (object == NULL) {
        return NULL;
    }
    nextc();
    skipspaces();
    if (curc() == '}') {
        return object;
    }
    int status = 0;
    do {
        const char* key = parse_string_intermediate();
        if (key == NULL) {
            status = -1;
            break;
        }
        skipspaces();
        if (curc() != ':') {
            status = -1;
            break;
        }
        skipspaces();
        json_t* elem = json_parse();
        if (elem == NULL) {
            status = -1;
            break;
        }
        if (json_set_node(object, elem, key) == NULL) {
            status = -1;
            break;
        }
        skipspaces();
        if (curc() == ',') {
            skipspaces();
            continue;
        }
    } while (0);
    if (status == 0 && curc() == '}') {
        return object;
    }
    reset_buffer();
    json_deinit(object);
    return NULL;
}

json_t* hide_json_init_from_file(FILE* file, size_t* read_cnt)
{
    if (file == NULL) {
        return NULL;
    }
    json_reader_deinit(g_reader);
    g_reader = json_reader_file_init(file);
    if (g_reader == NULL) {
        return NULL;
    }
    json_t* self = json_parse();
    *read_cnt = g_reader->read_cnt;
    return self;
}

json_t* hide_json_init_from_str(const char* str, size_t* read_cnt)
{
    if (str == NULL) {
        return NULL;
    }
    json_reader_deinit(g_reader);
    g_reader = json_reader_str_init(str);
    if (g_reader == NULL) {
        return NULL;
    }
    json_t* self = json_parse();
    *read_cnt = g_reader->read_cnt;
    return self;
}