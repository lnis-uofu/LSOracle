; RUN: sed 's/^;[ ]*CHECK.\+$//g' %s | not %OutputCheck %s 2> %t
; RUN: grep 'Could not find a match for CheckNext Directive' %t
; RUN: grep 'expected at <stdin>:7' %t
; CHECK: foo
; CHECK-NEXT: bar
foo
okay
bar
