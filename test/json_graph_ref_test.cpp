/// Copyright © Alexander Kaluzhnyy
#include <cstddef>
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

#define ASSERT_JSONSTREQ(object, str) ({         \
    char* object##_str = json_sprint(object, 0); \
    ASSERT_STREQ(str, object##_str);             \
    free(object##_str);                          \
})

TEST_F(json_graph_ref_test, deinit_no_effect_for_objects_have_root_positive)
{
    log_trace_func();
    json_t* obj = json_init_from_str("[[123]]", nullptr);
    ASSERT_NE(nullptr, obj);
    json_t* child = json_get_by_id(obj, 0);
    ASSERT_NE(nullptr, child);
    json_deinit(child);
    EXPECT_EQ(child, json_get_by_id(obj, 0));
    ASSERT_JSONSTREQ(obj, "[[123]]");
    json_deinit(obj);
}
TEST_F(json_graph_ref_test, create_simple_circular_to_empty_array_positive)
{
    log_trace_func();
    json_t* root = json_init_from_str("[]", nullptr);
    ASSERT_NE(nullptr, json_set_by_id(root, root, 0));
    ASSERT_JSONSTREQ(root, "[[]]");
    json_deinit(root);
}
TEST_F(json_graph_ref_test, create_simple_circular_to_array_with_null_positive)
{
    log_trace_func();
    json_t* root = json_init_from_str("[null]", nullptr);
    ASSERT_NE(nullptr, json_set_by_id(root, root, 0));
    ASSERT_JSONSTREQ(root, "[[null]]");
    json_deinit(root);
}

TEST_F(json_graph_ref_test, create_simple_circular_to_array_with_array_positive)
{
    log_trace_func();
    json_t* root = json_init_from_str("[[]]", nullptr);
    ASSERT_NE(nullptr, json_set_by_id(root, root, 0));
    ASSERT_JSONSTREQ(root, "[[[]]]");
    json_deinit(root);
}
TEST_F(json_graph_ref_test, create_simple_circular_to_array_with_array_with_null_positive)
{
    log_trace_func();
    json_t* root = json_init_from_str("[[null]]", nullptr);
    ASSERT_NE(nullptr, json_set_by_id(root, root, 0));
    ASSERT_JSONSTREQ(root, "[[[null]]]");
    json_deinit(root);
}
TEST_F(json_graph_ref_test, create_double_insert_positive)
{
    log_trace_func();
    json_t* root = json_init_from_str("[null]", nullptr);
    ASSERT_NE(nullptr, json_set_by_id(root, root, 1));
    ASSERT_NE(nullptr, json_set_by_id(root, root, 2));
    ASSERT_JSONSTREQ(root, "[null,[null],[null,[null]]]");
    json_deinit(root);
}
TEST_F(json_graph_ref_test, deep_ref)
{
    log_trace_func();
    json_t* root = json_init_from_str("[[null]]", nullptr);
    ASSERT_NE(nullptr, json_set_by_id(json_get_by_id(root, 0), root, 0));
    ASSERT_JSONSTREQ(root, "[[[[null]]]]");
    json_deinit(root);
}

class json_graph_mega_ref_test : public Test {

private:
    json_t* node1 = nullptr;

protected:
    json_t* node = nullptr;
    void SetUp() override
    {
        log_trace_func();
        node1 = json_init_from_str("[1]", nullptr);
        json_t* node2 = json_init_from_str("[2]", nullptr);
        json_t* node3 = json_init_from_str("[3]", nullptr);
        json_t* node4 = json_init_from_str("[4]", nullptr);
        json_t* node5 = json_init_from_str("[5]", nullptr);
        json_t* node6 = json_init_from_str("[6]", nullptr);
        ASSERT_NE(nullptr, node1);
        ASSERT_NE(nullptr, node2);
        ASSERT_NE(nullptr, node3);
        ASSERT_NE(nullptr, node4);
        ASSERT_NE(nullptr, node5);
        ASSERT_NE(nullptr, node6);
        ASSERT_NE(nullptr, json_set_by_id(node5, node6, 1));
        ASSERT_NE(nullptr, json_set_by_id(node4, node5, 1));
        ASSERT_NE(nullptr, json_set_by_id(node2, node4, 1));
        ASSERT_NE(nullptr, json_set_by_id(node2, node5, 2));
        ASSERT_NE(nullptr, json_set_by_id(node3, node5, 1));
        ASSERT_NE(nullptr, json_set_by_id(node3, node6, 2));
        ASSERT_NE(nullptr, json_set_by_id(node1, node2, 1));
        ASSERT_NE(nullptr, json_set_by_id(node1, node3, 2));
        ASSERT_NE(nullptr, json_set_by_id(node1, node5, 3));
/*
digraph G {
1->2
1->5
1->3
2->5
2->4
3->6
3->5
4->5
5->6
}
*/
#define NODE6 "[6]"
        ASSERT_JSONSTREQ(node6, "[6]");
        json_deinit(node6);
#define NODE5 "[5," NODE6 "]" // "[5,[6]]"
        ASSERT_JSONSTREQ(node5, NODE5);
        json_deinit(node5);
#define NODE4 "[4," NODE5 "]" // "[4,[5,[6]]]"
        ASSERT_JSONSTREQ(node4, NODE4);
        json_deinit(node4);
#define NODE3 "[3," NODE5 "," NODE6 "]" // "[3,[5,[6]],[6]]"
        ASSERT_JSONSTREQ(node3, NODE3);
        json_deinit(node3);
#define NODE2 "[2," NODE4 "," NODE5 "]" // "[2,[4,[5,[6]]],[5,[6]]]"
        ASSERT_JSONSTREQ(node2, NODE2);
        json_deinit(node2);
#define NODE1 "[1," NODE2 "," NODE3 "," NODE5 "]" // "[1,[2,[4,[5,[6]]],[5,[6]]],[3,[5,[6]],[6]],[5,[6]]]"
        ASSERT_JSONSTREQ(node1, NODE1);
        node = json_copy(node1);
        ASSERT_JSONSTREQ(node1, NODE1);
        ASSERT_JSONSTREQ(node, NODE1);
    }
    void TearDown() override
    {
        ASSERT_JSONSTREQ(node1, NODE1);
        json_deinit(node);
        ASSERT_JSONSTREQ(node1, NODE1);
        json_deinit(node1);
    }
};

TEST_F(json_graph_mega_ref_test, base)
{
}
//#define get(node, id) json_get_by_id(node, id)
#define set(node, child, id) json_set_by_id(node, child, id)

#define mega_ref_test(name, action, expected_str) \
    TEST_F(json_graph_mega_ref_test, name)        \
    {                                             \
        ASSERT_NE(nullptr, action);               \
        ASSERT_JSONSTREQ(node, expected_str);     \
    }

mega_ref_test(set_node_1_to_node, set(node, node, 1), "[1," NODE1 "," NODE3 "," NODE5 "]");
mega_ref_test(set_node_2_to_node, set(node, node, 2), "[1," NODE2 "," NODE1 "," NODE5 "]");
mega_ref_test(set_node_3_to_node, set(node, node, 3), "[1," NODE2 "," NODE3 "," NODE1 "]");

// mega_ref_test(set_Node_1_1_to_node, set(get(node, 1), node, 1), "[1,[2," NODE1 "," NODE5 "]," NODE3 "," NODE5 "]");
// mega_ref_test(set_Node_1_2_to_node, set(get(node, 1), node, 2), "[1,[2," NODE4 "," NODE1 "]," NODE3 "," NODE5 "]");
}