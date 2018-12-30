Introduction
============

The C++ library `kitty` provides data structures and algorithms for
truth table manipulation.  A truth table is represented in terms of
64-bit words, where each bit in the word represents a function value.
For example, the truth table for the function :math:`x_0 \land x_1` is
``0x8`` (which is ``1000`` in base 2) and the truth table for the
function :math:`\langle x_0x_1x_2\rangle` is ``0xe8`` (which is
``1110100`` in base 2).  A single 64-bit word can represent functions
with up to 6 variables.  To store functions with 7 variables, one
needs two words, for 8 variables one needs 4 words, and so on.

Two main data structures are provided in `kitty` to represent truth
tables: ``static_truth_table`` and ``dynamic_truth_table``.  The
choice to take which depends on whether one knows the number of
variables at compile-time or not.  The structure
``static_truth_table`` is parameterized by a template argument for the
number of variables, ``dynamic_truth_table`` retrieves this
information as constructor argument.

