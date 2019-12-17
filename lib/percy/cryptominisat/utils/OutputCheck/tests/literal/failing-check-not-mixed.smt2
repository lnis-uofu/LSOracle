; RUN: sed 's/^;[ ]*CHECK.\+$//g' %s | not %OutputCheck %s 2> %t
; RUN: grep 'ERROR: ParsingException: CheckNot Directive' %t
; RUN: grep 'must have a CHECK: or CHECK-L: directive after it instead of a CheckNotLiteral' %t

; We normally merge NOT directives but we can't merge Literal and Regex directives
; Perhaps we could fix this by using some sort of promotion of Literal CHECK-NOTs to Regex CHECK-NOTs
; CHECK-NOT: foo
something boo
; CHECK-NOT-L: boo(5)
something boo(5) there goes
