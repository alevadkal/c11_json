/// Copyright © Alexander Kaluzhnyy

#include <cstdio>
#include <gtest/gtest.h>
#include <cstdint>
#include "json_printer.h"
#include "log.h"

namespace json_test {

using namespace ::testing;

class json_init_from_str_negative_tests : public Test {
protected:
};

#define NOT_JSON "qwerty"

#define json_init_from_str_negative_tests_base(str, endptr_value)                \
    TEST_F(json_init_from_str_negative_tests, json_init_from_1_##str##_negative) \
    {                                                                            \
        log_trace_func();                                                        \
        const char* endptr;                                                      \
        ASSERT_EQ(nullptr, json_init_from_str(str, &endptr));                    \
        EXPECT_STREQ(endptr, endptr_value) << str;                               \
        printf("%s", endptr_value);                                              \
    }
#define json_init_from_str_negative_tests(str, endptr_value)                            \
    json_init_from_str_negative_tests_base(str, endptr_value);                          \
    TEST_F(json_init_from_str_negative_tests, json_init_from_1_##str##_qwerty_negative) \
    {                                                                                   \
        log_trace_func();                                                               \
        const char* endptr;                                                             \
        std::string str_value = std::string { str } + NOT_JSON;                         \
        std::string endptr_expected = std::string { endptr_value } + NOT_JSON;          \
        ASSERT_EQ(nullptr, json_init_from_str(str_value.c_str(), &endptr));             \
        EXPECT_STREQ(endptr, endptr_expected.c_str());                                  \
    }

static const char JSONS_WRONG_EMPTY_STRING[] = "";
json_init_from_str_negative_tests(JSONS_WRONG_EMPTY_STRING, JSONS_WRONG_EMPTY_STRING);
static const char JSONS_WRONG_nul[] = "nul";
json_init_from_str_negative_tests(JSONS_WRONG_nul, "");
static const char JSONS_WRONG_nula[] = "nula";
json_init_from_str_negative_tests(JSONS_WRONG_nula, "a");
static const char JSONS_WRONG_tru[] = "tru";
json_init_from_str_negative_tests(JSONS_WRONG_tru, "");
static const char JSONS_WRONG_trus[] = "trus";
json_init_from_str_negative_tests(JSONS_WRONG_trus, "s");
static const char JSONS_WRONG_fals[] = "fals";
json_init_from_str_negative_tests(JSONS_WRONG_fals, "");
static const char JSONS_WRONG_falsa[] = "falsa";
json_init_from_str_negative_tests(JSONS_WRONG_falsa, "a");
static const char JSONS_WRONG_NUM_0_DOT[] = "0.";
json_init_from_str_negative_tests(JSONS_WRONG_NUM_0_DOT, "");
static const char JSONS_WRONG_NUM_123_DOT_E[] = "123.E";
json_init_from_str_negative_tests(JSONS_WRONG_NUM_123_DOT_E, "E");
static const char JSONS_WRONG_NUM_MINUS_0_DOT[] = "-0.";
json_init_from_str_negative_tests(JSONS_WRONG_NUM_MINUS_0_DOT, "");
static const char JSONS_WRONG_NUM_MINUS_DOT[] = "-.";
json_init_from_str_negative_tests(JSONS_WRONG_NUM_MINUS_DOT, ".");
static const char JSONS_WRONG_NUM_MINUS_1_DOT[] = "-1.";
json_init_from_str_negative_tests(JSONS_WRONG_NUM_MINUS_1_DOT, "");
static const char JSONS_WRONG_NUM_MINUS_123_DOT[] = "-123.";
json_init_from_str_negative_tests(JSONS_WRONG_NUM_MINUS_123_DOT, "");
static const char JSONS_WRONG_NUM_0_DOT_0_E[] = "0.0E";
json_init_from_str_negative_tests(JSONS_WRONG_NUM_0_DOT_0_E, "");
static const char JSONS_WRONG_NUM_0_DOT_0_e[] = "0.0e";
json_init_from_str_negative_tests(JSONS_WRONG_NUM_0_DOT_0_e, "");
static const char JSONS_WRONG_NUM_0_DOT_0_E_PLUS[] = "0.0E+";
json_init_from_str_negative_tests(JSONS_WRONG_NUM_0_DOT_0_E_PLUS, "");
static const char JSONS_WRONG_NUM_0_DOT_0_e_PLUS[] = "0.0e+";
json_init_from_str_negative_tests(JSONS_WRONG_NUM_0_DOT_0_e_PLUS, "");

static const char JSONS_WRONG_STR_NO_END[] = R"JSON(")JSON";
json_init_from_str_negative_tests_base(JSONS_WRONG_STR_NO_END, "");
static const char JSONS_WRONG_STR_WITH_SYMBOLS_NO_END[] = R"JSON("qwerty)JSON";
json_init_from_str_negative_tests_base(JSONS_WRONG_STR_WITH_SYMBOLS_NO_END, "");
static const char JSONS_WRONG_STR_NO_END_SLASHED_BRACE[] = R"JSON("\")JSON";
json_init_from_str_negative_tests_base(JSONS_WRONG_STR_NO_END_SLASHED_BRACE, "");
static const char JSONS_WRONG_STR_BAD_SLASHED_e[] = R"JSON("\e")JSON";
json_init_from_str_negative_tests(JSONS_WRONG_STR_BAD_SLASHED_e, "e\"");
static const char JSONS_WRONG_STR_UNENDED_U1[] = R"JSON("\u1)JSON";
json_init_from_str_negative_tests(JSONS_WRONG_STR_UNENDED_U1, "");
static const char JSONS_WRONG_STR_UNENDED_U2[] = R"JSON("\u12)JSON";
json_init_from_str_negative_tests(JSONS_WRONG_STR_UNENDED_U2, "");
static const char JSONS_WRONG_STR_UNENDED_U3[] = R"JSON("\u123)JSON";
json_init_from_str_negative_tests(JSONS_WRONG_STR_UNENDED_U3, "");

static const char JSON_WRONG_ARRAY_EMPTY_UNFINISHED[] = "[";
json_init_from_str_negative_tests(JSON_WRONG_ARRAY_EMPTY_UNFINISHED, "");
static const char JSON_WRONG_ARRAY_WITH_NULL_UNFINISHED[] = "[null";
json_init_from_str_negative_tests(JSON_WRONG_ARRAY_WITH_NULL_UNFINISHED, "");
static const char JSON_WRONG_ARRAY_WITH_TRUE_UNFINISHED[] = "[true";
json_init_from_str_negative_tests(JSON_WRONG_ARRAY_WITH_TRUE_UNFINISHED, "");
static const char JSON_WRONG_ARRAY_WITH_FALSE_UNFINISHED[] = "[false";
json_init_from_str_negative_tests(JSON_WRONG_ARRAY_WITH_FALSE_UNFINISHED, "");
static const char JSON_WRONG_ARRAY_WITH_NUMBER_UNFINISHED[] = "[12345";
json_init_from_str_negative_tests(JSON_WRONG_ARRAY_WITH_NUMBER_UNFINISHED, "");
static const char JSON_WRONG_ARRAY_WITH_STRING_UNFINISHED[] = "[\"12345\"";
json_init_from_str_negative_tests(JSON_WRONG_ARRAY_WITH_STRING_UNFINISHED, "");
static const char JSON_WRONG_ARRAY_WITH_UNFINISHED_STRING[] = "[\"12345]";
json_init_from_str_negative_tests_base(JSON_WRONG_ARRAY_WITH_UNFINISHED_STRING, "");
static const char JSON_WRONG_ARRAY_WITH_ARRAY_UNFINISHED[] = "[[]";
json_init_from_str_negative_tests(JSON_WRONG_ARRAY_WITH_ARRAY_UNFINISHED, "");
static const char JSON_WRONG_ARRAY_WITH_OBJECT_UNFINISHED[] = "[{}";
json_init_from_str_negative_tests(JSON_WRONG_ARRAY_WITH_OBJECT_UNFINISHED, "");
static const char JSON_WRONG_ARRAY_WITH_MULTIELEMENTS_UNFINISHED[] = "[{},[],\"qwerty\",12345,true,false,null";
json_init_from_str_negative_tests(JSON_WRONG_ARRAY_WITH_MULTIELEMENTS_UNFINISHED, "");
static const char JSON_WRONG_ARRAY_EMPTY_WITH_COMMA[] = "[,]";
json_init_from_str_negative_tests(JSON_WRONG_ARRAY_EMPTY_WITH_COMMA, ",]");
static const char JSON_WRONG_ARRAY_WITH_NULL_AND_COMMA[] = "[null,]";
json_init_from_str_negative_tests(JSON_WRONG_ARRAY_WITH_NULL_AND_COMMA, "]");
static const char JSON_WRONG_ARRAY_WITH_TRUE_AND_COMMA[] = "[true,]";
json_init_from_str_negative_tests(JSON_WRONG_ARRAY_WITH_TRUE_AND_COMMA, "]");
static const char JSON_WRONG_ARRAY_WITH_FALSE_AND_COMMA[] = "[false,]";
json_init_from_str_negative_tests(JSON_WRONG_ARRAY_WITH_FALSE_AND_COMMA, "]");
static const char JSON_WRONG_ARRAY_WITH_NUMBER_AND_COMMA[] = "[12345,]";
json_init_from_str_negative_tests(JSON_WRONG_ARRAY_WITH_NUMBER_AND_COMMA, "]");
static const char JSON_WRONG_ARRAY_WITH_STRING_AND_COMMA[] = "[\"12345\",]";
json_init_from_str_negative_tests(JSON_WRONG_ARRAY_WITH_STRING_AND_COMMA, "]");
static const char JSON_WRONG_ARRAY_WITH_ARRAY_AND_COMMA[] = "[[],]";
json_init_from_str_negative_tests(JSON_WRONG_ARRAY_WITH_ARRAY_AND_COMMA, "]");
static const char JSON_WRONG_ARRAY_WITH_OBJECT_AND_COMMA[] = "[{},]";
json_init_from_str_negative_tests(JSON_WRONG_ARRAY_WITH_OBJECT_AND_COMMA, "]");
static const char JSON_WRONG_ARRAY_WITH_MULTIOBJECT_AND_COMMA[] = "[{},[],\"qwerty\",12345,true,false,null,]";
json_init_from_str_negative_tests(JSON_WRONG_ARRAY_WITH_MULTIOBJECT_AND_COMMA, "]");

static const char JSON_WRONG_OBJECT_EMPTY_UNFINISHED[] = "{";
json_init_from_str_negative_tests(JSON_WRONG_OBJECT_EMPTY_UNFINISHED, "");
static const char JSON_WRONG_OBJECT_EMPTY_UNFINISHED_KEY[] = "{\"";
json_init_from_str_negative_tests_base(JSON_WRONG_OBJECT_EMPTY_UNFINISHED_KEY, "");
static const char JSON_WRONG_OBJECT_ONLY_KEY[] = "{\"key\"";
json_init_from_str_negative_tests(JSON_WRONG_OBJECT_ONLY_KEY, "");
static const char JSON_WRONG_OBJECT_KEY_AND_DDOT[] = "{\"key\":";
json_init_from_str_negative_tests(JSON_WRONG_OBJECT_KEY_AND_DDOT, "");
static const char JSON_WRONG_OBJECT_KEY_AND_END[] = "{\"key\":}";
json_init_from_str_negative_tests(JSON_WRONG_OBJECT_KEY_AND_END, "}");
static const char JSON_WRONG_OBJECT_UNFINISHED_KEY_AND_NULL[] = "{\"key\":null";
json_init_from_str_negative_tests(JSON_WRONG_OBJECT_UNFINISHED_KEY_AND_NULL, "");
static const char JSON_WRONG_OBJECT_UNFINISHED_KEY_AND_TRUE[] = "{\"key\":true";
json_init_from_str_negative_tests(JSON_WRONG_OBJECT_UNFINISHED_KEY_AND_TRUE, "");
static const char JSON_WRONG_OBJECT_UNFINISHED_KEY_AND_FALSE[] = "{\"key\":false";
json_init_from_str_negative_tests(JSON_WRONG_OBJECT_UNFINISHED_KEY_AND_FALSE, "");
static const char JSON_WRONG_OBJECT_UNFINISHED_KEY_AND_NUM[] = "{\"key\":12345";
json_init_from_str_negative_tests(JSON_WRONG_OBJECT_UNFINISHED_KEY_AND_NUM, "");
static const char JSON_WRONG_OBJECT_UNFINISHED_KEY_AND_STRING[] = "{\"key\":\"string\"";
json_init_from_str_negative_tests(JSON_WRONG_OBJECT_UNFINISHED_KEY_AND_STRING, "");
static const char JSON_WRONG_OBJECT_UNFINISHED_KEY_AND_ARRAY[] = "{\"key\":[]";
json_init_from_str_negative_tests(JSON_WRONG_OBJECT_UNFINISHED_KEY_AND_ARRAY, "");
static const char JSON_WRONG_OBJECT_UNFINISHED_KEY_AND_OBJECT[] = "{\"key\":{}";
json_init_from_str_negative_tests(JSON_WRONG_OBJECT_UNFINISHED_KEY_AND_OBJECT, "");
static const char JSON_WRONG_OBJECT_UNFINISHED_MULTIOBJECT[] = R"JSON({"o":{},"a":[],"s":"qwerty","v":12345,"t":true,"f":false,"n":null)JSON";
json_init_from_str_negative_tests(JSON_WRONG_OBJECT_UNFINISHED_MULTIOBJECT, "");

static const char JSON_WRONG_OBJECT_ONLY_KEY_AND_COMMA[] = "{\"key\",}";
json_init_from_str_negative_tests(JSON_WRONG_OBJECT_ONLY_KEY_AND_COMMA, ",}");
static const char JSON_WRONG_OBJECT_KEY_AND_DDOT_AND_COMMA[] = "{\"key\":,}";
json_init_from_str_negative_tests(JSON_WRONG_OBJECT_KEY_AND_DDOT_AND_COMMA, ",}");
static const char JSON_WRONG_OBJECT_KEY_AND_NULL_AND_COMMA[] = "{\"key\":null,}";
json_init_from_str_negative_tests(JSON_WRONG_OBJECT_KEY_AND_NULL_AND_COMMA, "}");
static const char JSON_WRONG_OBJECT_KEY_AND_TRUE_AND_COMMA[] = "{\"key\":true,}";
json_init_from_str_negative_tests(JSON_WRONG_OBJECT_KEY_AND_TRUE_AND_COMMA, "}");
static const char JSON_WRONG_OBJECT_KEY_AND_FALSE_AND_COMMA[] = "{\"key\":false,}";
json_init_from_str_negative_tests(JSON_WRONG_OBJECT_KEY_AND_FALSE_AND_COMMA, "}");
static const char JSON_WRONG_OBJECT_KEY_AND_NUM_AND_COMMA[] = "{\"key\":12345,}";
json_init_from_str_negative_tests(JSON_WRONG_OBJECT_KEY_AND_NUM_AND_COMMA, "}");
static const char JSON_WRONG_OBJECT_KEY_AND_STRING_AND_COMMA[] = "{\"key\":\"string\",}";
json_init_from_str_negative_tests(JSON_WRONG_OBJECT_KEY_AND_STRING_AND_COMMA, "}");
static const char JSON_WRONG_OBJECT_KEY_AND_ARRAY_AND_COMMA[] = "{\"key\":[],}";
json_init_from_str_negative_tests(JSON_WRONG_OBJECT_KEY_AND_ARRAY_AND_COMMA, "}");
static const char JSON_WRONG_OBJECT_KEY_AND_OBJECT_AND_COMMA[] = "{\"key\":{},}";
json_init_from_str_negative_tests(JSON_WRONG_OBJECT_KEY_AND_OBJECT_AND_COMMA, "}");
static const char JSON_WRONG_OBJECT_MULTIOBJECT_AND_COMMA[] = R"JSON({"o":{},"a":[],"s":"qwerty","v":12345,"t":true,"f":false,"n":null,})JSON";
json_init_from_str_negative_tests(JSON_WRONG_OBJECT_MULTIOBJECT_AND_COMMA, "}");
}
