; RUN: sed 's/^;[ ]*CHECK.\+$//g' %s | not %OutputCheck -d %s 2> %t
; RUN: grep 'ERROR: ParsingException:' %t
; RUN: grep 'failing-offset-before-first-line.smt2:5:8 offset gives line number < 1' %t

; CHECK-L: file.c:(${LINE:-5})
file.c:(3)
