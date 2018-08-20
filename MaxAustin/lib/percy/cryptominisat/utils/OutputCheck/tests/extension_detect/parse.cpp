// RUN: sed 's/^[ \t]*\/\/[ ]*CHECK.\+$//g' %s | %OutputCheck %s
#include <iostream>

int main()
{
    // CHECK: hello
    std::cout << "hello" << std::endl;
    return 0;
}
