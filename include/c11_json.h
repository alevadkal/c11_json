/// Copyright © Alexander Kaluzhnyy

#ifndef JSON_H_INCLUDED
#define JSON_H_INCLUDED

#include <stdio.h>
#include <string.h>

typedef struct json_t json_t;

////////////////// INTERNAL API DON'T USE!!!//////////////////
char* hide_json_undefined_reference(void); // not implimented rise error on linker step in case of type error

json_t* hide_json_get_node_by_id(json_t* self, size_t id);
json_t* hide_json_get_node_by_key(json_t* self, const char* key);

json_t* hide_json_init(const char* value, const char* type);
json_t* hide_json_init_for_type(const char* type, const char* value);
json_t* hide_json_init_from_str(const char* value, size_t* read_cnt);
json_t* hide_json_init_from_file(FILE* file, size_t* read_cnt);
json_t* hide_json_set_node_by_id(json_t* self, json_t* value, size_t id);
json_t* hide_json_set_node_by_key(json_t* self, json_t* value,
    const char* key);

#define hide_json_init(first, second) _Generic((first),                                                            \
                                               const char*                                                         \
                                               : _Generic((second),                                                \
                                                          char*                                                    \
                                                          : hide_json_init_for_type((void*)first, (void*)second),  \
                                                          const char*                                              \
                                                          : hide_json_init_for_type((void*)first, (void*)second),  \
                                                          size_t*                                                  \
                                                          : hide_json_init_from_str((void*)first, (void*)second)), \
                                               FILE*                                                               \
                                               : _Generic((second),                                                \
                                                          char*                                                    \
                                                          : hide_json_undefined_reference(),                       \
                                                          const char*                                              \
                                                          : hide_json_undefined_reference(),                       \
                                                          size_t*                                                  \
                                                          : hide_json_init_from_file((void*)first, (void*)second)))

#define hide_json_get_node_by_id_define(self, key)   \
    _Generic((key),                                  \
             size_t                                  \
             : hide_json_get_node_by_id(self, key),  \
             const char*                             \
             : hide_json_get_node_by_key(self, key), \
             char*                                   \
             : hide_json_get_node_by_key(self, key))

////////////////////////////////////////////////

extern const char JSON_TRUE[];
extern const char JSON_FALSE[];

extern const char JSON_NULL[];
extern const char JSON_BOOL[];
extern const char JSON_NUMBER[];
extern const char JSON_STRING[];
extern const char JSON_ARRAY[];
extern const char JSON_OBJECT[];

///@fn const char* json_get_type(json_t* self);
///@brief Get type of json object
///@param self json object
///@return const char* one of next values:
///        - #JSON_NULL
///        - #JSON_BOOL
///        - #JSON_NUMBER
///        - #JSON_STRING
///        - #JSON_ARRAY
///        - #JSON_OBJECT
///        - in case of error return #NULL
///
const char* json_get_type(json_t* self);

///
///@fn size_t json_size(json_t* self)
///@brief Get size of object
/// Get size for json object with typ #JSON_ARRAY and #JSON_OBJECT
///@param self json object
///@return number of elements for object. In case of error return 0;
///
size_t json_size(json_t* self);

///
///@fn const char* json_key(json_t* self, size_t id);
///@brief Get key for object element
/// Get key for json object with type #JSON_OBJECT
///@param self [in] json object
///@param id [in] index
///@return key for json object with type #JSON_OBJECT. In case of error return NULL
///
const char* json_key(json_t* self, size_t id);

///
/// \fn json_t* json_init(const char* string, size_t* read_cnt=NULL)
/// \brief Init json object from string
/// \param [in] string String with json string
/// \param [out] read_cnt Number of characters parsed from string
/// \return json object. In case of error return NULL
///

///
/// \fn json_t* json_init(FILE* file, size_t* read_cnt=NULL)
/// \brief Init json object from string
/// \param [in] string File with json object
/// \param [out] read_cnt Number of characters parsed from string
/// \return json object. In case of error return NULL
///

///
/// \fn json_t* json_init(const char* type)
/// \brief Init default json object for type
/// \param [in] type Json object type
/// \return json object. In case of error return NULL
///

///
/// \fn json_t* json_init(const char* type, const char* value)
/// \brief Init default json object by value.
///        - Applicable for #JSON_NULL, #JSON_BOOL, #JSON_STRING, #JSON_NUMBER types
/// \param [in] type Json object type
/// \param [in] value Json object value
/// \return json object. In case of error return NULL
///

#define json_init(first, ...) ({                    \
    __VA_OPT__(if (0) { ) \
        hide_json_init(first, (size_t*)NULL); \
    __VA_OPT__( })                          \
    __VA_OPT__(hide_json_init(first, __VA_ARGS__);) \
})

///
/// \fn json_t* json_value(json_t* self)
/// \brief Get string
///        - Applicable for #JSON_STRING, #JSON_NUMBER, #JSON_BOOL, #JSON_NULL
///        - For #JSON_NULL return #JSON_NULL as value
///        - For #JSON_BOOL return #JSON_TRUE or JSON_FALSE
/// \param [in] self Json object
/// \return string reperesents of object. In case of error return NULL
const char* json_get_value(json_t* self);

///
/// \fn json_t* json_get_node(json_t* self, size_t id)
/// \brief Return json node from json object by index.
///        - Applicable for #JSON_ARRAY and #JSON_OBJECT types
/// \param [in] self Json object
/// \param [in] id index of json_node
/// \return json object. In case of error return NULL
///

///
/// \fn json_t* json_get_node(json_t* self, const char* key)
/// \brief Return json node from json object by key.
///        Applicable for #JSON_OBJECT type
/// \param [in] self Json object
/// \param [in] id index of json_node
/// \return json object. In case of error return NULL
///

#define json_get_node(self, key)                     \
    _Generic((key),                                  \
             size_t                                  \
             : hide_json_get_node_by_id(self, key),  \
             const char*                             \
             : hide_json_get_node_by_key(self, key), \
             char*                                   \
             : hide_json_get_node_by_key(self, key))

///
/// \fn json_t* json_set_node(json_t* self, json_t* value, size_t id)
/// \brief Set json node by id.
///        - Applicable for #JSON_ARRAY and #JSON_OBJECT types
///        - For JSON_ARRAY: if id equal size of array - value set as last element
/// \param [in] self Json object
/// \param [in] value Json object to set
/// \param [in] id index of json_node
/// \return self. In case of error return NULL
///

///
/// \fn json_t* json_set_node(json_t* self, json_t* value, const char* key)
/// \brief Set json node by kye.
///        - Applicable for #JSON_OBJECT type
///        - If key not exist create elemt in end of object with key
/// \param [in] self Json object
/// \param [in] value Json object to set
/// \param [in] key key of json_node
/// \return json object. In case of error return NULL
///

#define json_set_node(self, value, key)                            \
    _Generic((key),                                                \
             size_t                                                \
             : hide_json_set_node_by_id(self, value, (size_t)key), \
             const char*                                           \
             : hide_json_set_node_by_key(self, value, (void*)key), \
             char*                                                 \
             : hide_json_set_node_by_key(self, value, (void*)key))

///
///@brief Deinit json object
///@param [in] self
///
void json_deinit(json_t* self);

#endif // JSON_H_INCLUDED