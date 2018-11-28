; RUN: sed '/^;/d' %s | %OutputCheck %s
; CHECK-NEXT: foo
; CHECK: bar
; CHECK-NEXT: hundrum
; CHECK-NEXT: life
foo
something
something
bar
hundrum
life
