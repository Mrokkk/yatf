#define YATF_MAIN
#include "../include/yatf.h"
#include <cstdio>

int main(int argc, const char *argv[]) {
    return yatf::main(printf, argc, argv);
}

