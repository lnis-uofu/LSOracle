; RUN: sed 's/^;[ ]*CHECK.\+$//g' %s | not %OutputCheck %s 2> %t
; RUN: grep 'ParsingException: Failed to parse regular expression' %t
; RUN: grep 'unbalanced parenthesis' %t
; CHECK: slow(ing?
