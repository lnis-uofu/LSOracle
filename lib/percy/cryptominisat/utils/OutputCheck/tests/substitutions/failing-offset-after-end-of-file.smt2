; RUN: sed 's/^;[ ]*CHECK.\+$//g' %s | not %OutputCheck -d %s 2> %t
; RUN: grep 'ERROR: ParsingException:' %t
; RUN: grep 'failing-offset-after-end-of-file.smt2:5:8 offset gives line number past the end of file' %t

; CHECK-L: file.c:(${LINE:+2})
file.c:(3)
