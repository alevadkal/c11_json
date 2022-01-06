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

TEST_F(json_graph_ref_test, deinit_no_effect_for_objects_have_root_positive)
{
    json_t* obj = json_init_from_str("[[123]]", nullptr);
    ASSERT_NE(nullptr, obj);
    json_t* child = json_get_by_id(obj, 0);
    ASSERT_NE(nullptr, child);
    json_deinit(child);
    EXPECT_EQ(child, json_get_by_id(obj, 0));
    char* str = json_sprint(obj, 0);
    EXPECT_STREQ(str, "[[123]]");
    free(str);
    json_deinit(obj);
}

TEST_F(json_graph_ref_test, create_simple_circular_to_empty_array_positive)
{
    json_t* root = json_init_from_str("[]", nullptr);
    ASSERT_NE(nullptr, json_set_by_id(root, root, 0));
    char* str = json_sprint(root, 0);
    EXPECT_STREQ(str, "[[]]");
    free(str);
    json_deinit(root);
}
/*
TEST_F(json_graph_ref_test, create_simple_circular_to_array_with_array_positive)
{
    json_t* root = json_init_from_str("[[]]", nullptr);
    ASSERT_NE(nullptr, json_set_by_id(root, root, 0));
    char* str = json_sprint(root, 0);
    EXPECT_STREQ(str, "[[[]]]");
    free(str);
    json_deinit(root);
}*/
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