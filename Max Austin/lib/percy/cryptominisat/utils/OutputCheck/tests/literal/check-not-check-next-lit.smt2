; RUN: sed '/^;/d' %s | not %OutputCheck %s 2> %t
; RUN: grep 'ParsingException: CheckNot Directive' %t
; RUN: grep 'must have a CHECK: or CHECK-L: directive after it instead of a CheckNextLiteral Directive' %t
; CHECK-NOT: foo
; CHECK-NEXT-L: boo
something foo
boo
