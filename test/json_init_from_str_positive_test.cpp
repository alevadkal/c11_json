/// Copyright © Alexander Kaluzhnyy

#include <cstdio>
#include <gtest/gtest.h>
#include <cstdint>
#include "json.h"
#include "json_printer.h"
#include "log.h"

namespace json_test {

using namespace ::testing;

class json_init_from_str_positive_base_tests : public Test {

protected:
    json_t* m_object = nullptr;
    char* m_out = nullptr;
    const char* m_out_expected = nullptr;
    json_t* m_object_second = nullptr;
    char* m_out_second = nullptr;

    void TearDown() override
    {
        ASSERT_NE(nullptr, m_object);
        ASSERT_NE(nullptr, m_out);
        ASSERT_NE(nullptr, m_object_second);
        ASSERT_NE(nullptr, m_out_second);
        EXPECT_STREQ(m_out_expected, m_out);
        EXPECT_STREQ(m_out_expected, m_out_second);

        free(m_out_second);
        json_deinit(&m_object_second);
        free(m_out);
        json_deinit(&m_object);
    }
};

class json_init_from_str_positive_tests : public json_init_from_str_positive_base_tests {

protected:
    const char* m_endptr = NULL;
    const char* m_endptr_expected = NULL;
    const char* m_endptr_second = nullptr;

    void TearDown() override
    {
        ASSERT_NE(nullptr, m_endptr);
        ASSERT_NE(nullptr, m_endptr_second);
        EXPECT_STREQ(m_endptr_expected, m_endptr);
        EXPECT_STREQ(m_endptr_expected, m_endptr);
        json_init_from_str_positive_base_tests::TearDown();
    }
};

#define json_init_from_str_positive_tests_base(str)                                   \
    TEST_F(json_init_from_str_positive_base_tests, json_init_from_1_##str##_positive) \
    {                                                                                 \
        log_trace_func();                                                             \
        m_out_expected = str;                                                         \
        m_object = json_init_from_str(str, nullptr);                                  \
        m_out = json_sprint(&m_object, 0);                                            \
        m_object_second = json_init_from_str(m_out, nullptr);                         \
        m_out_second = json_sprint(&m_object_second, 0);                              \
    }

#define json_init_from_str_positive_tests_impl_2(str, expected)                  \
    TEST_F(json_init_from_str_positive_tests, json_init_from_1_##str##_positive) \
    {                                                                            \
        log_trace_func();                                                        \
        m_out_expected = expected;                                               \
        m_endptr_expected = "";                                                  \
        m_object = json_init_from_str(str, &m_endptr);                           \
        m_out = json_sprint(&m_object, 0);                                       \
        m_object_second = json_init_from_str(m_out, &m_endptr_second);           \
        m_out_second = json_sprint(&m_object_second, 0);                         \
    }

#define json_init_from_str_positive_tests_impl_1(str) json_init_from_str_positive_tests_impl_2(str, str)
#define json_init_from_str_positive_tests_impl_1_expected(strname) json_init_from_str_positive_tests_impl_2(strname, strname##_EXPECTED)

static const char JSONS_NULL[] = "null";
json_init_from_str_positive_tests_impl_1(JSONS_NULL);
json_init_from_str_positive_tests_base(JSONS_NULL);
static const char JSONS_TRUE[] = "true";
json_init_from_str_positive_tests_impl_1(JSONS_TRUE);
static const char JSONS_FALSE[] = "false";
json_init_from_str_positive_tests_impl_1(JSONS_FALSE);
static const char JSONS_NUM_0[] = "0";
json_init_from_str_positive_tests_impl_1(JSONS_NUM_0);
static const char JSONS_NUM_minus_0[] = "-0";
json_init_from_str_positive_tests_impl_1(JSONS_NUM_minus_0);
static const char JSONS_NUM_0_0[] = "0.0";
json_init_from_str_positive_tests_impl_1(JSONS_NUM_0_0);
static const char JSONS_NUM_0_000[] = "0.000";
json_init_from_str_positive_tests_impl_1(JSONS_NUM_0_000);
static const char JSONS_NUM_0_E_0[] = "0E0";
json_init_from_str_positive_tests_impl_1(JSONS_NUM_0_E_0);
static const char JSONS_NUM_0_E_munus_0[] = "0E-0";
json_init_from_str_positive_tests_impl_1(JSONS_NUM_0_E_munus_0);
static const char JSONS_NUM_0_E_plus_0[] = "0E+0";
json_init_from_str_positive_tests_impl_1(JSONS_NUM_0_E_plus_0);
static const char JSONS_NUM_0_e_0[] = "0e0";
json_init_from_str_positive_tests_impl_1(JSONS_NUM_0_e_0);
static const char JSONS_NUM_0_e_munus_0[] = "0e-0";
json_init_from_str_positive_tests_impl_1(JSONS_NUM_0_e_munus_0);
static const char JSONS_NUM_0_e_plus_0[] = "0e+0";
json_init_from_str_positive_tests_impl_1(JSONS_NUM_0_e_plus_0);
static const char JSONS_NUM_0_e_plus_000[] = "0e+000";
json_init_from_str_positive_tests_impl_1(JSONS_NUM_0_e_plus_000);
static const char JSONS_NUM_DIGIT[] = "1";
json_init_from_str_positive_tests_impl_1(JSONS_NUM_DIGIT);
static const char JSONS_NUM_NDIGIT[] = "1234567890";
json_init_from_str_positive_tests_impl_1(JSONS_NUM_NDIGIT);
static const char JSONS_NUM_NDIGIT_0[] = "1234567890.0";
json_init_from_str_positive_tests_impl_1(JSONS_NUM_NDIGIT_0);
static const char JSONS_NUM_NDIGIT_000[] = "1234567890.000";
json_init_from_str_positive_tests_impl_1(JSONS_NUM_NDIGIT_000);
static const char JSONS_NUM_NDIGIT_DIGIT[] = "1234567890.9";
json_init_from_str_positive_tests_impl_1(JSONS_NUM_NDIGIT_DIGIT);
static const char JSONS_NUM_NDIGIT_NDIGIT[] = "1234567890.123456789";
json_init_from_str_positive_tests_impl_1(JSONS_NUM_NDIGIT_NDIGIT);
static const char JSONS_NUM_minus_NDIGIT_NDIGIT[] = "-1234567890.123456789";
json_init_from_str_positive_tests_impl_1(JSONS_NUM_minus_NDIGIT_NDIGIT);
static const char JSONS_NUM_DIGIT_E_0[] = "1E0";
json_init_from_str_positive_tests_impl_1(JSONS_NUM_DIGIT_E_0);
static const char JSONS_NUM_NDIGIT_E_plus_DIGIT[] = "1234567890E+1";
json_init_from_str_positive_tests_impl_1(JSONS_NUM_NDIGIT_E_plus_DIGIT);
static const char JSONS_NUM_NDIGIT_0_E_minus_DIGIT[] = "1234567890.0E-1";
json_init_from_str_positive_tests_impl_1(JSONS_NUM_NDIGIT_0_E_minus_DIGIT);
static const char JSONS_NUM_NDIGIT_000_e_000_DIGIT[] = "1234567890.000e0001";
json_init_from_str_positive_tests_impl_1(JSONS_NUM_NDIGIT_000_e_000_DIGIT);
static const char JSONS_NUM_NDIGIT_DIGIT_e_plus_000_DIGIT[] = "1234567890.7e+0001";
json_init_from_str_positive_tests_impl_1(JSONS_NUM_NDIGIT_DIGIT_e_plus_000_DIGIT);
static const char JSONS_NUM_NDIGIT_DIGIT_e_minus_000_DIGIT[] = "1234567890.1234567890e-1234567890";
json_init_from_str_positive_tests_impl_1(JSONS_NUM_NDIGIT_DIGIT_e_minus_000_DIGIT);
static const char JSONS_NUM_minus_NDIGIT_DIGIT_e_minus_000_DIGIT[] = "-1234567890.1234567890e-1234567890";
json_init_from_str_positive_tests_impl_1(JSONS_NUM_minus_NDIGIT_DIGIT_e_minus_000_DIGIT);
static const char JSONS_EMPTY_STRING[] = R"JSON("")JSON";
json_init_from_str_positive_tests_impl_1(JSONS_EMPTY_STRING);
static const char JSONS_STRING_QUOT_MARK[] = R"JSON(" \" ")JSON";
json_init_from_str_positive_tests_impl_1(JSONS_STRING_QUOT_MARK);
static const char JSONS_STRING_BACK_SLASH[] = R"JSON(" \\ ")JSON";
json_init_from_str_positive_tests_impl_1(JSONS_STRING_BACK_SLASH);
static const char JSONS_STRING_SLASH[] = R"JSON(" \/ ")JSON";
static const char JSONS_STRING_SLASH_EXPECTED[] = R"JSON(" / ")JSON";
json_init_from_str_positive_tests_impl_1_expected(JSONS_STRING_SLASH);
static const char JSONS_STRING_BACKSPACE[] = R"JSON(" \b ")JSON";
static const char JSONS_STRING_BACKSPACE_EXPECTED[] = "\" \b \"";
json_init_from_str_positive_tests_impl_1_expected(JSONS_STRING_BACKSPACE);
static const char JSONS_STRING_FORMFEED[] = R"JSON(" \f ")JSON";
static const char JSONS_STRING_FORMFEED_EXPECTED[] = "\" \f \"";
json_init_from_str_positive_tests_impl_1_expected(JSONS_STRING_FORMFEED);
static const char JSONS_STRING_LINEFEED[] = R"JSON(" \n ")JSON";
static const char JSONS_STRING_LINEFEED_EXPECTED[] = "\" \n \"";
json_init_from_str_positive_tests_impl_1_expected(JSONS_STRING_LINEFEED);
static const char JSONS_STRING_CARRIEGE_RETURN[] = R"JSON(" \r ")JSON";
static const char JSONS_STRING_CARRIEGE_RETURN_EXPECTED[] = "\" \r \"";
json_init_from_str_positive_tests_impl_1_expected(JSONS_STRING_CARRIEGE_RETURN);
static const char JSONS_STRING_HORISONTAL_TAB[] = R"JSON(" \t ")JSON";
static const char JSONS_STRING_HORISONTAL_TAB_EXPECTED[] = "\" \t \"";
json_init_from_str_positive_tests_impl_1_expected(JSONS_STRING_HORISONTAL_TAB);
static const char JSONS_STRING_U_0000_CHAR[] = R"JSON(" \u0000 ")JSON";
static const char JSONS_STRING_U_0000_CHAR_EXPECTED[] = R"JSON(" ")JSON";
json_init_from_str_positive_tests_impl_1_expected(JSONS_STRING_U_0000_CHAR);
static const char JSONS_STRING_U_1_BYTE_CHAR[] = R"JSON(" \u007E ")JSON";
static const char JSONS_STRING_U_1_BYTE_CHAR_EXPECTED[] = R"JSON(" ~ ")JSON";
json_init_from_str_positive_tests_impl_1_expected(JSONS_STRING_U_1_BYTE_CHAR);
static const char JSONS_STRING_U_2_BYTE_CHAR[] = R"JSON(" \u0398 ")JSON";
static const char JSONS_STRING_U_2_BYTE_CHAR_EXPECTED[] = R"JSON(" Θ ")JSON";
json_init_from_str_positive_tests_impl_1_expected(JSONS_STRING_U_2_BYTE_CHAR);
static const char JSONS_STRING_U_3_BYTE_CHAR[] = R"JSON(" \u262D ")JSON";
static const char JSONS_STRING_U_3_BYTE_CHAR_EXPECTED[] = R"JSON(" ☭ ")JSON";
json_init_from_str_positive_tests_impl_1_expected(JSONS_STRING_U_3_BYTE_CHAR);
static const char JSONS_STRING_U_0123_CHAR[] = R"JSON(" \u0123 ")JSON";
static const char JSONS_STRING_U_0123_CHAR_EXPECTED[] = R"JSON(" ģ ")JSON";
json_init_from_str_positive_tests_impl_1_expected(JSONS_STRING_U_0123_CHAR);
static const char JSONS_STRING_U_4567_CHAR[] = R"JSON(" \u4567 ")JSON";
static const char JSONS_STRING_U_4567_CHAR_EXPECTED[] = R"JSON(" 䕧 ")JSON";
json_init_from_str_positive_tests_impl_1_expected(JSONS_STRING_U_4567_CHAR);
static const char JSONS_STRING_U_89AB_CHAR[] = R"JSON(" \u89AB ")JSON";
static const char JSONS_STRING_U_89AB_CHAR_EXPECTED[] = R"JSON(" 覫 ")JSON";
json_init_from_str_positive_tests_impl_1_expected(JSONS_STRING_U_89AB_CHAR);
static const char JSONS_STRING_U_CDEF_CHAR[] = R"JSON(" \uCDEF ")JSON";
static const char JSONS_STRING_U_CDEF_CHAR_EXPECTED[] = R"JSON(" 췯 ")JSON";
json_init_from_str_positive_tests_impl_1_expected(JSONS_STRING_U_CDEF_CHAR);
static const char JSONS_STRING_U_89ab_CHAR[] = R"JSON(" \u89ab ")JSON";
static const char JSONS_STRING_U_89ab_CHAR_EXPECTED[] = R"JSON(" 覫 ")JSON";
json_init_from_str_positive_tests_impl_1_expected(JSONS_STRING_U_89ab_CHAR);
static const char JSONS_STRING_U_cdef_CHAR[] = R"JSON(" \ucdef ")JSON";
static const char JSONS_STRING_U_cdef_CHAR_EXPECTED[] = R"JSON(" 췯 ")JSON";
json_init_from_str_positive_tests_impl_1_expected(JSONS_STRING_U_cdef_CHAR);

static const char JSONS_EMPTY_ARRAY[] = "[]";
json_init_from_str_positive_tests_impl_1(JSONS_EMPTY_ARRAY);
static const char JSONS_ARRAY_WITH_NULL[] = "[null]";
json_init_from_str_positive_tests_impl_1(JSONS_ARRAY_WITH_NULL);
static const char JSONS_ARRAY_WITH_FALSE[] = "[false]";
json_init_from_str_positive_tests_impl_1(JSONS_ARRAY_WITH_FALSE);
static const char JSONS_ARRAY_WITH_TRUE[] = "[true]";
json_init_from_str_positive_tests_impl_1(JSONS_ARRAY_WITH_TRUE);
static const char JSONS_ARRAY_WITH_NUMBER_0[] = "[0]";
json_init_from_str_positive_tests_impl_1(JSONS_ARRAY_WITH_NUMBER_0);
static const char JSONS_ARRAY_WITH_NUMBER_123[] = "[123]";
json_init_from_str_positive_tests_impl_1(JSONS_ARRAY_WITH_NUMBER_123);
static const char JSONS_ARRAY_WITH_NUMBER_0_123[] = "[0.123]";
json_init_from_str_positive_tests_impl_1(JSONS_ARRAY_WITH_NUMBER_0_123);
static const char JSONS_ARRAY_WITH_NUMBER_0_E_123[] = "[0E123]";
json_init_from_str_positive_tests_impl_1(JSONS_ARRAY_WITH_NUMBER_0_E_123);
static const char JSONS_ARRAY_WITH_EMPTY_OBJECT[] = "[{}]";
json_init_from_str_positive_tests_impl_1(JSONS_ARRAY_WITH_EMPTY_OBJECT);
static const char JSONS_ARRAY_WITH_EMPTY_OBJECT_WITH_NULL[] = R"JSON([{"null":null}])JSON";
json_init_from_str_positive_tests_impl_1(JSONS_ARRAY_WITH_EMPTY_OBJECT_WITH_NULL);
static const char JSONS_ARRAY_WITH_EMPTY_ARRAY[] = "[[]]";
json_init_from_str_positive_tests_base(JSONS_ARRAY_WITH_EMPTY_ARRAY);
json_init_from_str_positive_tests_impl_1(JSONS_ARRAY_WITH_EMPTY_ARRAY);
static const char JSONS_ARRAY_WITH_ARRAY_WITH_NULL[] = "[[null]]";
json_init_from_str_positive_tests_impl_1(JSONS_ARRAY_WITH_ARRAY_WITH_NULL);
static const char JSONS_ARRAY_WITH_STRING[] = R"JSON(["qwerty"])JSON";
json_init_from_str_positive_tests_impl_1(JSONS_ARRAY_WITH_STRING);
static const char JSONS_ARRAY_WITH_MULTIPLE_VALUES[] = "[null,123,false,\"string\",{},[],0]";
json_init_from_str_positive_tests_impl_1(JSONS_ARRAY_WITH_MULTIPLE_VALUES);
static const char JSONS_ARRAY_WITH_ARRAYS[] = "[[[[null]]],[[true,false]],[1,2,3]]";
json_init_from_str_positive_tests_impl_1(JSONS_ARRAY_WITH_ARRAYS);

static const char JSONS_EMPTY_OBJECT[] = "{}";
json_init_from_str_positive_tests_impl_1(JSONS_EMPTY_OBJECT);
static const char JSONS_OBJECT_WITH_NULL[] = R"JSON({"null":null})JSON";
json_init_from_str_positive_tests_impl_1(JSONS_OBJECT_WITH_NULL);
static const char JSONS_OBJECT_WITH_FALSE[] = R"JSON({"false":false})JSON";
json_init_from_str_positive_tests_impl_1(JSONS_OBJECT_WITH_FALSE);
static const char JSONS_OBJECT_WITH_TRUE[] = R"JSON({"true":true})JSON";
json_init_from_str_positive_tests_impl_1(JSONS_OBJECT_WITH_TRUE);
static const char JSONS_OBJECT_WITH_NUMBER_0[] = R"JSON({"0":0})JSON";
json_init_from_str_positive_tests_impl_1(JSONS_OBJECT_WITH_NUMBER_0);
static const char JSONS_OBJECT_WITH_NUMBER_123[] = R"JSON({"123":123})JSON";
json_init_from_str_positive_tests_impl_1(JSONS_OBJECT_WITH_NUMBER_123);
static const char JSONS_OBJECT_WITH_NUMBER_0_123[] = R"JSON({"0.123":0.123})JSON";
json_init_from_str_positive_tests_impl_1(JSONS_OBJECT_WITH_NUMBER_0_123);
static const char JSONS_OBJECT_WITH_NUMBER_0_E_123[] = R"JSON({"0E123":0E123})JSON";
json_init_from_str_positive_tests_impl_1(JSONS_OBJECT_WITH_NUMBER_0_E_123);
static const char JSONS_OBJECT_WITH_EMPTY_STRINGS[] = R"JSON({"":""})JSON";
json_init_from_str_positive_tests_impl_1(JSONS_OBJECT_WITH_EMPTY_STRINGS);
static const char JSONS_OBJECT_WITH_STRING[] = R"JSON({"qwerty":"qwerty"})JSON";
json_init_from_str_positive_tests_impl_1(JSONS_OBJECT_WITH_STRING);
static const char JSONS_OBJECT_WITH_EMPTY_OBJECT[] = R"JSON({"{}":{}})JSON";
json_init_from_str_positive_tests_impl_1(JSONS_OBJECT_WITH_EMPTY_OBJECT);
static const char JSONS_OBJECT_WITH_EMPTY_OBJECT_WITH_NULL[] = R"JSON({"{}":{},"null":null})JSON";
json_init_from_str_positive_tests_impl_1(JSONS_OBJECT_WITH_EMPTY_OBJECT_WITH_NULL);
static const char JSONS_OBJECT_WITH_EMPTY_ARRAY[] = R"JSON({"[]":[]})JSON";
json_init_from_str_positive_tests_impl_1(JSONS_OBJECT_WITH_EMPTY_ARRAY);
static const char JSONS_OBJECT_WITH_ARRAY_WITH_NULL[] = R"JSON({"[null]":[null]})JSON";
json_init_from_str_positive_tests_impl_1(JSONS_OBJECT_WITH_ARRAY_WITH_NULL);
static const char JSONS_OBJECT_WITH_MULTIPLE_VALUES[] = R"JSON({"null":null,"123":123,"false":false,"string":"string","{}":{},"[]":[],"0":0})JSON";
json_init_from_str_positive_tests_impl_1(JSONS_OBJECT_WITH_MULTIPLE_VALUES);
static const char JSONS_OBJECT_WITH_OBJECT_WITH_NULL[] = R"JSON({"key0":{"key1":null}})JSON";
json_init_from_str_positive_tests_impl_1(JSONS_OBJECT_WITH_OBJECT_WITH_NULL);
static const char JSONS_OBJECT_WITH_OBJECT_WITH_OBJECT_WITH_EMPTY_OBJECT[] = R"JSON({"key0":{"key1":{"key2":{}}}})JSON";
json_init_from_str_positive_tests_impl_1(JSONS_OBJECT_WITH_OBJECT_WITH_OBJECT_WITH_EMPTY_OBJECT);
static const char JSONS_OBJECT_WITH_OBJECTS[] = R"JSON({"objects":{"[[null]]":[[null]],"123":123},"":{"object":{"true":true,"false":false}},"[1,2,3]":[1,2,3]})JSON";
json_init_from_str_positive_tests_impl_1(JSONS_OBJECT_WITH_OBJECTS);

static const char JSONS_OBJECT_NON_REPEATED_KEYS[] = R"JSON({"key1":{"123":123},"key2":"key2","key3":123,"key4":[]})JSON";
json_init_from_str_positive_tests_impl_1(JSONS_OBJECT_NON_REPEATED_KEYS);
static const char JSONS_OBJECT_REPEATED_KEYS[] = R"JSON({"key1":{"123":123},"key2":"key2","key1":123,"key2":[]})JSON";
// static const char JSONS_OBJECT_REPEATED_KEYS_EXPECTED[] = R"JSON({"key1":123,"key2":[]})JSON";
json_init_from_str_positive_tests_impl_1(JSONS_OBJECT_REPEATED_KEYS);

static const char JSONS_SPACE_NULL[] = R"JSON(    null)JSON";
static const char JSONS_SPACE_NULL_EXPECTED[] = R"JSON(null)JSON";
json_init_from_str_positive_tests_impl_1_expected(JSONS_SPACE_NULL);
static const char JSONS_SPACE_FALSE[] = R"JSON(   false)JSON";
static const char JSONS_SPACE_FALSE_EXPECTED[] = R"JSON(false)JSON";
json_init_from_str_positive_tests_impl_1_expected(JSONS_SPACE_FALSE);
static const char JSONS_SPACE_TRUE[] = R"JSON(    true)JSON";
static const char JSONS_SPACE_TRUE_EXPECTED[] = R"JSON(true)JSON";
json_init_from_str_positive_tests_impl_1_expected(JSONS_SPACE_TRUE);
static const char JSONS_SPACE_NUMBER[] = R"JSON(  12354)JSON";
static const char JSONS_SPACE_NUMBER_EXPECTED[] = R"JSON(12354)JSON";
json_init_from_str_positive_tests_impl_1_expected(JSONS_SPACE_NUMBER);
static const char JSONS_SPACE_STRING[] = R"JSON(  "string")JSON";
static const char JSONS_SPACE_STRING_EXPECTED[] = R"JSON("string")JSON";
json_init_from_str_positive_tests_impl_1_expected(JSONS_SPACE_STRING);
static const char JSONS_SPACE_ARRAY[] = R"JSON(   [  null  ,  true  ,  false  ,  123  ,  "string"  ,  [   ]  ,  {   }    ])JSON";
static const char JSONS_SPACE_ARRAY_EXPECTED[] = R"JSON([null,true,false,123,"string",[],{}])JSON";
json_init_from_str_positive_tests_impl_1_expected(JSONS_SPACE_ARRAY);
static const char JSONS_SPACE_ARRAY_INDENT[] = R"JSON(
    [
        null,
        true,
        false,
        123,
        "string",
        [

        ],
        {
        
        }
    ])JSON";
static const char JSONS_SPACE_ARRAY_INDENT_EXPECTED[] = R"JSON([null,true,false,123,"string",[],{}])JSON";
json_init_from_str_positive_tests_impl_1_expected(JSONS_SPACE_ARRAY_INDENT);
static const char JSONS_SPACE_OBJECT[] = R"JSON(  {  "null"  :  null  ,  "true"  :  true  ,  "number"  :  1234  ,   "string"   :  "string"  ,  "array"  :  [  ]  ,  "object"  :  {  }  })JSON";
static const char JSONS_SPACE_OBJECT_EXPECTED[] = R"JSON({"null":null,"true":true,"number":1234,"string":"string","array":[],"object":{}})JSON";
json_init_from_str_positive_tests_impl_1_expected(JSONS_SPACE_OBJECT);
static const char JSONS_SPACE_OBJECT_INDENT[] = R"JSON(  
    {
        "null" : null,
        "true" : true,
        "number" : 1234,
        "string" : "string",
        "array" : [

        ],
        "object" : {

        }
    })JSON";
static const char JSONS_SPACE_OBJECT_INDENT_EXPECTED[] = R"JSON({"null":null,"true":true,"number":1234,"string":"string","array":[],"object":{}})JSON";
json_init_from_str_positive_tests_impl_1_expected(JSONS_SPACE_OBJECT_INDENT);
}
