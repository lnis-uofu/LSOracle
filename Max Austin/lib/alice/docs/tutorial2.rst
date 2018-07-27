Tutorial 2: Adding a store and writing a simple command
=======================================================

We extend on the previous example and add a store to the shell.  A shell can
have several stores, each is indexed by its type.

.. literalinclude:: ../examples/tutorial2/tutorial2.cpp
    :language: c++
    :lines: 26-47

The macro :c:macro:`ALICE_ADD_STORE` registers a store for strings (using type
``std::string``).  The type is the first argument to the macro.  The other four
are used to build commands.  The values ``str`` and ``s`` are long and short
flag names, respectively, and will be used to select this type in several
store-related commands, e.g., ``print --str`` or ``print -s`` to print a string
to the terminal.  The last two arguments are a singular and plural name that is
used to generate help strings in store-related commands.  Let's have a look what
``help`` shows for this tutorial::

    tutorial2> help
    Generation commands:
     hello
    
    General commands:
     alias            convert          current          help
     print            ps               quit             select
     show             store

First, we see two categories of commands, the first one (`Generation commands`)
listing the custom command ``hello``.  We'll get to that one in a bit.  There
are also several other general commands compared to the previous tutorial.
These are called store-related commands are as follows:

+-------------+------------------------------------------------------------------------+
| ``convert`` | Converts a store element of one type into another                      |
+-------------+------------------------------------------------------------------------+
| ``current`` | Changes the current store element                                      |
+-------------+------------------------------------------------------------------------+
| ``print``   | Prints the current store element                                       |
+-------------+------------------------------------------------------------------------+
| ``ps``      | Prints statistics about the current store element                      |
+-------------+------------------------------------------------------------------------+
| ``show``    | Creates and shows a visual representation of the current store element |
+-------------+------------------------------------------------------------------------+
| ``store``   | Shows a summary of store elements                                      |
+-------------+------------------------------------------------------------------------+

In each command the type of store must be addressed by the flag name that was
defined for the store in :c:macro:`ALICE_ADD_STORE`.  For example, ``print -s``
prints the current element from the string store to the terminal.  The code
provided by the :c:macro:`ALICE_PRINT_STORE` macro is used to describe what
should be printed for the specific store type.  In case of this string store, we
just print the string followed by a new line.

One new command is added using the macro :c:macro:`ALICE_COMMAND`.  This macro
only allows us to add very simple commands, with no custom arguments, and no
custom logging behavior (we will see how to create more advanced commands in the
next tutorials).  The command ``hello`` is defined using two other arguments to
the macro, the second being a category used to partition commands when calling
``help``, the third being a description text that is printed when calling
``hello -h`` and is used by ``help -s`` to find commands.  The code accesses the
store of strings, using ``store<std::string>()`` and extends it by one element
using the method ``extend()``.  Since ``extend()`` returns a reference to the
newly created element, we can assign it the value ``"hello world"``.  Let's have
a look at a session that makes use of the new command::

    tutorial2> store -s
    [i] no strings in store
    tutorial2> hello
    tutorial2> store -s
    [i] strings in store:
      *  0:
    tutorial2> print -s
    hello world
    tutorial2> quit
