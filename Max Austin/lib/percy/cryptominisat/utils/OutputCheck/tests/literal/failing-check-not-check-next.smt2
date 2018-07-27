; RUN: sed 's/^;[ ]*CHECK.\+$//g' %s | not %OutputCheck %s 2> %t
; RUN: grep 'ERROR: ParsingException: CheckNotLiteral Directive' %t
; RUN: grep 'must have a CHECK: or CHECK-L: directive after it instead of a CheckNext Directive' %t

; CHECK-NOT-L: foo
something boo
; CHECK-NEXT: boo(5)
something boo(5) there goes
