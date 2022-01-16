/// Copyright © Alexander Kaluzhnyy
#include <gtest/gtest.h>
#include "json.h"
#include "json_printer.h"
#include "log.h"
#include <stdio.h>

namespace json_test {

using namespace ::testing;

class json_base : public Test {
protected:
    json_t* m_object = nullptr;
    json_t* m_child = nullptr;
    json_t** get_child()
    {
        if (m_child == nullptr) {
            m_child = json_init_from_str("[{},[],\"\",123,false,true,null]", nullptr);
            EXPECT_NE(nullptr, m_child);
        }
        return &m_child;
    }

    virtual ~json_base()
    {
        log_trace_func();
        json_deinit(&m_child);
        json_deinit(&m_object);
    }
};
const char UNEXSISTED[] = "unexisted_key";
const char NEW_KEY[] = "new_key";
#define SOME_NUMBER "12345"
#define SOME_STRING "some string"

#define JSON_STR(STRING) "\"" STRING "\""

#define KEY1 "1"
#define KEY2 "2"

#define NODE_FIXTURE(node_type, string)                     \
    class json_##node_type##_node : public json_base {      \
    protected:                                              \
        void SetUp() override                               \
        {                                                   \
            log_trace_func();                               \
            m_object = json_init_from_str(string, nullptr); \
            ASSERT_NE(nullptr, m_object);                   \
        }                                                   \
    }

NODE_FIXTURE(null, "null");
NODE_FIXTURE(true, "true");
NODE_FIXTURE(false, "false");
NODE_FIXTURE(number, SOME_NUMBER);
NODE_FIXTURE(string, JSON_STR(SOME_STRING));
NODE_FIXTURE(empty_array, "[]");
NODE_FIXTURE(empty_object, "{}");
NODE_FIXTURE(not_empty_array, "[" SOME_NUMBER "," JSON_STR(SOME_STRING) "]");
NODE_FIXTURE(not_empty_object, "{" JSON_STR(KEY1) ":" SOME_NUMBER "," JSON_STR(KEY2) ":" JSON_STR(SOME_STRING) "}");

#define test_size(node_type, type, method, expected) \
    TEST_F(json_##node_type##_node, method##_##type) \
    {                                                \
        EXPECT_EQ(expected, method(&m_object));      \
    }

#define test_str(node_type, type, method, expected)  \
    TEST_F(json_##node_type##_node, method##_##type) \
    {                                                \
        EXPECT_STREQ(expected, method(&m_object));   \
    }

#define test_node_get(node_type, type, method, key, expected)         \
    TEST_F(json_##node_type##_node, method##_key_##key##_##type)      \
    {                                                                 \
        EXPECT_STREQ(expected, json_get_str(method(&m_object, key))); \
    }
#define test_node_key(node_type, type, method, key, expected)    \
    TEST_F(json_##node_type##_node, method##_key_##key##_##type) \
    {                                                            \
        EXPECT_STREQ(expected, method(&m_object, key));          \
    }

#define test_node_set(node_type, type, method, key, expected)     \
    TEST_F(json_##node_type##_node, method##_key_##key##_##type)  \
    {                                                             \
        EXPECT_EQ(expected, method(&m_object, get_child(), key)); \
    }

#define BASE_TESTS(type, type_str, expected_str)                         \
    test_str(type, positive, json_get_type, type_str);                   \
    test_str(type, positive, json_get_str, expected_str);                \
    test_size(type, negative, json_size, 0);                             \
    test_node_get(type, negative, json_get_by_id, 0, nullptr);           \
    test_node_set(type, negative, json_set_by_id, 0, nullptr);           \
    test_node_key(type, negative, json_key, 0, nullptr);                 \
    test_node_get(type, negative, json_get_by_key, UNEXSISTED, nullptr); \
    test_node_set(type, negative, json_set_by_key, NEW_KEY, nullptr);

BASE_TESTS(null, JSON_NULL, JSON_NULL)
BASE_TESTS(true, JSON_TRUE, JSON_TRUE)
BASE_TESTS(false, JSON_FALSE, JSON_FALSE)
BASE_TESTS(number, JSON_NUMBER, SOME_NUMBER)
BASE_TESTS(string, JSON_STRING, SOME_STRING)

test_str(empty_array, positive, json_get_type, JSON_ARRAY);
test_str(empty_array, negative, json_get_str, nullptr);
test_size(empty_array, positive, json_size, 0);
test_node_get(empty_array, negative, json_get_by_id, 0, nullptr);
test_node_set(empty_array, positive, json_set_by_id, 0, &m_object);
test_node_key(empty_array, negative, json_key, 0, nullptr);
test_node_get(empty_array, negative, json_get_by_key, UNEXSISTED, nullptr);
test_node_set(empty_array, negative, json_set_by_key, NEW_KEY, nullptr);

test_str(empty_object, positive, json_get_type, JSON_OBJECT);
test_str(empty_object, negative, json_get_str, nullptr);
test_size(empty_object, positive, json_size, 0);
test_node_get(empty_object, negative, json_get_by_id, 0, nullptr);
test_node_set(empty_object, negative, json_set_by_id, 0, nullptr);
test_node_key(empty_object, negative, json_key, 0, nullptr);
test_node_get(empty_object, negative, json_get_by_key, UNEXSISTED, nullptr);
test_node_set(empty_object, negative, json_set_by_key, NEW_KEY, &m_object);

test_str(not_empty_array, positive, json_get_type, JSON_ARRAY);
test_str(not_empty_array, negative, json_get_str, nullptr);
test_size(not_empty_array, positive, json_size, 2);
test_node_get(not_empty_array, positive, json_get_by_id, 0, SOME_NUMBER);
test_node_get(not_empty_array, positive, json_get_by_id, 1, SOME_STRING);
test_node_get(not_empty_array, negative, json_get_by_id, 2, nullptr);
test_node_set(not_empty_array, positive, json_set_by_id, 0, &m_object);
test_node_set(not_empty_array, positive, json_set_by_id, 1, &m_object);
test_node_set(not_empty_array, positive, json_set_by_id, 2, &m_object);
test_node_set(not_empty_array, negative, json_set_by_id, 3, nullptr);
test_node_key(not_empty_array, negative, json_key, 0, nullptr);
test_node_get(not_empty_array, negative, json_get_by_key, UNEXSISTED, nullptr);
test_node_set(not_empty_array, negative, json_set_by_key, NEW_KEY, nullptr);

test_str(not_empty_object, positive, json_get_type, JSON_OBJECT);
test_str(not_empty_object, negative, json_get_str, nullptr);
test_size(not_empty_object, positive, json_size, 2);
test_node_get(not_empty_object, positive, json_get_by_id, 0, SOME_NUMBER);
test_node_get(not_empty_object, positive, json_get_by_id, 1, SOME_STRING);
test_node_get(not_empty_object, negative, json_get_by_id, 2, nullptr);
test_node_set(not_empty_object, positive, json_set_by_id, 0, &m_object);
test_node_set(not_empty_object, positive, json_set_by_id, 1, &m_object);
test_node_set(not_empty_object, negative, json_set_by_id, 2, nullptr);
test_node_key(not_empty_object, positive, json_key, 0, KEY1);
test_node_key(not_empty_object, positive, json_key, 1, KEY2);
test_node_key(not_empty_object, negative, json_key, 2, nullptr);
test_node_get(not_empty_object, negative, json_get_by_key, KEY1, SOME_NUMBER);
test_node_get(not_empty_object, negative, json_get_by_key, KEY2, SOME_STRING);
test_node_get(not_empty_object, negative, json_get_by_key, UNEXSISTED, nullptr);
test_node_set(not_empty_object, negative, json_set_by_key, NEW_KEY, &m_object);

#define test_node_set_and_check_result(node_type, method, key, value, expected)                         \
    TEST_F(json_##node_type##_node, method##_key_##key##_elem_##value##_positive)                       \
    {                                                                                                   \
        m_child = json_init_from_str(value, nullptr);                                                   \
        ASSERT_NE(nullptr, m_child);                                                                    \
        ASSERT_NE(nullptr, method(&m_object, &m_child, key));                                           \
        char* str = json_sprint(&m_object, 0);                                                          \
        EXPECT_STREQ(expected, str);                                                                    \
        free(str);                                                                                      \
    }                                                                                                   \
    TEST_F(json_##node_type##_node, copy_not_affected_##method##_key_##key##_elem_##value##_positive)   \
    {                                                                                                   \
        auto copy = json_copy(&m_object);                                                               \
        auto copy_str = json_sprint(&copy, 0);                                                          \
        {                                                                                               \
            m_child = json_init_from_str(value, nullptr);                                               \
            ASSERT_NE(nullptr, m_child);                                                                \
            ASSERT_NE(nullptr, method(&m_object, &m_child, key));                                       \
            char* str = json_sprint(&m_object, 0);                                                      \
            EXPECT_STREQ(expected, str);                                                                \
            free(str);                                                                                  \
        }                                                                                               \
        char* str = json_sprint(&copy, 0);                                                              \
        EXPECT_STREQ(str, copy_str);                                                                    \
        free(copy_str);                                                                                 \
        free(str);                                                                                      \
        json_deinit(&copy);                                                                             \
    }                                                                                                   \
    TEST_F(json_##node_type##_node, object_not_affected_##method##_key_##key##_elem_##value##_positive) \
    {                                                                                                   \
        auto copy = json_copy(&m_object);                                                               \
        auto object_str = json_sprint(&copy, 0);                                                        \
        {                                                                                               \
            m_child = json_init_from_str(value, nullptr);                                               \
            ASSERT_NE(nullptr, m_child);                                                                \
            ASSERT_NE(nullptr, method(&copy, &m_child, key));                                           \
            char* str = json_sprint(&copy, 0);                                                          \
            EXPECT_STREQ(expected, str);                                                                \
            free(str);                                                                                  \
        }                                                                                               \
        auto str = json_sprint(&m_object, 0);                                                           \
        EXPECT_STREQ(str, object_str);                                                                  \
        free(object_str);                                                                               \
        free(str);                                                                                      \
        json_deinit(&copy);                                                                             \
    }

#define NULL_STR "null"
test_node_set_and_check_result(empty_array, json_set_by_id, 0, NULL_STR, "[" NULL_STR "]");
#define ANY_DATA "{\"key\":[123,false,{},[]]}"
test_node_set_and_check_result(empty_array, json_set_by_id, 0, ANY_DATA, "[" ANY_DATA "]");
test_node_set_and_check_result(empty_object, json_set_by_key, KEY1, NULL_STR, "{" JSON_STR(KEY1) ":" NULL_STR "}");
test_node_set_and_check_result(empty_object, json_set_by_key, KEY2, ANY_DATA, "{" JSON_STR(KEY2) ":" ANY_DATA "}");
test_node_set_and_check_result(not_empty_array, json_set_by_id, 0, NULL_STR, "[" NULL_STR "," JSON_STR(SOME_STRING) "]");
test_node_set_and_check_result(not_empty_array, json_set_by_id, 1, NULL_STR, "[" SOME_NUMBER "," NULL_STR "]");
test_node_set_and_check_result(not_empty_array, json_set_by_id, 2, NULL_STR, "[" SOME_NUMBER "," JSON_STR(SOME_STRING) "," NULL_STR "]");
}