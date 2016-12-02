#include "../include/yatf.h"

TEST(self_tests, require_works) {
    REQUIRE(1 == 1);
    REQUIRE_FALSE(0 == 1);
}

