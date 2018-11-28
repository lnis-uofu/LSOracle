Tutorial 3: Read and write commands
===================================

This tutorial shows how to integrate read and write functions to the shell
interface.  The general mechanism works as follows.  We first define a file
type, and name it using a tag.  Then we can enable read and write commands for
this tag to a store type, which result in commands ``read_<tag> --<flag>`` and
``write_<tag> --<flag>``, where ``<tag>`` is the tag name of the file type and
``<flag>`` is the flag name of the store type.

Our tutorial catches up where we left in the previous tutorial and defines a
single store type for strings, and adds a function to print store elements.

.. literalinclude:: ../examples/tutorial3/tutorial3.cpp
    :language: c++
    :lines: 26-42

We now add the file type to read and write to text files using the macro
:c:macro:`ALICE_ADD_FILE_TYPE`, which receives the tag name as first argument
and a string defining it as second argument.  The second argument will be used
to generate help strings.

.. literalinclude:: ../examples/tutorial3/tutorial3.cpp
    :language: c++
    :lines: 44

If we wish to only read from or only write to a file type, we can use the macros
:c:macro:`ALICE_ADD_FILE_TYPE_READ_ONLY` or
:c:macro:`ALICE_ADD_FILE_TYPE_WRITE_ONLY`, respectively.  They have the same
signature.

Once the file type is declared, we can link the string store type to the file
type and add functions to read from files and to write from files.  Reading from
files is enabled using the macro :c:macro:`ALICE_READ_FILE`, which receives four
parameters.  The first two parameters are store type and tag name.  The third
parameter is the variable name containing the filename, and the last parameter
gives access to the command parsing interface, which we won't use in this
tutorial.

.. literalinclude:: ../examples/tutorial3/tutorial3.cpp
    :language: c++
    :lines: 46-52

Similarly, we enable writing from files using the macro
:c:macro:`ALICE_WRITE_FILE`. It receives one further parameter called
``element``, which is a variable accessing the current store element that should
be written to a file.

.. literalinclude:: ../examples/tutorial3/tutorial3.cpp
    :language: c++
    :lines: 54-58

That's all we need to read and write from files.  Finally, we add one further
command to manipulate store entries.  The command ``upper`` will allow to change
string elements into upper case.

.. literalinclude:: ../examples/tutorial3/tutorial3.cpp
    :language: c++
    :lines: 60-68
