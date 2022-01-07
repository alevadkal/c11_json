/// Copyright © Alexander Kaluzhnyy
#include <cstddef>
#include <gmock/gmock-actions.h>
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
    NiceMock<system_mock> m_mock;
};
/*
TEST_F(base_tester, test)
{
    log_trace_func();
    for (int i = 0; m_mock.VerifyAndClear(); i++) {
        InSequence s;
        // EXPECT_CALL(m_mock, calloc(_, _)).Times(i).WillRepeatedly(DoDefault());
        EXPECT_CALL(m_mock, calloc(_, _)).WillOnce(ReturnNull());
        std::cout << "   Check for " << i << "succesfull calloc cals" << std::endl;
        auto obj = json_init_from_str("123", nullptr);
        if (obj != nullptr) {
            std::cout << "All calloc cals checked" << std::endl;
            json_deinit(obj);
        }
    }
}
*/
}