/// Copyright © Alexander Kaluzhnyy

#include <cstdio>
#include <gtest/gtest.h>
#include <gtest/internal/gtest-internal.h>

#include <cstdint>
#include "json.h"

namespace json_test {

using namespace ::testing;

static void* WRONG_POINTER = []() {
    struct {
        union {
            uint32_t value[sizeof(void*) / sizeof(uint16_t)];
            void* ptr;
        };
    } value;
    for (size_t i = 0; i < sizeof(value.value) / sizeof(value.value[0]); i++) {
        value.value[i] = 0xDEADBEAF;
    }
    return value.ptr;
}();

static const char* WRONG_STRING_PTR = reinterpret_cast<const char*>(WRONG_POINTER);
json_getc_t WRONG_GETC = reinterpret_cast<json_getc_t>(WRONG_POINTER);
static const char** WRONG_STRING_PPTR = reinterpret_cast<const char**>(WRONG_POINTER);
static json_t* NULL_JSON_PTR = nullptr;
static json_t** NULL_JSON_PPTR = &NULL_JSON_PTR;
static json_t* WRONG_JSON_PTR = reinterpret_cast<json_t*>(WRONG_POINTER);
static json_t** WRONG_JSON_PPTR = &WRONG_JSON_PTR;

class json_nullptr_test : public Test {
protected:
};

#define test_name_3(prefix, postfix, par1, par2, par3, ...) prefix##_p1_##par1##_p2_##par2##_p3_##par3##_##postfix
#define test_name_2(prefix, postfix, par1, par2, ...) prefix##_p1_##par1##_p2_##par2##_##postfix
#define test_name_1(prefix, postfix, par1, ...) prefix##_p1_##par1##_##postfix

#define test_name_(prefix, postfix, p1, p2, p3, fname, ...) fname(prefix, postfix, p1, p2, p3)

#define test_name(prefix, postfix, p1, ...) test_name_(prefix, postfix, p1, __VA_OPT__(__VA_ARGS__, ) test_name_3, test_name_2, test_name_1)

#define json_nullptr_test_impl(expected, function, par1, ...)                     \
    TEST_F(json_nullptr_test, test_name(function, negative, par1, ##__VA_ARGS__)) \
    {                                                                             \
        EXPECT_EQ(expected, function(par1, ##__VA_ARGS__));                       \
    }

#define json_nullptr_test_impl_json_1(expected, function, ...)          \
    json_nullptr_test_impl(expected, function, nullptr, ##__VA_ARGS__); \
    json_nullptr_test_impl(expected, function, NULL_JSON_PPTR, ##__VA_ARGS__);

#define json_nullptr_test_impl_json_2(expected, function, ...)                           \
    json_nullptr_test_impl_json_1(expected, function, WRONG_JSON_PPTR, ##__VA_ARGS__);   \
    json_nullptr_test_impl(expected, function, WRONG_JSON_PPTR, nullptr, ##__VA_ARGS__); \
    json_nullptr_test_impl(expected, function, WRONG_JSON_PPTR, NULL_JSON_PPTR, ##__VA_ARGS__);

// json_t* json_init_from_value(const char* type, const char* value);
json_nullptr_test_impl(nullptr, json_init_from_value, nullptr, nullptr);
json_nullptr_test_impl(nullptr, json_init_from_value, nullptr, WRONG_STRING_PTR);

// json_t* json_init_from(json_getc_t getc, void* data);
json_nullptr_test_impl(nullptr, json_init_from, nullptr, nullptr);
json_nullptr_test_impl(nullptr, json_init_from, nullptr, WRONG_POINTER);

// json_t* json_init_from_file(FILE* file);
json_nullptr_test_impl(nullptr, json_init_from_file, nullptr);

// json_t* json_init_from_str(const char* value, const char** endptr);
json_nullptr_test_impl(nullptr, json_init_from_str, nullptr, nullptr);

// json_t* json_copy(json_t** self);
json_nullptr_test_impl_json_1(nullptr, json_copy);

// const char* json_get_type(json_t** self);
json_nullptr_test_impl_json_1(nullptr, json_get_type);

// const char* json_get_str(json_t** self);
json_nullptr_test_impl_json_1(nullptr, json_get_str);

// size_t json_size(json_t** self);
json_nullptr_test_impl_json_1(0, json_size);

// json_t** json_set(json_t** dst, json_t** src);
json_nullptr_test_impl_json_2(nullptr, json_set);

// json_t** json_get_by_id(json_t** self, size_t id);
json_nullptr_test_impl_json_1(0, json_get_by_id, 0);

// json_t** json_set_by_id(json_t** self, json_t** value, size_t id);
json_nullptr_test_impl_json_2(nullptr, json_set_by_id, 0);

// const char* json_key(json_t** self, size_t id);
json_nullptr_test_impl_json_1(nullptr, json_key, 0);

// json_t** json_get_by_key(json_t** self, const char* key);
json_nullptr_test_impl_json_1(nullptr, json_get_by_key, nullptr);
json_nullptr_test_impl_json_1(nullptr, json_get_by_key, WRONG_STRING_PTR);
json_nullptr_test_impl(nullptr, json_get_by_key, WRONG_JSON_PPTR, nullptr);

// json_t** json_set_by_key(json_t** self_ptr, json_t** value, const char* key);
json_nullptr_test_impl_json_2(nullptr, json_set_by_key, nullptr);
json_nullptr_test_impl_json_2(nullptr, json_set_by_key, WRONG_STRING_PTR);
json_nullptr_test_impl(nullptr, json_set_by_key, WRONG_JSON_PPTR, WRONG_JSON_PPTR, nullptr);

// void json_deinit(json_t** self);
TEST_F(json_nullptr_test, json_deinit_p1_nullptr_negative)
{
    json_deinit(nullptr);
}
TEST_F(json_nullptr_test, json_deinit_p1_NULL_JSON_PPTR_negative)
{
    json_deinit(NULL_JSON_PPTR);
}

}