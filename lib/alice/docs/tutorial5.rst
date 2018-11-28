Tutorial 5: Showing store elements
==================================

We extend the example from :ref:`tutorial4` by functionality that allows to show
store entries for strings and numbers with ``show -s`` and ``show -d``,
respectively.

Showing strings
---------------

For strings, we will use SVG as visualization format, and since we are not
adding any customized user settings, we can simply add the following code to the
example:

.. literalinclude:: ../examples/tutorial5/tutorial5.cpp
    :language: c++
    :lines: 72-81

We are using the :c:macro:`ALICE_SHOW` macro to implement the functionality. The
first argument to the macro is the store type, followed by a default extension.
This extension is used to create temporary filenames, if no filename argument is
provided to ``show``.  The third argument is a reference to an output stream and
the last argument is a reference to the current store element.  In the
implementation, we prepare an SVG string (using C++'s raw string literal), and
insert the element text.

We can now show a string element using the default application as follows::

    tutorial5> number 5; convert --number_to_str; print -s
    five
    tutorial5> show -s

We can also override the default program by explicitly specifying it using the
``--program`` option.  The value should contain a ``{}`` where the filename is
being inserted::

    tutorial5> show -s --program "open -a \"Google Chrome\" {}"

The last command opens the generated SVG file using the Chrome web browser
(``open`` command works only in Mac OS).

Showing numbers
---------------

The number stores should be visualizes as PS file, and the font size should be
customizable by the user.  The macro :c:macro:`ALICE_SHOW` does not allow to add
options to the command; instead, we override the functions `can_show` and `show`
directly:

.. literalinclude:: ../examples/tutorial5/tutorial5.cpp
    :language: c++
    :lines: 83-91

In the implementation of ``can_show``, we first define the default extension to
be ``"ps"``.  Afterwards, we add a command option ``--fontsize``, which can take
an ``unsigned int`` as a value.  We also add it to the option group `Numbers` to
organize the help string for ``show -h``.  Finally, we return ``true`` to enable
the ``show`` command for numbers.  This will add the ``-d`` flag to the command.

.. literalinclude:: ../examples/tutorial5/tutorial5.cpp
    :language: c++
    :lines: 93-109

Similar as to the implementation for string visualization, we first create the
output for the PostScript visualization and leave to placeholders for the font
size and the actual number to print.  The font size is read using the function
``option_value``, which takes as first parameter the same option name that was
given to ``add_option`` and as second parameter a default value.  Note that the
type argument ``unsigned`` must match the type that was used for ``add_option``.

