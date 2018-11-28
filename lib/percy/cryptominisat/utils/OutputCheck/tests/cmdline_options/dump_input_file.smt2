; RUN: %OutputCheck --dump-file-to-check --file-to-check %s %s > %t 2>&1
; RUN: sed -n '2,$p' %t > %t_dump
; RUN: diff -u %s %t_dump
; CHECK: hello
hello
