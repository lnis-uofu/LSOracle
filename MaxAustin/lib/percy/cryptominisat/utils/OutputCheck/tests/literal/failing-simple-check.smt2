; RUN: sed 's/^;[ ]*CHECK.\+$//g' %s | not %OutputCheck %s 2> %t
; RUN: grep 'Could not find a match for CheckLiteral Directive' %t
; RUN: grep "failing-simple-check.smt2:9 Literal: 'boo\^'" %t

; CHECK-L: foo
something foo
; CHECK-L: boo(5)
something boo(5) there goes
; CHECK-L: boo^
hello ^ there ^booxy^
