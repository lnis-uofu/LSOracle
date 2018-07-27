// RUN: sed '/\/\//d' %s | %OutputCheck %s

// CHECK: ^foobar$
// CHECK-NEXT: 0xfff$
foobar
0xfff

// CHECK-NOT: ^bar$
barED
