; RUN: sed 's/^[ \t]*;[ ]*CHECK.\+$//g' %s | %OutputCheck --disable-substitutions -d %s
; CHECK-L: ${LINE}
${LINE}
