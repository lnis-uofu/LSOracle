; RUN: sed 's/^;[ ]*CHECK.\+$//g' %s | %OutputCheck -d %s

; CHECK-L: ${CHECKFILE_NAME}(${LINE:+1})
check-file-name.smt2(4)
