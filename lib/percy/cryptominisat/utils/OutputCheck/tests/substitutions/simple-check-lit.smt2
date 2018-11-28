; RUN: sed 's/^;[ ]*CHECK.\+$//g' %s | %OutputCheck -d %s

; CHECK-L: file.c(${LINE:+1})
file.c(4)
