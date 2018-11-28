; RUN: sed 's/^;[ ]*CHECK.\+$//g' %s | not %OutputCheck %s 2> %t
; RUN: grep 'Found a match for CheckNot Directive' %t
; RUN: grep 'in <stdin>:9' %t
; CHECK: foo

foo

; CHECK-NOT: bang
bang
; CHECK: boo
boo
