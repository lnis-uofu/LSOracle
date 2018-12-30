Console, Pyton, and C interface
===============================

Alice supports different usage modes.  By default the code is compiled to a
stand-alone executable.  But the same source code can also be compiled into a
Python module or C library.

- **Console mode:** the source code is compiled as **executable** and a
  **stand-alone program** is offering a console shell interface, which accepts
  commands.

- **Python mode:** the source code is compiled as **Python module** which offers
  an API according to the commands.  Commands become function names, and command
  arguments become function arguments.

- **C library mode:** the source code is compiled as **C library** with
  functions call commands.  This is useful to interface with Alice CLIs from other
  languages.

Which mode is taken is determined by compilation.  No changes in the source code
are necessary, when making use of the :ref:`macroapi`.

Let's say we have a source file `shell.cpp`, which defines an alice CLI as
explained in the tutorials of this manual.  Then add the following lines into a
CMakeLists.txt file in order to compile it as an executable in console mode::

    add_executable(shell shell.cpp)
    target_link_libraries(shell alice)

The same file can be compiled as a Python module with the following command::

    add_alice_python_module(shell shell.cpp)

This creates a compile target ``shell_python``.

In order to compile it as C library one uses the following commands::

    add_alice_c_library(shell shell.cpp)

This creates a compile target ``shell_c``.

.. note::

    The name of the Python module *must* equal the name of the prefix that was
    used in the :c:macro:`ALICE_MAIN` macro.  Our example file shell.cpp must
    finish with ``ALICE_MAIN(shell)``.

Shell commands as Python functions
----------------------------------

If the alice shell has the prefix ``shell``, then the corresponding Python
module has the name `shell` and can be imported as follows:

.. code-block:: python

   import shell

Commands are mapped into Python functions with the same name.  Assume there is a
command called ``command``, then one can call it from Python as follows:

.. code-block:: python

   import shell
   shell.command()

Long option and flag names are mapped into keyword arguments of the
corresponding Python command.  Assume that the command ``command`` has the
following synopsis::

    shell> command -h
    A test command
    Usage: command [OPTIONS]

    Options:
      -h,--help                   Print this help message and exit
      -s,--sopt TEXT              A string option
      -n,--nopt INT               An integer option
      -f,--flag                   A flag

Then the individual arguments in this command can be called in Python mode as
follows:

.. code-block:: python

   import shell
   shell.command(sopt = "Some text", nopt = 42, flag = True)

The order in which the keyword arguments are passed does not matter; also, not
all of them need to be provided.  Note again, that the short option and flag
names cannot be used in Python mode.  Also flags must be assigned a Boolean
value.  Assigning ``False`` to a flag argument is as omitting it.

The return value of a Python function corresponds to the logging output of the
corresponding command.  Each command can contribute to the log by implementing
the ``log()`` function.  It returns a JSON object.  The return value of the
function in Python mode can be considered as a Python ``dict``, in which the
entries correspond to the JSON object.

Assume that the example command ``command`` implements the following ``log()``
function:

.. code-block:: c++

   nlohmann::json log() const
   {
     return nlohmann::json({
       {"str", "Some string"},
       {"number", 42}
     });
   }

Then one can access these values from the return value of the Python function:

.. code-block:: python

   import shell
   r = shell.command()
   print(r["number"])    # Prints 42

C library
---------

Assuming that the alice shell has the prefix ``shell``, then the C library will
implement the following three functions:

.. code-block:: c

   extern void* shell_create();
   extern void shell_delete( void* cli );
   extern int shell_command( void* cli, const char* command, char* log, size_t size );

The prefix is being used as prefix for the C functions.  By copying the above
three lines into a C file and linking to the compiled C library allows to
interact with the alice CLI shell.

The first two functions ``shell_create`` and ``shell_delete`` create and
delete a CLI object.  Note that the object is passed as ``void*``.  The third
function calls a single command.  The first argument is a pointer to a CLI
object and the second argument is the command as string.  The third argument is
a string pointer which can be passed to store the JSON log produced by the
command; it can also be null.  If not null, the last argument should contain
the maximum size of the ``log`` string.  The function returns -1, if the command
was not executed successfully, 0, if the command was executed successfully, but
nothing was written into the ``log`` string, and otherwise the actual size of
the JSON string.  The actual size may be longer than ``size``.

Being a C library, it can also be used in other languages, e.g., in C#.  In
the next example, we assume that the library has been compiled on a Linux
machine and has the name ``libshell_c.so``:

.. code-block:: c#

   using System;
   using System.Runtime.InteropServices;
   using System.Text;
   
   public class Library {
     [DllImport("libshell_c.dylib", EntryPoint = "shell_create")]
     public static extern IntPtr shell_create();
   
     [DllImport("libshell_c.dylib", EntryPoint = "shell_delete")]
     public static extern void shell_delete(IntPtr cli);
   
     [DllImport("libshell_c.dylib", EntryPoint = "shell_command")]
     public static extern int shell_command(IntPtr cli, string command, StringBuilder json, int size);
   }
