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
json_t* json_copy(json_t** self);
void json_deinit(json_t** self);

const char* json_get_type(json_t** self);
const char* json_get_str(json_t** self);

size_t json_size(json_t** self);
json_t** json_get_by_id(json_t** self, size_t id);

///
///@brief Set json value by id
///@param self pointer to containter to set
/// \n self value may be changed
///@param elem pointer to value tht will be set.
/// \n If elem **created by user** the ownership of the elem is **transferred** to self.
/// \n object moved to container and elem value changed to new place of store.
/// \n **elem invalidated** after **self removing**.
/// \n json_deinit() not have effect for elem
///@param id index of value in container.
/// \n May be less or equal(for array only) size of object
///@return Return self. In case of error return NULL.
/// \n available for JSON_ARRAY and JSON_OBJECT. Cause error foranither types
///
json_t** json_set_by_id(json_t** self, json_t** elem, size_t id);

const char* json_key(json_t** self, size_t id);
json_t** json_get_by_key(json_t** self, const char* key);
///
///@brief Set json value by key
///@param self pointer to containter to set
/// \n self value may be changed
///@param elem pointer to value tht will be set.
/// \n If elem **created by user** the ownership of the elem is **transferred** to self.
/// \n object moved to container and elem value changed to new place of store.
/// \n **elem invalidated** after **self removing**.
/// \n json_deinit() not have effect for elem
///@param key index of value in container.
/// \n If key not exist - new key created
/// \n New key insert in end of object
///@return Return self. In case of error return NULL.
/// \n available for JSON_OBJECT. Cause error for another types
///
json_t** json_set_by_key(json_t** self_ptr, json_t** value, const char* key);

json_t** json_set(json_t** dst, json_t** src); // not tested

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // JSON_H_INCLUDED