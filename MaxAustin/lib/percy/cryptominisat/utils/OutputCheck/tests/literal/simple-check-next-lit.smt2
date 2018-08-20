; RUN: sed '/^;[ ]*CHECK.\+$/d' %s | %OutputCheck %s

; CHECK-L: foo
something foo
; CHECK-NEXT-L: bo$$
something bo$$
; CHECK-NOT: xxx
xxyxx
