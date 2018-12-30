// RUN: sed '/\/\//d' %s | not %OutputCheck %s 2> %t
// RUN: grep 'Found a match for CheckNot Directive' %t
// CHECK: Hello World
Hello World
// CHECK-NOT: Testing
Testing
// CHECK: Goodbye
Goodbye
