## YATF
Branch | Build | Coverage
------ | ----- | --------
master | [![Build Status](https://travis-ci.org/Mrokkk/yatf.svg?branch=master)](https://travis-ci.org/Mrokkk/yatf) | [![codecov](https://codecov.io/gh/Mrokkk/yatf/branch/master/graph/badge.svg)](https://codecov.io/gh/Mrokkk/yatf)
devel | [![Build Status](https://travis-ci.org/Mrokkk/yatf.svg?branch=devel)](https://travis-ci.org/Mrokkk/yatf) | [![codecov](https://codecov.io/gh/Mrokkk/yatf/branch/devel/graph/badge.svg)](https://codecov.io/gh/Mrokkk/yatf)

YATF, which stands for Yet Another Test Framework, is an automated test framework for C++. It is designed to be usable in a bare-metal environment and consists only of one header file.
### How to use it
```
// in main.cpp
#define YATF_MAIN
#include <yatf.hpp>
#include <cstdio>

int main(int argc, const char *argv[]) {
    // you have to provide printf-like function -
    // in this case it's from cstdio
    return yatf::main(printf, argc, argv);
}

```
```
// in tests
#include <yatf.hpp>

TEST(suite_name, test_case1) {
    REQUIRE(true);
    REQUIRE_EQ(1, 1);
}
```
It also supports fixtures, which are passed as a third parameter to the `TEST` macro:
```
struct some_fixture {};

TEST(suite_name, test_case2, some_fixture) {
}
```
