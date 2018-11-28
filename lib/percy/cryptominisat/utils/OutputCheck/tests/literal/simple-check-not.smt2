; RUN: sed '/^;/d' %s | %OutputCheck %s

; CHECK-L: foo
something foo
; CHECK-NOT-L: boo
; CHECK-NOT-L: foo
something coo
something doo
; CHECK-L: boo^
hello ^ there ^boo^
