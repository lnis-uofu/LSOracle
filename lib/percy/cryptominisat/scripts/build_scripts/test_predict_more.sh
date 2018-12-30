#!/bin/bash
set -e
set -x

status=`./cryptominisat5 --hhelp | grep sql || ret=$?`
if [ "$ret" -ne 0 ]; then
    echo "You must compile SQL into cryptominisat"
    exit -1
fi

OUTDIR="test_predict_more"
rm -rf ${OUTDIR}
mkdir -p ${OUTDIR}

for i in `seq 1 6`;
do
    FNAME="my_cnf_$i.cnf"

    # generate CNF
    if [ $((a%2)) -eq 0 ];
    then
         ./tests/cnf-utils/sgen4 -unsat -n 64 -s 1 > ${OUTDIR}/${FNAME}
    fi

    if [ $((a%2)) -eq 1 ];
    then
        ./tests/sha1-sat/sha1-gen --attack preimage --rounds 16 \
            --message-bits 430 --zero --seed ${i} > "${OUTDIR}/${FNAME}"
    fi

    # running CNF
    # --maxtemp 1000000000 --presimp 0 --schedsimp 0
    ./cryptominisat5 --verb 0 --keepguess 0 --zero-exit-status --clid --sql 2 \
        "${OUTDIR}/${FNAME}" "${OUTDIR}/drat_out" --sqlitedb "${OUTDIR}/${FNAME}.sqlite"

    # getting lemmas used from DRAT
    ./tests/drat-trim/drat-trim "${OUTDIR}/${FNAME}" "${OUTDIR}/drat_out" -l "${OUTDIR}/lemmas${i}"

    # add lemma indices that were good
    ./add_lemma_ind.py "${OUTDIR}/${FNAME}.sqlite" "${OUTDIR}/lemmas${i}"
    echo "Done with $i"
done


# run prediction on SQLite database
./predict.py ${OUTDIR}/*.sqlite

# generate DOT and display it
dot -Tpng ${OUTDIR}/final.dot -o ${OUTDIR}/tree.png
okular ${OUTDIR}/tree.png
