#!/bin/bash
set -e
set -x

./cryptominisat5 drat_test.cnf drat_test.drat --zero-exit-status
cat drat_test.drat
./test/drat-trim/drat-trim drat_test.cnf drat_test.drat
