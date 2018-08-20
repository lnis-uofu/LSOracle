; RUN: sed 's/^;[ ]*CHECK.\+$//g' %s | sed "s@XXX@%S@" | %OutputCheck -d %s

; CHECK-L: ${CHECKFILE_ABS_PATH}(${LINE:+1})
XXX/check-file-name-abs.smt2(4)
