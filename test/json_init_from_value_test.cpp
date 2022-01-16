/// Copyright © Alexander Kaluzhnyy

#include "json.h"
#include <cstdio>
#include <gtest/gtest.h>
#include <cstdint>
#include "log.h"

namespace json_test {

using namespace ::testing;

class json_init_tests : public Test {
protected:
    json_t* object = nullptr;
    void TearDown() override
    {
        log_trace_func();
        json_deinit(&object);
    }
};

class json_init_from_value_positive_tests : public json_init_tests {
protected:
};

#define json_init_from_value_positive_test_impl(type, value, expected)                                \
    TEST_F(json_init_from_value_positive_tests, json_init_from_value_1_##type##_2_##value##_positive) \
    {                                                                                                 \
        log_trace_func();                                                                             \
        object                                                                                        \
            = json_init_from_value(type, value);                                                      \
        ASSERT_NE(nullptr, object);                                                                   \
        ASSERT_STREQ(type, json_get_type(&object));                                                   \
        if (strcmp(type, JSON_ARRAY) || strcmp(type, JSON_OBJECT)) {                                  \
            EXPECT_STREQ(expected, json_get_str(&object));                                            \
        } else {                                                                                      \
            EXPECT_EQ(nullptr, json_get_str(&object));                                                \
        }                                                                                             \
    }

static char* WRONG_STRING_PTR = []() {
    struct {
        union {
            uint32_t value[sizeof(void*) / sizeof(uint16_t)];
            void* ptr;
        };
    } value;
    for (size_t i = 0; i < sizeof(value.value) / sizeof(value.value[0]); i++) {
        value.value[i] = 0xDEADBEAF;
    }
    return (char*)value.ptr;
}();

json_init_from_value_positive_test_impl(JSON_NULL, nullptr, JSON_NULL);
json_init_from_value_positive_test_impl(JSON_NULL, WRONG_STRING_PTR, JSON_NULL);
static const char JSON_STRING_NULL[] = "null";
json_init_from_value_positive_test_impl(JSON_STRING_NULL, WRONG_STRING_PTR, JSON_NULL);
json_init_from_value_positive_test_impl(JSON_FALSE, nullptr, JSON_FALSE);
json_init_from_value_positive_test_impl(JSON_FALSE, WRONG_STRING_PTR, JSON_FALSE);
static const char JSON_STRING_FALSE[] = "false";
json_init_from_value_positive_test_impl(JSON_STRING_FALSE, WRONG_STRING_PTR, JSON_FALSE);
json_init_from_value_positive_test_impl(JSON_TRUE, nullptr, JSON_TRUE);
json_init_from_value_positive_test_impl(JSON_TRUE, WRONG_STRING_PTR, JSON_TRUE);
static const char JSON_STRING_TRUE[] = "true";
json_init_from_value_positive_test_impl(JSON_STRING_TRUE, WRONG_STRING_PTR, JSON_TRUE);
static const char NUMBER_ZERO[] = "0";
json_init_from_value_positive_test_impl(JSON_NUMBER, nullptr, NUMBER_ZERO);
json_init_from_value_positive_test_impl(JSON_NUMBER, NUMBER_ZERO, NUMBER_ZERO);
static const char NUMBER_MINUS_ZERO[] = "-0";
json_init_from_value_positive_test_impl(JSON_NUMBER, NUMBER_MINUS_ZERO, NUMBER_MINUS_ZERO);
static const char NUMBER_DIGIT[] = "5";
json_init_from_value_positive_test_impl(JSON_NUMBER, NUMBER_DIGIT, NUMBER_DIGIT);
static const char NUMBER_MULTYDIGIT[] = "1234567";
json_init_from_value_positive_test_impl(JSON_NUMBER, NUMBER_MULTYDIGIT, NUMBER_MULTYDIGIT);
static const char NUMBER_MINUS_MULTYDIGIT[] = "-1234567";
json_init_from_value_positive_test_impl(JSON_NUMBER, NUMBER_MINUS_MULTYDIGIT, NUMBER_MINUS_MULTYDIGIT);
static const char NUMBER_MINUS_DIGIT[] = "-5";
json_init_from_value_positive_test_impl(JSON_NUMBER, NUMBER_MINUS_DIGIT, NUMBER_MINUS_DIGIT);
static const char NUMBER_ZERO_POINT_ZERO[] = "0.0";
json_init_from_value_positive_test_impl(JSON_NUMBER, NUMBER_ZERO_POINT_ZERO, NUMBER_ZERO_POINT_ZERO);
static const char NUMBER_MINUS_ZERO_POINT_ZERO[] = "-0.0";
json_init_from_value_positive_test_impl(JSON_NUMBER, NUMBER_MINUS_ZERO_POINT_ZERO, NUMBER_MINUS_ZERO_POINT_ZERO);
static const char NUMBER_ZERO_POINT_DIGIT[] = "0.0";
json_init_from_value_positive_test_impl(JSON_NUMBER, NUMBER_ZERO_POINT_DIGIT, NUMBER_ZERO_POINT_DIGIT);
static const char NUMBER_ZERO_POINT_MULTYDIGIT[] = "0.01234567890";
json_init_from_value_positive_test_impl(JSON_NUMBER, NUMBER_ZERO_POINT_MULTYDIGIT, NUMBER_ZERO_POINT_MULTYDIGIT);
static const char NUMBER_MULTYDIGIT_POINT_MULTYDIGIT[] = "123454654.01234567890";
json_init_from_value_positive_test_impl(JSON_NUMBER, NUMBER_MULTYDIGIT_POINT_MULTYDIGIT, NUMBER_MULTYDIGIT_POINT_MULTYDIGIT);
static const char NUMBER_ZERO_EXP_ZERO[] = "0E0";
json_init_from_value_positive_test_impl(JSON_NUMBER, NUMBER_ZERO_EXP_ZERO, NUMBER_ZERO_EXP_ZERO);
static const char NUMBER_ZERO_exp_ZERO[] = "0e0";
json_init_from_value_positive_test_impl(JSON_NUMBER, NUMBER_ZERO_exp_ZERO, NUMBER_ZERO_exp_ZERO);
static const char NUMBER_ZERO_EXP_PLUS_ZERO[] = "0E+0";
json_init_from_value_positive_test_impl(JSON_NUMBER, NUMBER_ZERO_EXP_PLUS_ZERO, NUMBER_ZERO_EXP_PLUS_ZERO);
static const char NUMBER_ZERO_exp_MINUS_ZERO[] = "0e-0";
json_init_from_value_positive_test_impl(JSON_NUMBER, NUMBER_ZERO_exp_MINUS_ZERO, NUMBER_ZERO_exp_MINUS_ZERO);
static const char NUMBER_ZERO_POINT_ZERO_EXP_PLUS_ZERO[] = "0.0E+0";
json_init_from_value_positive_test_impl(JSON_NUMBER, NUMBER_ZERO_POINT_ZERO_EXP_PLUS_ZERO, NUMBER_ZERO_POINT_ZERO_EXP_PLUS_ZERO);
static const char NUMBER_ZERO_POINT_ZERO_EXP_MINUS_ZERO[] = "0.0e-0";
json_init_from_value_positive_test_impl(JSON_NUMBER, NUMBER_ZERO_POINT_ZERO_EXP_MINUS_ZERO, NUMBER_ZERO_POINT_ZERO_EXP_MINUS_ZERO);
static const char NUMBER_ZERO_POINT_ZERO_EXP_ZERO[] = "0.0E0";
json_init_from_value_positive_test_impl(JSON_NUMBER, NUMBER_ZERO_POINT_ZERO_EXP_ZERO, NUMBER_ZERO_POINT_ZERO_EXP_ZERO);
static const char NUMBER_ZERO_POINT_ZERO_exp_ZERO[] = "0.0e0";
json_init_from_value_positive_test_impl(JSON_NUMBER, NUMBER_ZERO_POINT_ZERO_exp_ZERO, NUMBER_ZERO_POINT_ZERO_exp_ZERO);
static const char NUMBER_ZERO_POINT_MULTYZERO_exp_MULTYZERO[] = "0.000000e000000";
json_init_from_value_positive_test_impl(JSON_NUMBER, NUMBER_ZERO_POINT_MULTYZERO_exp_MULTYZERO, NUMBER_ZERO_POINT_MULTYZERO_exp_MULTYZERO);

class json_init_from_value_negative_tests : public json_init_tests {
protected:
    void TearDown() override
    {
        log_trace_func();
        json_init_tests::TearDown();
    }
};

#define json_init_from_value_negative_test_impl(type, value)                                          \
    TEST_F(json_init_from_value_negative_tests, json_init_from_value_1_##type##_2_##value##_negative) \
    {                                                                                                 \
        log_trace_func();                                                                             \
        object = json_init_from_value(type, value);                                                   \
        ASSERT_EQ(nullptr, object);                                                                   \
    }

static const char WRONG_NUMBER_ZERO_PLUS_EXP_ZERO[] = "0+E0";
json_init_from_value_negative_test_impl(JSON_NUMBER, WRONG_NUMBER_ZERO_PLUS_EXP_ZERO);
static const char WRONG_NUMBER_ZERO_MINUS_EXP_ZERO[] = "0-e0";
json_init_from_value_negative_test_impl(JSON_NUMBER, WRONG_NUMBER_ZERO_MINUS_EXP_ZERO);
static const char WRONG_NUMBER_MINUS_DOT_ZERO[] = "-.0";
json_init_from_value_negative_test_impl(JSON_NUMBER, WRONG_NUMBER_MINUS_DOT_ZERO);
static const char WRONG_NUMBER_ZERO_MULTYDIGIT[] = "0123234535";
json_init_from_value_negative_test_impl(JSON_NUMBER, WRONG_NUMBER_ZERO_MULTYDIGIT);
static const char WRONG_NUMBER_DOT_MULTYDIGIT[] = ".123234535";
json_init_from_value_negative_test_impl(JSON_NUMBER, WRONG_NUMBER_DOT_MULTYDIGIT);
static const char WRONG_NUMBER_ZERO_DOT_EXP[] = "-.0E";
json_init_from_value_negative_test_impl(JSON_NUMBER, WRONG_NUMBER_ZERO_DOT_EXP);
static const char WRONG_NUMBER_ZERO_DOT[] = "0.";
json_init_from_value_negative_test_impl(JSON_NUMBER, WRONG_NUMBER_ZERO_DOT);
static const char WRONG_NUMBER_ZERO_EXP[] = "0E";
json_init_from_value_negative_test_impl(JSON_NUMBER, WRONG_NUMBER_ZERO_EXP);
static const char WRONG_NUMBER_ZERO_EXP_PLUS[] = "0E+";
json_init_from_value_negative_test_impl(JSON_NUMBER, WRONG_NUMBER_ZERO_EXP_PLUS);
static const char WRONG_NUMBER_DOT_ZERO[] = ".0";
json_init_from_value_negative_test_impl(JSON_NUMBER, WRONG_NUMBER_DOT_ZERO);
static const char WRONG_NUMBER_MULTYDIGIT_SPACE[] = "0.01234567890 ";
json_init_from_value_negative_test_impl(JSON_NUMBER, WRONG_NUMBER_MULTYDIGIT_SPACE);
static const char WRONG_ZERO_POINT_MULTYDIGIT_SPACE[] = "0.01234567890 ";
json_init_from_value_negative_test_impl(JSON_NUMBER, WRONG_ZERO_POINT_MULTYDIGIT_SPACE);
static const char WRONG_NUMBER_SPACE_ZERO[] = " 0";
json_init_from_value_negative_test_impl(JSON_NUMBER, WRONG_NUMBER_SPACE_ZERO);

static const char EMPTY_STRING[] = "";
json_init_from_value_positive_test_impl(JSON_STRING, nullptr, EMPTY_STRING);
json_init_from_value_positive_test_impl(JSON_STRING, EMPTY_STRING, EMPTY_STRING);
static const char NOT_EMPTY_STRING[] = "qwerty";
json_init_from_value_positive_test_impl(JSON_STRING, NOT_EMPTY_STRING, NOT_EMPTY_STRING);
json_init_from_value_positive_test_impl(JSON_STRING, NUMBER_ZERO, NUMBER_ZERO);
json_init_from_value_positive_test_impl(JSON_STRING, WRONG_NUMBER_SPACE_ZERO, WRONG_NUMBER_SPACE_ZERO);

json_init_from_value_positive_test_impl(JSON_ARRAY, nullptr, nullptr);
json_init_from_value_positive_test_impl(JSON_ARRAY, WRONG_STRING_PTR, nullptr);
json_init_from_value_positive_test_impl(JSON_OBJECT, nullptr, nullptr);
json_init_from_value_positive_test_impl(JSON_OBJECT, WRONG_STRING_PTR, nullptr);

static const char JSON_WRONG_TYPE[] = "qwerty";
json_init_from_value_negative_test_impl(JSON_WRONG_TYPE, WRONG_STRING_PTR);
}
