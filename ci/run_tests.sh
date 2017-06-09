#!/bin/bash

set -e

dir=$(dirname $0)

cd $dir/..
mkdir -p build
cd build

cores=$(nproc)

case "$JOB" in
    "valgrind")
        cmake -DCPP_STD=${CPP_STANDARD:-c++1z} ..
        make tests-valgrind -j$cores ;;
    "coverage")
        cmake -DCOVERAGE=ON -DCPP_STD=${CPP_STANDARD:-c++1z} ..
        make tests-cov -j$cores ;;
    "sanitizers")
        cmake -DSANITIZERS=ON -DCPP_STD=${CPP_STANDARD:-c++1z} ..
        make tests-run -j$cores ;;
    *)
        cmake -DCPP_STD=${CPP_STANDARD:-c++1z} ..
        make tests-run -j$cores ;;
esac

