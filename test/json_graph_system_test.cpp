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
    EXPECT_STREQ(actual_str, str);            \
    free(actual_str);                         \
})

#define system_test_base(function, system_func_name, system_func_params, string, ...) \
    TEST_F(json_system_test, function##_##system_func_name##_##string##_negative)     \
    {                                                                                 \
        log_trace_func();                                                             \
        {                                                                             \
            auto check = function(string, ##__VA_ARGS__);                             \
            ASSERT_NE(check, nullptr);                                                \
            json_deinit(&check);                                                      \
        }                                                                             \
        for (int i = 0; true; i++) {                                                  \
            NiceMock<system_mock> m_mock;                                             \
            InSequence s;                                                             \
            log_debug_msg("Check for %i succesfull calloc calls", i);                 \
            if (i) {                                                                  \
                EXPECT_CALL(m_mock, system_func_name system_func_params)              \
                    .Times(i)                                                         \
                    .WillRepeatedly(DoDefault());                                     \
            }                                                                         \
            EXPECT_CALL(m_mock, system_func_name system_func_params)                  \
                .Times(AtMost(1))                                                     \
                .WillRepeatedly(ReturnNull());                                        \
            m_object = function(string, ##__VA_ARGS__);                               \
            if (m_object) {                                                           \
                log_debug_msg("Checking done");                                       \
                break;                                                                \
            }                                                                         \
        }                                                                             \
        JSON_STREQ(&m_object, string);                                                \
    }

#define system_test(function, string, ...)                              \
    system_test_base(function, malloc, (_), string, ##__VA_ARGS__);     \
    system_test_base(function, calloc, (_, _), string, ##__VA_ARGS__);  \
    system_test_base(function, realloc, (_, _), string, ##__VA_ARGS__); \
    system_test_base(function, strdup, (_), string, ##__VA_ARGS__);

#define system_test_init_from_str(string) system_test(json_init_from_str, string, nullptr)

const char JSON_NULL[] = "null";
system_test_init_from_str(JSON_NULL);
const char JSON_TRUE[] = "true";
system_test_init_from_str(JSON_TRUE);
const char JSON_FALSE[] = "false";
system_test_init_from_str(JSON_FALSE);
const char JSON_123[] = "123";
system_test_init_from_str(JSON_123);
const char JSON_STR_QWERTY[] = "\"QWERTY\"";
system_test_init_from_str(JSON_STR_QWERTY);
const char JSON_ARRAY[] = "[[null],{\"false\":false},\"QWERTY\",12345,false,true,null]";
system_test_init_from_str(JSON_ARRAY);
const char JSON_OBJECT[] = "{\"1\":[null],\"2\":{\"false\":false},\"3\":\"QWERTY\",\"4\":12345,\"5\":false,\"6\":true,\"7\":null}";
system_test_init_from_str(JSON_OBJECT);
}