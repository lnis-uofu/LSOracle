; RUN: sed '/^;/d' %s | not %OutputCheck %s 2> %t
; RUN: grep 'Found a match for CheckNotLiteral Directive' %t
; RUN: grep 'in <stdin>:4' %t

; CHECK-L: foo
something foo
; CHECK-NOT-L: boo
; CHECK-NOT-L: foo
something coo
something foo
; CHECK-L: boo^
hello ^ there ^boo^
