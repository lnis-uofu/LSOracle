; RUN: sed 's/^;[ ]*CHECK.\+$//g' %s | %OutputCheck %s

; CHECK: foo
something foo
; CHECK: boo
something boo
