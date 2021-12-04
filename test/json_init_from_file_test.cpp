/// Copyright © Alexander Kaluzhnyy
#include <gtest/gtest.h>
#include "json.h"
#include "json_printer.h"
#include "log.h"
#include <stdio.h>

namespace json_test {

using namespace ::testing;

class json_init_from_file_tests : public Test {
protected:
    FILE* m_fin = nullptr;
    FILE* m_fout = nullptr;
    char* m_out = nullptr;
    size_t m_out_size;
    json_t* m_object = nullptr;
    void TearDown() override
    {
        if (m_fin != nullptr) {
            fclose(m_fin);
        }
        if (m_fout != nullptr) {
            fclose(m_fout);
        }
        free(m_out);
        json_deinit(m_object);
    }
};

const char JSON_POSITIVE_STRING[] = "123";

TEST_F(json_init_from_file_tests, positive_zero_end)
{
    m_fin = fmemopen(const_cast<char*>(JSON_POSITIVE_STRING), sizeof(JSON_POSITIVE_STRING), "r");
    m_fout = open_memstream(&m_out, &m_out_size);
    ASSERT_NE(nullptr, m_fin);
    ASSERT_NE(nullptr, m_fout);
    m_object = json_init_from_file(m_fin);
    ASSERT_NE(nullptr, m_object);
    ASSERT_NE(-1, json_fprint(m_object, 0, m_fout));
    fflush(m_fout);
    EXPECT_STREQ(JSON_POSITIVE_STRING, m_out);
}

TEST_F(json_init_from_file_tests, positive_eof_end)
{
    m_fin = fmemopen(const_cast<char*>(JSON_POSITIVE_STRING), sizeof(JSON_POSITIVE_STRING) - 1, "r");
    m_fout = open_memstream(&m_out, &m_out_size);
    ASSERT_NE(nullptr, m_fin);
    ASSERT_NE(nullptr, m_fout);
    m_object = json_init_from_file(m_fin);
    ASSERT_NE(nullptr, m_object);
    ASSERT_NE(-1, json_fprint(m_object, 0, m_fout));
    fflush(m_fout);
    EXPECT_STREQ(JSON_POSITIVE_STRING, m_out);
}

const char JSON_NEGATIVE_STRING[] = "qwerty";

TEST_F(json_init_from_file_tests, negative)
{
    m_fin = fmemopen(const_cast<char*>(JSON_NEGATIVE_STRING), sizeof(JSON_NEGATIVE_STRING), "r");
    ASSERT_NE(nullptr, m_fin);
    m_object = json_init_from_file(m_fin);
    ASSERT_EQ(nullptr, m_object);
}
}