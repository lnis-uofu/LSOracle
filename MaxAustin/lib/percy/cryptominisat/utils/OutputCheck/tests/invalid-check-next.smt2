; RUN: sed 's/^;[ ]*CHECK.\+$//g' %s | not %OutputCheck %s 2> %t
; RUN: grep -E 'ParsingException: (CheckNext|CheckNot) Directive' %t
; CHECK-NOT: foo
; CHECK-NEXT: bar
humdrum
bar
