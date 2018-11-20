OutputCheck
===========

[![Build Status](https://travis-ci.org/stp/OutputCheck.png?branch=master)](https://travis-ci.org/stp/OutputCheck)

OutputCheck is a tool for checking the output of console programs
that is inspired by the FileCheck tool used by LLVM. It has its own
small language (Check Directives) for describing the expected output of 
a tool that is considerably more convenient and more powerful than GNU ``grep``.

This tool was originally written for [STP](http://github.com/stp/stp)
but it ended up being a very useful tool for other projects so it
became its own project!

Obtaining OutputCheck
=====================

OutputCheck can be obtained in multple ways.

Cloning Git repository
----------------------

```
$ git clone https://github.com/stp/OutputCheck.git
```

the tool can now be run as

```
$ cd OutputCheck/
$ bin/OutputCheck --help
```

Installing PyPi package
-----------------------

The tool is [available](https://pypi.python.org/pypi/OutputCheck/) in the [Python package index](https://pypi.python.org/pypi). It can be installed using the ``pip`` tool.

```
$ pip install OutputCheck
```

the tool can now be run as

```
$ OutputCheck --help
```

Please note that this package may not be up to date.

It is recommended that you use [virtualenv](http://www.virtualenv.org/en/latest/) in conjunction with ``pip`` so that you do not need to install python packages as root.

Check Directives
================

Check Directives declare what output is expected from a tool. They are
written as single line comments in a file (this file is usually used
to by the tool being tested by OutputCheck).

The advantage of writing directives in this way is that the directives
can be written next to the code that generates the output that the directive
is checking for.

All directives use the regular expression syntax used by the ``re`` python
module. It is also important to note that the any spaces after the ``:``
until the first non-whitespace character are not considered part of the
regular expression.

The following directives are supported

CHECK: ``<regex>``
------------------

This declares that that regular expression ``<regex>`` should match somewhere
on a single line. This match must occur after previously declared Check directives.

**Succesful example**

``HelloWorld.c``
```C
#include <stdio.h>

int main()
{
    // CHECK: Hello World
    printf("Hello World\n");

    // CHECK: Goodbye
    printf("Goodbye\n");
    
    return 0;
}
```

```
$ cc HelloWorld.c -o HelloWorld
$ ./HelloWorld | OutputCheck HelloWorld.c
```

This example shows a simple ``C`` program being compiled and its output being checked. There are two ``CHECK:`` declarations which effectively say...

1. At least one of the lines must match the regular expression ``Hello World``.
2. At least one line after the previous match must match regular expression ``Goodbye``.

It can be seen that the order the ``CHECK:`` directives are declared is important. If the directives were specified the other way round then the OutputCheck tool would report an error as shown below

**Failing example**

``BrokenHelloWorld.c``
```C
#include <stdio.h>

int main()
{
    // CHECK: Goodbye
    printf("Hello World\n");

    // CHECK: Hello World
    printf("Goodbye\n");
    
    return 0;
}
```

```
$ cc BrokenHelloWorld.c -o BrokenHelloWorld
$ ./BrokenHelloWorld | OutputCheck BrokenHelloWorld.c
ERROR: Could not find a match for Check Directive (BrokenHelloWorld.c:8 Pattern: 'Hello World')
```

CHECK-L: ``<string>``
---------------------

This is the string literal version of the ``CHECK:`` directive. This is identical to the ``CHECK:`` directive except that ``<string>`` is a literal string rather than a regular expression. This is useful if using python's regular expression syntax is too cumbersome.

For example

```
// CHECK: foo\.c\(5\) '\*' is not a valid identifier\.
// CHECK-L: foo.c(5) '*' is not a valid identifier.
```

the above directives are equivalent but the ``CHECK-L:`` is much easier to write.

CHECK-NEXT: ``<regex>``
-----------------------

This declares that the next line after the previous match must match the regular expression ``<regex>``. If there was no previous directive then ``CHECK-NEXT:`` matches the first line of the tool's output.

**Succesful example**

``HelloWorld2.c``
```C
#include <stdio.h>

int main()
{
    // CHECK: Hello World
    // CHECK-NEXT: Goodbye
    printf("Hello World\nGoodbye");

    return 0;
}
```

```
$ cc HelloWorld2.c -o HelloWorld2
$ ./HelloWorld2 | OutputCheck HelloWorld2.c
```

**Failing example**

``BrokenHelloWorld2.c``
```C
#include <stdio.h>

int main()
{
    // CHECK: Hello World
    
    printf("Hello World\n");
    printf("Testing...\n");
    
    // CHECK-NEXT: Goodbye
    printf("Goodbye\n");

    return 0;
}
```

```
$ cc BrokenHelloWorld2.c -o BrokenHelloWorld2
$ ./BrokenHelloWorld2 | OutputCheck BrokenHelloWorld2.c
ERROR: Could not find a match for CheckNext Directive (BrokenHelloWorld2.c:10 Pattern: 'Goodbye') expected at <stdin>:2
```

CHECK-NEXT-L: ``<string>``
--------------------------

This is the string literal version of the ``CHECK-NEXT:`` directive. This is identical to the ``CHECK-NEXT:`` directive except that ``<string>`` is a literal string rather than a regular expression. This is useful if using python's regular expression syntax is too cumbersome.

CHECK-NOT: ``<regex>``
----------------------

This declares that between the previous match (if there is none, search starts from the first line of tool's output) and the next match (if there is none the search will search to the end of the tool's output) that no line will match the regular expression ``<regex>``.

**Succesful example**

``HelloWorld3.c``
```C
#include <stdio.h>

int main()
{
    // CHECK: Hello World
    // CHECK-NOT: Testing
    // CHECK: Goodbye
    printf("Hello World\nGoodbye");

    return 0;
}
```

```
$ cc HelloWorld3.c -o HelloWorld3
$ ./HelloWorld3 | OutputCheck HelloWorld3.c
```

**Failing example**

``BrokenHelloWorld3.c``
```C
#include <stdio.h>

int main()
{
    // CHECK: Hello World
    printf("Hello World\n");
    
    // CHECK-NOT: Testing
    // CHECK: Goodbye
    printf("Testing...\n");
    printf("Goodbye\n");

    return 0;
}
```

```
$ cc BrokenHelloWorld3.c -o BrokenHelloWorld3
$ ./BrokenHelloWorld3 | OutputCheck BrokenHelloWorld3.c
ERROR: Found a match for CheckNot Directive (BrokenHelloWorld3.c:8 : Pattern: 'Testing') in <stdin>:2
```

CHECK-NOT-L: ``<string>``
-------------------------

This is the string literal version of the ``CHECK-NOT:`` directive. This is identical to the ``CHECK-NOT:`` directive except that ``<string>`` is a literal string rather than a regular expression. This is useful if using python's regular expression syntax is too cumbersome.

Substitutions
=============

For convenience several substitutions are provided for use in ``OutputCheck``. These substitutions can used in ``<regex>`` or ``<string>`` for all the check directives. All directives can be escaped by prepending ``\``.

``${LINE}``
-----------

This is substituted by the line number that the check directive is on in the check file.

```
// CHECK-L : file.c(${LINE})
```

``${LINE:+N}``
--------------

This is substituted by the line number that the check directive is on in the check file plus an offset (``N``).


```
// CHECK-L : file.c(${LINE:+5})
```

``${LINE:-N}``
--------------

This is substituted by the line number that the check directive is on in the check file minus an offset (``N``).


```
// CHECK-L : file.c(${LINE:-5})
```

``${CHECKFILE_NAME}``
---------------------

This is substituted by the name of the check file.

```
// CHECK-L : ${CHECKFILE_NAME}(5)
```

``${CHECKFILE_ABS_PATH}``
---------------------

This is substituted by the absolute path to check file.

```
// CHECK-L : ${CHECKFILE_ABS_PATH}(5)
```

Tests
=====

A small set of tests are present in the ``tests/`` directory. These tests are designed to be driven using ``llvm-lit`` from LLVM >=3.4 . These tests are not cross platform and will only work on systems with the ``sed`` and ``grep`` programs. It should be noted that the OutputCheck tool is implemented purely in python so the tool should work on platforms that support Python.
