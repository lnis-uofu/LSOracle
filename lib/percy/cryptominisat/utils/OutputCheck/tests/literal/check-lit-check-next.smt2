; RUN: sed 's/^;[ ]*CHECK.\+$//g' %s | %OutputCheck %s

; CHECK-L: foo
; CHECK-NEXT: boo
something foo
boo
