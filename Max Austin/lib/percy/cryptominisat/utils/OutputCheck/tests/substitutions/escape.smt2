; RUN: sed 's/^;[ ]*CHECK.\+$//g' %s | %OutputCheck -d %s

; CHECK-L: a=\${LINE}
a=${LINE}

; CHECK-L: b=\${LINE:-500}
b=${LINE:-500}
