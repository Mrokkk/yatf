#include "../include/yatf.h"

TEST(self_tests, require_works) {
    REQUIRE(true);
    REQUIRE_FALSE(false);
}

