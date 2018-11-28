;  RUN: sed 's/^[ \t]*;[ ]*CHECK.\+$//g' %s | %OutputCheck %s

; CHECK: hello
; FIXME: add real smt query
hello
