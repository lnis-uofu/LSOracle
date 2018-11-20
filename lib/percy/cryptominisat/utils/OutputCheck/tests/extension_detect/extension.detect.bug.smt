; RUN: sed 's/^;[ ]*CHECK.\+$//g' %s | %OutputCheck %s
; CHECK: hello
hello
