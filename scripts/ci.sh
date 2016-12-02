#!/bin/bash

set -e

docker run -t -v $PWD:$PWD $IMAGE /bin/bash -c "CXX=$COMPILER JOB=$JOB $PWD/scripts/run_tests.sh"

