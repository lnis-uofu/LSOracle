; RUN: sed '/^;[ ]*CHECK.\+$/d' %s | not %OutputCheck %s 2> %t
; RUN: grep -E 'Could not find a match for CheckNextLiteral Directive' %t
; RUN: grep -E "failing-simple-check-next-lit.smt2:7 Literal: 'bo\$\$'" %t

; CHECK-L: foo
something foo
; CHECK-NEXT-L: bo$$
something bo$
; CHECK-NOT: xxx
xxyxx
