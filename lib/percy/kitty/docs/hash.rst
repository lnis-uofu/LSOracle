Hash values
===========

The header ``<kitty/hash.hpp>`` implements hash functions for truth
tables.  Given some truth table of some type ``TT``, one can use
``kitty::hash<TT>`` as a hash function, e.g., for
``std::unordered_map`` and ``std::unordered_set``.

.. doc_brief_table::
   hash_block
   hash_combine

