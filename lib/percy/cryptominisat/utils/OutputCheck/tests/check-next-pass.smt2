; RUN: sed 's/^;[ ]*CHECK.\+$//g' %s | %OutputCheck %s
; CHECK: foo
; CHECK-NEXT: bar
foo
bar
