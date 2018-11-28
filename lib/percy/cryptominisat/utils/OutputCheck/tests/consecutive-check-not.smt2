; RUN: sed 's/^;[ ]*CHECK.\+$//g' %s | %OutputCheck %s
; CHECK-NOT: foo
; CHECK-NOT: bar
one
two
