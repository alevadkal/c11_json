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
static const char** WRONG_STRING_PPTR = reinterpret_cast<const char**>(WRONG_POINTER);
static size_t* WRONG_SIZE_PTR = reinterpret_cast<size_t*>(WRONG_POINTER);
static json_t* WRONG_JSON_PTR = reinterpret_cast<json_t*>(WRONG_POINTER);

class json_nullptr_test : public Test {
protected:
};

#define json_nullptr_test_1_impl(function, expect, ...)                    \
    TEST_F(json_nullptr_test, function##_nullptr_##__VA_ARGS__##_negative) \
    {                                                                      \
        EXPECT_EQ(expect, function(nullptr __VA_OPT__(, ) __VA_ARGS__));   \
    }

#define json_nullptr_test_2_make_test(function, value1, value2, expect, ...)             \
    TEST_F(json_nullptr_test, function##_##value1##_##value2##_##__VA_ARGS__##_negative) \
    {                                                                                    \
        EXPECT_EQ(expect, function(value1, value2 __VA_OPT__(, ) __VA_ARGS__));          \
    }

#define json_nullptr_test_2_impl(function, value1, value2, expect, ...)             \
    json_nullptr_test_2_make_test(function, nullptr, nullptr, expect, __VA_ARGS__); \
    json_nullptr_test_2_make_test(function, value1, nullptr, expect, __VA_ARGS__);  \
    json_nullptr_test_2_make_test(function, nullptr, value2, expect, __VA_ARGS__);

#define json_nullptr_test_3_make_test(function, value1, value2, value3, expect, ...)               \
    TEST_F(json_nullptr_test, function##_##value1##_##value2##_##value3##_##__VA_ARGS__##negative) \
    {                                                                                              \
        EXPECT_EQ(expect, function(value1, value2, value3 __VA_OPT__(, ) __VA_ARGS__));            \
    }

#define json_nullptr_test_3_impl(function, value1, value2, value3, expect, ...)              \
    json_nullptr_test_3_make_test(function, nullptr, nullptr, nullptr, expect, __VA_ARGS__); \
    json_nullptr_test_3_make_test(function, value1, nullptr, nullptr, expect, __VA_ARGS__);  \
    json_nullptr_test_3_make_test(function, nullptr, value2, nullptr, expect, __VA_ARGS__);  \
    json_nullptr_test_3_make_test(function, value1, value2, nullptr, expect, __VA_ARGS__);   \
    json_nullptr_test_3_make_test(function, nullptr, nullptr, value3, expect, __VA_ARGS__);  \
    json_nullptr_test_3_make_test(function, value1, nullptr, value3, expect, __VA_ARGS__);   \
    json_nullptr_test_3_make_test(function, nullptr, value2, value3, expect, __VA_ARGS__);

TEST_F(json_nullptr_test, json_deinit_self_nullptr_positive)
{
    json_deinit(nullptr);
}

json_nullptr_test_1_impl(json_init_from, nullptr, WRONG_POINTER);
json_nullptr_test_1_impl(json_init_from_value, nullptr, nullptr);
json_nullptr_test_1_impl(json_init_from_value, nullptr, WRONG_STRING_PTR);

json_nullptr_test_1_impl(json_init_from_str, nullptr, nullptr);

json_nullptr_test_1_impl(json_init_from_file, nullptr);

json_nullptr_test_1_impl(json_get_type, nullptr);

json_nullptr_test_1_impl(json_get_str, nullptr);

json_nullptr_test_1_impl(json_size, 0);

json_nullptr_test_1_impl(json_get_by_id, nullptr, 0);

json_nullptr_test_1_impl(json_key, nullptr, 0);

json_nullptr_test_2_impl(json_set_by_id, WRONG_JSON_PTR, WRONG_JSON_PTR, nullptr, 0);

json_nullptr_test_2_impl(json_get_by_key, WRONG_JSON_PTR, WRONG_STRING_PTR, nullptr);

json_nullptr_test_3_impl(json_set_by_key, WRONG_JSON_PTR, WRONG_JSON_PTR, WRONG_STRING_PTR, nullptr);
}