/// Copyright © Alexander Kaluzhnyy

#ifndef JSON_H_INCLUDED
#define JSON_H_INCLUDED

#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct json_t json_t;
typedef char (*json_getc_t)(void*);

extern const char JSON_NULL[];
extern const char JSON_TRUE[];
extern const char JSON_FALSE[];
extern const char JSON_NUMBER[];
extern const char JSON_STRING[];
extern const char JSON_ARRAY[];
extern const char JSON_OBJECT[];

json_t* json_init_from_value(const char* type, const char* value);
json_t* json_init_from(json_getc_t getc, void* data);
json_t* json_init_from_file(FILE* file);
json_t* json_init_from_str(const char* value, const char** endptr);
json_t* json_copy(json_t* self);
void json_deinit(json_t* self);

const char* json_get_type(json_t* self);
const char* json_get_str(json_t* self);

size_t json_size(const json_t* self);
json_t* json_get_by_id(json_t* self, size_t id);
json_t* json_set_by_id(json_t* self, json_t* value, size_t id);

const char* json_key(const json_t* self, size_t id);
json_t* json_get_by_key(json_t* self, const char* key);
json_t* json_set_by_key(json_t* self, json_t* value, const char* key);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // JSON_H_INCLUDED