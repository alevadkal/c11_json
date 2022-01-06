/// Copyright © Alexander Kaluzhnyy
#include <gtest/gtest.h>
#include "json.h"
#include "json_printer.h"
#include "log.h"
#include <stdio.h>

namespace json_test {

using namespace ::testing;

class json_graph_ref_test : public Test {
protected:
};

#define EXPECT_JSON_EQ(object, str) ({           \
    char* object##_str = json_sprint(object, 0); \
    EXPECT_STREQ(str, object##_str);             \
    free(object##_str);                          \
})

TEST_F(json_graph_ref_test, deinit_no_effect_for_objects_have_root_positive)
{
    json_t* obj = json_init_from_str("[[123]]", nullptr);
    ASSERT_NE(nullptr, obj);
    json_t* child = json_get_by_id(obj, 0);
    ASSERT_NE(nullptr, child);
    json_deinit(child);
    EXPECT_EQ(child, json_get_by_id(obj, 0));
    EXPECT_JSON_EQ(obj, "[[123]]");
    json_deinit(obj);
}
TEST_F(json_graph_ref_test, create_simple_circular_to_empty_array_positive)
{
    json_t* root = json_init_from_str("[]", nullptr);
    ASSERT_NE(nullptr, json_set_by_id(root, root, 0));
    EXPECT_JSON_EQ(root, "[[]]");
    json_deinit(root);
}
TEST_F(json_graph_ref_test, create_simple_circular_to_array_with_null_positive)
{
    json_t* root = json_init_from_str("[null]", nullptr);
    ASSERT_NE(nullptr, json_set_by_id(root, root, 0));
    EXPECT_JSON_EQ(root, "[[null]]");
    json_deinit(root);
}

TEST_F(json_graph_ref_test, create_simple_circular_to_array_with_array_positive)
{
    json_t* root = json_init_from_str("[[]]", nullptr);
    ASSERT_NE(nullptr, json_set_by_id(root, root, 0));
    EXPECT_JSON_EQ(root, "[[[]]]");
    json_deinit(root);
}
TEST_F(json_graph_ref_test, create_simple_circular_to_array_with_array_with_null_positive)
{
    json_t* root = json_init_from_str("[[null]]", nullptr);
    ASSERT_NE(nullptr, json_set_by_id(root, root, 0));
    EXPECT_JSON_EQ(root, "[[[null]]]");
    json_deinit(root);
}
TEST_F(json_graph_ref_test, create_double_insert_positive)
{
    json_t* root = json_init_from_str("[null]", nullptr);
    ASSERT_NE(nullptr, json_set_by_id(root, root, 1));
    ASSERT_NE(nullptr, json_set_by_id(root, root, 2));
    EXPECT_JSON_EQ(root, "[null,[null],[null,[null]]]");
    json_deinit(root);
}
/*
TEST_F(json_graph_ref_test, deep_ref)
{
    json_t* root = json_init_from_str("[[null]]", nullptr);
    ASSERT_NE(nullptr, json_set_by_id(json_get_by_id(root, 0), root, 0));
    EXPECT_JSON_EQ(root, "[[[[null]]]]");
    json_deinit(root);
}
*/
/*
TEST_F(json_graph_ref_test, create_simple_circular_ref)
{
    json_t* root = json_init_from_str("[[{},123],null]", nullptr);
    json_t* child = json_get_by_id(root, 0);
    ASSERT_NE(nullptr, json_set_by_id(child, root, 1));
    char* str = json_sprint(root, 0);
    EXPECT_STREQ(str, "[[[[{},123],null]],null]");
    json_deinit(root);
}
*/
}