;  RUN: sed 's/^[ \t]*;[ ]*CHECK.\+$//g' %s | not %OutputCheck --comment=// %s 2> %t
;  RUN: grep 'does not contain any CHECK directives' %t
; CHECK: hello
hello
