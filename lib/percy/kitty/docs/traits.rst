Type traits
===========

A type trait ``is_truth_table<T>`` can be used to check whether type ``T``
implements a truth table type.  The value ``is_truth_table<T>::value``
evaluates to true if ``T`` is ``dynamic_truth_table`` or
``static_truth_table<NumVars>`` (for any ``NumVars``), and ``false``, otherwise.
