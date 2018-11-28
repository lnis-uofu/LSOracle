;  RUN: sed 's/^[ \t]*;[ ]*FOO.\+$//g' %s | %OutputCheck --check-prefix=FOO %s
; FOO: hello
hello
; FOO-NOT: bob
