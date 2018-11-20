; RUN: sed 's/^;[ ]*CHECK.\+$//g' %s | %OutputCheck %s
; CHECK: foo

foo

; CHECK-NOT: bang

; CHECK: boo
boo
