#include <gtest/gtest.h>
#include "Parallel.hpp"

namespace utility {

    TEST(Parallel, Common)
    {
        std::vector<int> vec;
        vec.resize(1000);
        for (auto i = 0; i < vec.size(); i++) 
            vec[i] = int(i);
        auto retvec = ArrayTaskHelper::map([](int a) {
            return a + 1;
        }, vec, vec.size(), ArrayMTHelper::calcTaskNumber(vec.size()));

        auto sum = ArrayTaskHelper::reduce([](int a, int b) { return a + b; }
            , retvec, retvec.size(), ArrayMTHelper.calcTaskNumber(retvec.size()));
        EXPECT_EQ(sum, vec.size() * (1 + vec.size()) / 2);
    }

}