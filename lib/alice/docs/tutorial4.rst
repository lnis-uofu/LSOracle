.. _tutorial4:

Tutorial 4: Two stores and conversion
=====================================

We are now describing an example in which we use two store elements, one for
strings and for for integers, and we add the possibility to convert an element
from the integer store into an element from the string store.

We start by defining two stores and also add methods to print the store entries
using ``print``.  Using the macro :c:macro:`ALICE_DESCRIBE_STORE` we can return
a short description string that is used to summarize a store element when
showing the store content with ``store``.

.. literalinclude:: ../examples/tutorial4/tutorial4.cpp
    :language: c++
    :lines: 26-56

Using the macro :c:macro:`ALICE_CONVERT` we can enable conversion from entry to
another, which is performed by the convert command.  We plan to write a
conversion routine from integers (flag name ``"number"``) to strings (flag name
``"string"``).  By implementing the following macro, we will add a flag
``--number_to_string`` to the ``convert`` command.

.. literalinclude:: ../examples/tutorial4/tutorial4.cpp
    :language: c++
    :lines: 58-70

The macro :c:macro:`ALICE_CONVERT` expects three arguments.  The first argument
is the store type that we wish to convert from, followed by an identifier that
we use to access the current element from that store in the implementation of
the macro.  The third argument is the store type that we wish to convert to.
The code will convert all numbers between 0 and 9 to respective strings, and
otherwise to ``"many"``.

Next, we implement a command ``number`` to load a number into the store.  We
wish to specify the number using a command option ``--load``.  Since we need to
initialize command arguments in the constructor, we cannot use the
:c:macro:`ALICE_COMMAND` macro but need to implement our own class by deriving
from the ``alice::command`` base class.  Make sure to use the ``_command``
suffix when giving a name to the new command.  We can add the command using the
:c:macro:`ALICE_ADD_COMMAND` after defining the command. The second argument to
this macro is a category that is used by the ``help`` command.

.. literalinclude:: ../examples/tutorial4/tutorial4.cpp
    :language: c++
    :lines: 72-92

Finally, we call the main macro for alice.

.. literalinclude:: ../examples/tutorial4/tutorial4.cpp
    :language: c++
    :lines: 94