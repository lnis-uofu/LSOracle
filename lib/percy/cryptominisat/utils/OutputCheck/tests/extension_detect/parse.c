// RUN: sed 's/^[ \t]*\/\/[ ]*CHECK.\+$//g' %s | %OutputCheck %s
#include <stdio.h>

int main()
{
    // CHECK: hello
    printf("hello\n");
    return 0;
}
