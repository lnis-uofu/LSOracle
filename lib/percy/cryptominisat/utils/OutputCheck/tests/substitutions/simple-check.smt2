; RUN: sed 's/^;[ ]*CHECK.\+$//g' %s | %OutputCheck -d %s

; CHECK: file\.c:\(${LINE}\)
file.c:(3)
