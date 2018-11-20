#!/bin/bash
set -e
set -x

OUTDIR="out_test_id"
mkdir -p ${OUTDIR}
rm -f ${OUTDIR}/test_id.drat*
rm -f "${OUTDIR}/lemmas"
rm -f "${OUTDIR}/core"

echo ""
echo "Solving..."
./cryptominisat5 --clid --verb 0 test_id.cnf "${OUTDIR}/test_id.drat" --zero-exit-status


echo ""
echo "CHANGED DRAT:"
./tests/drat-trim/drat-trim test_id.cnf "${OUTDIR}/test_id.drat" -c "${OUTDIR}/core" -l "${OUTDIR}/lemmas"

echo ""
echo "DRAT NORM:"
./clean_drat.py "${OUTDIR}/test_id.drat" > "${OUTDIR}/test_id.drat2"
./tests/drat-trim/drat-trim test_id.cnf "${OUTDIR}/test_id.drat2"

diff -y "${OUTDIR}/test_id.drat" "${OUTDIR}/test_id.drat2" | head -n 10
