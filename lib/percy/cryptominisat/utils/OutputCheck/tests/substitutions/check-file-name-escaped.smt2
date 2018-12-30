; RUN: sed 's/^;[ ]*CHECK.\+$//g' %s | %OutputCheck -d %s

; CHECK-L: \${CHECKFILE_NAME}(${LINE:+1})
${CHECKFILE_NAME}(4)
