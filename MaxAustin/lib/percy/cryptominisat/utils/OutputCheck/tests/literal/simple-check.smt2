; RUN: sed 's/^;[ ]*CHECK.\+$//g' %s | %OutputCheck %s

; CHECK-L: foo
something foo
; CHECK-L: boo
something boo
; CHECK-L: boo^
hello ^ there ^boo^
