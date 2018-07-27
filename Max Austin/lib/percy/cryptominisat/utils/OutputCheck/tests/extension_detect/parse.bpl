// RUN: sed 's/^[ \t]*\/\/[ ]*CHECK.\+$//g' %s | %OutputCheck %s
procedure main()
{
    // CHECK: var a:bv8
    var a:bv8;
}
