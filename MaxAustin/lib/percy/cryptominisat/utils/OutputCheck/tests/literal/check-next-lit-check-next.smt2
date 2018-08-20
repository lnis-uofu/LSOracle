; RUN: sed '/^;/d' %s | %OutputCheck %s
; CHECK-NEXT-L: foo
; CHECK-NEXT: boo
something foo
boo
