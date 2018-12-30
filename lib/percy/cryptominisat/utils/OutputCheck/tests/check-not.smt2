; RUN: sed 's/^;[ ]*CHECK.\+$//g' %s | %OutputCheck %s
hello there
; CHECK-NOT: poo
