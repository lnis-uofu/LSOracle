;  RUN: sed 's/^[ \t]*;[ ]*CHECK.\+$//g' %s > %t
;  RUN: %OutputCheck --file-to-check=%t %s
; CHECK: hello
hello
