/// Copyright © Alexander Kaluzhnyy
#include <cstddef>
#include <gmock/gmock-actions.h>
#include <gmock/gmock-cardinalities.h>
#include <gmock/gmock-nice-strict.h>
#include <gmock/gmock-spec-builders.h>
#include <gtest/gtest.h>
#include <ostream>
#include "json.h"
#include "json_printer.h"
#include "log.h"
#include "system_mock.hpp"
#include <functional>

namespace json_test {

using namespace ::testing;

class json_system_test : public Test {

protected:
    json_t* m_object = nullptr;

    ~json_system_test()
    {
        json_deinit(&m_object);
    }
};

#define JSON_STREQ(object, str) ({            \
    auto actual_str = json_sprint(object, 0); \
    ASSERT_STREQ(actual_str, str);            \
    free(actual_str);                         \
})

#define EXPECT_SET(iterations, function, params)                       \
    log_debug_msg("Check for %i successful calloc calls", iterations); \
    if (iterations) {                                                  \
        EXPECT_CALL(mock, function params)                             \
            .Times(iterations)                                         \
            .WillRepeatedly(DoDefault());                              \
    }                                                                  \
    EXPECT_CALL(mock, function params)                                 \
        .Times(AtMost(1))                                              \
        .WillRepeatedly(ReturnNull());

#define system_test_base(function, params, test_name, setup, test, ...) \
    TEST_F(json_system_test, test_name##_##function##_negative)         \
    {                                                                   \
        int ok = 0;                                                     \
        log_trace_func();                                               \
        {                                                               \
            NiceMock<system_mock> mock;                                 \
            InSequence s;                                               \
            setup(ok, ##__VA_ARGS__);                                   \
            test(ok, ##__VA_ARGS__);                                    \
            json_deinit(&m_object);                                     \
        }                                                               \
        ASSERT_NE(0, ok);                                               \
        ok = 0;                                                         \
        for (int iteration = 0; ok == 0; iteration++) {                 \
            NiceMock<system_mock> mock;                                 \
            InSequence s;                                               \
            setup(ok, ##__VA_ARGS__);                                   \
            EXPECT_SET(iteration, function, params);                    \
            test(ok, ##__VA_ARGS__);                                    \
            json_deinit(&m_object);                                     \
        }                                                               \
    }

#define NOTHING_FUNC(...)

#define system_test(test, ...)            \
    test(calloc, (_, _), ##__VA_ARGS__);  \
    test(strdup, (_), ##__VA_ARGS__);     \
    test(realloc, (_, _), ##__VA_ARGS__); \
    test(malloc, (_), ##__VA_ARGS__);

#define json_init_from_str_test(ok, string, ...) ({     \
    m_object = json_init_from_str(string, nullptr);     \
    if (m_object) {                                     \
        EXPECT_TRUE(mock.VerifyAndClearExpectations()); \
        const char* expected = string;                  \
        __VA_OPT__(expected = __VA_ARGS__;)             \
        JSON_STREQ(&m_object, expected);                \
        json_deinit(&m_object);                         \
        ok = 1;                                         \
    }                                                   \
})

#define system_test_json_init_from_str(function, params, string, ...) \
    system_test_base(function, params, json_init_from_str##_##string, NOTHING_FUNC, json_init_from_str_test, string, ##__VA_ARGS__);

static const char JSON_NULL_VALUE[] = "null";
system_test(system_test_json_init_from_str, JSON_NULL_VALUE);
static const char JSON_TRUE_VALUE[] = "true";
system_test(system_test_json_init_from_str, JSON_TRUE_VALUE);
static const char JSON_FALSE_VALUE[] = "false";
system_test(system_test_json_init_from_str, JSON_FALSE_VALUE);
const char JSON_NUMBER_VALUE[] = "-123.123E+123";
system_test(system_test_json_init_from_str, JSON_NUMBER_VALUE);

const char JSON_NUMBER_MINUS_ZERO[] = "-0";
system_test(system_test_json_init_from_str, JSON_NUMBER_MINUS_ZERO);

const char JSON_STR_QWERTY[] = "\"QWERTY\"";
system_test(system_test_json_init_from_str, JSON_STR_QWERTY);

static const char JSONS_STRING_U_3_BYTE_CHAR[] = R"JSON(" \u262D ")JSON";
static const char JSONS_STRING_U_3_BYTE_CHAR_EXPECTED[] = R"JSON(" ☭ ")JSON";
system_test(system_test_json_init_from_str, JSONS_STRING_U_3_BYTE_CHAR, JSONS_STRING_U_3_BYTE_CHAR_EXPECTED);

static const char JSONS_STRING_U_2_BYTE_CHAR[] = R"JSON(" \u0398 ")JSON";
static const char JSONS_STRING_U_2_BYTE_CHAR_EXPECTED[] = R"JSON(" Θ ")JSON";
system_test(system_test_json_init_from_str, JSONS_STRING_U_2_BYTE_CHAR, JSONS_STRING_U_2_BYTE_CHAR_EXPECTED);

const char JSON_ARRAY[] = "[[null],{\"false\":false},\"QWERTY\",12345,false,true,null]";
system_test(system_test_json_init_from_str, JSON_ARRAY);
const char JSON_OBJECT[] = "{\"1\":[null],\"2\":{\"false\":false},\"3\":\"QWERTY\",\"4\":12345,\"5\":false,\"6\":true,\"7\":null}";
system_test(system_test_json_init_from_str, JSON_OBJECT);

#define init_object_setup(ok, init_string, ...)          \
    m_object = json_init_from_str(init_string, nullptr); \
    ASSERT_NE(nullptr, m_object);

#define set_by_key_test(ok, unused1, key, expected)           \
    auto result = json_set_by_key(&m_object, &m_object, key); \
    if (result) {                                             \
        EXPECT_TRUE(mock.VerifyAndClearExpectations());       \
        JSON_STREQ(&m_object, expected);                      \
        json_deinit(&m_object);                               \
        ok = 1;                                               \
    }

#define system_test_function_for_someting(sys_function, sys_params, init_string, key) \
    system_test_base(sys_function, sys_params, json_set_by_key_##key, init_object_setup, set_by_key_test, init_string, key, EXPECTED_FOR_##key);

#define JSON_STRING(str) "\"" str "\""
#define JSON_OBJECT(str) "{" str "}"
#define OBJECT_PAIR(key, str) \
    JSON_STRING(key)          \
    ":" str

#define EXIST_KEY "exist_key"
#define NOT_EXIST_KEY "not_exist_key"
#define SOME_HEAVY_OBJECT JSON_OBJECT(OBJECT_PAIR(EXIST_KEY, "[null]"))
#define EXPECTED_FOR_EXIST_KEY JSON_OBJECT(OBJECT_PAIR(EXIST_KEY, SOME_HEAVY_OBJECT))
#define EXPECTED_FOR_NOT_EXIST_KEY JSON_OBJECT(OBJECT_PAIR(EXIST_KEY, "[null]") "," OBJECT_PAIR(NOT_EXIST_KEY, SOME_HEAVY_OBJECT))

system_test(system_test_function_for_someting, SOME_HEAVY_OBJECT, EXIST_KEY);
system_test(system_test_function_for_someting, SOME_HEAVY_OBJECT, NOT_EXIST_KEY);

}