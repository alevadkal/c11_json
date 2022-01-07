/// Copyright © Alexander Kaluzhnyy
#include <cstddef>
#include <gmock/gmock-actions.h>
#include <gmock/gmock-cardinalities.h>
#include <gmock/gmock-nice-strict.h>
#include <gmock/gmock-spec-builders.h>
#include <gtest/gtest.h>
#include <ostream>
#include "json.h"
#include "log.h"
#include "system_mock.hpp"

namespace json_test {

using namespace ::testing;

class base_tester : public Test {

protected:
    json_t* m_object = nullptr;
    ~base_tester()
    {
        json_deinit(m_object);
    }
};
TEST_F(base_tester, test)
{
    log_trace_func();
    for (int i = 0; true; i++) {
        NiceMock<system_mock> m_mock;
        InSequence s;
        log_debug_msg("Check for %i succesfull calloc calls", i);
        if (i) {
            EXPECT_CALL(m_mock, calloc(_, _))
                .Times(i)
                .WillRepeatedly(DoDefault());
        }
        EXPECT_CALL(m_mock, calloc(_, _))
            .Times(AtMost(1))
            .WillRepeatedly(ReturnNull());
        m_object = json_init_from_str("123", nullptr);
        if (m_object) {
            log_debug_msg("Checking finished");
            break;
        }
    }
}
}