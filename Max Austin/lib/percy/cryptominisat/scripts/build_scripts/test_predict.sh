#!/bin/bash
set -e
set -x

DIR=test_predict
./predict_one.sh drat_test2.cnf ${DIR}

dot -Tpng ${DIR}/data.sqlite.tree.dot -o tree.png
echo "Execute:"
echo "display tree.png"
