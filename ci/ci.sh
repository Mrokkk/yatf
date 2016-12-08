#!/bin/bash

set -e

docker run -t -v $PWD:$PWD $IMAGE /bin/bash -c "CPP_STANDARD=$CPP_STANDARD CXX=$COMPILER JOB=$JOB $PWD/ci/run_tests.sh"

