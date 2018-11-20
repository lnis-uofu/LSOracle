; RUN: sed 's/^;[ ]*CHECK.\+$//g' %s | %OutputCheck %s

; CHECK-NOT: foo
; CHECK-L: boo
something
boo
