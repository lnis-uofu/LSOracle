Change Log
==========

v0.5 (not yet released)
-----------------------

* Cube methods: ``difference`` (contributed by Heinz Riener)
  `#52 <https://github.com/msoeken/kitty/pull/52>`_

* Type traits: ``is_truth_table``
  `#53 <https://github.com/msoeken/kitty/pull/53>`_

* Assignment operator for truth tables
  `#54 <https://github.com/msoeken/kitty/pull/54>`_

v0.4 (May 4, 2018)
------------------

* Constructors: ``create``, ``create_from_clauses``, ``create_characteristic``, ``create_multiple_from_chain``
  `#41 <https://github.com/msoeken/kitty/pull/41>`_
  `#43 <https://github.com/msoeken/kitty/pull/43>`_
  `#45 <https://github.com/msoeken/kitty/pull/45>`_

* Operations: ``shrink_to`` and ``shrink_to_inplace``
  `#42 <https://github.com/msoeken/kitty/pull/42>`_
  `#47 <https://github.com/msoeken/kitty/pull/47>`_

* Operation ``extend_to`` is now called ``extend_to_inplace``, new special out-of-place versions ``extend_to``
  `#47 <https://github.com/msoeken/kitty/pull/47>`_

* CNF generation: ``cnf_characteristic``
  `#41 <https://github.com/msoeken/kitty/pull/41>`_

* Print all cubes: ``print_cubes``
  `#44 <https://github.com/msoeken/kitty/pull/44>`_

* Generate implicants and prime implicants: ``get_minterms``, ``get_jbuddies``, ``get_prime_implicants_morreale``
  `#46 <https://github.com/msoeken/kitty/pull/46>`_

* Function properties: ``chow_parameters``, ``is_canalizing``, ``is_horn``, ``is_krom``
  `#48 <https://github.com/msoeken/kitty/pull/48>`_

* Spectral canonization with resource limit
  `#49 <https://github.com/msoeken/kitty/pull/49>`_

* Compute different spectra: ``rademacher_walsh_spectrum``, ``autocorrelation_spectrum``, ``spectrum_distribution``
  `#49 <https://github.com/msoeken/kitty/pull/49>`_

* Compute spectral equivalence class index: ``get_spectral_class``
  `#50 <https://github.com/msoeken/kitty/pull/50>`_

v0.3 (February 25, 2018)
------------------------

* Constructors: ``create_from_chain``, ``create_symmetric``
  `#28 <https://github.com/msoeken/kitty/pull/28>`_
  `#35 <https://github.com/msoeken/kitty/pull/35>`_

* Cube constructors: ``pos_cube``, ``neg_cube``, constructor to create cube from string
  `#29 <https://github.com/msoeken/kitty/pull/29>`_
  `#31 <https://github.com/msoeken/kitty/pull/31>`_

* Cube methods: ``get_bit``, ``get_mask``, ``set_bit``, ``set_mask``, ``clear_bit``, ``clear_mask`` (contributed by Heinz Riener)
  `#30 <https://github.com/msoeken/kitty/pull/30>`_

* Operations: ``shift_left``, ``shift_left_inplace``, ``shift_right``, ``shift_right_inplace``
  `#37 <https://github.com/msoeken/kitty/pull/37>`_
  `#39 <https://github.com/msoeken/kitty/pull/39>`_

* Operators: ``!=``, ``<<``, ``<<=``, ``>>``, ``>>=``
  `#32 <https://github.com/msoeken/kitty/pull/32>`_
  `#37 <https://github.com/msoeken/kitty/pull/37>`_
  `#39 <https://github.com/msoeken/kitty/pull/39>`_

* Return truth table as string: ``to_binary``, ``to_hex``
  `#33 <https://github.com/msoeken/kitty/pull/33>`_

* Default comparison for cubes (contributed by Heinz Riener)
  `#34 <https://github.com/msoeken/kitty/pull/34>`_

* Linear and affine canonization: ``exact_linear_canonization``, ``exact_linear_output_canonization``, ``exact_affine_canonization``, ``exact_affine_output_canonization``
  `#36 <https://github.com/msoeken/kitty/pull/36>`_

* Compute PPRM ESOP for truth table: ``esop_from_pprm``
  `#38 <https://github.com/msoeken/kitty/pull/38>`_

* Compute permutation masks and delta-swap operations: ``delta_swap_inplace``, ``delta_swap``, ``permute_with_masks_inplace``, ``permute_with_masks``, ``compute_permutation_masks``, 
  `#40 <https://github.com/msoeken/kitty/pull/40>`_

v0.2 (December 21, 2017)
------------------------

* Generic algorithms: ``for_each_block``, ``for_each_block_reversed``, ``for_each_one_bit``
  `#14 <https://github.com/msoeken/kitty/pull/14>`_
  `#16 <https://github.com/msoeken/kitty/pull/16>`_

* Constructors: ``create_from_words``, ``create_from_raw``, ``create_threshold``, ``create_equals``
  `#15 <https://github.com/msoeken/kitty/pull/15>`_
  `#20 <https://github.com/msoeken/kitty/pull/20>`_
  `#24 <https://github.com/msoeken/kitty/pull/24>`_
  `#25 <https://github.com/msoeken/kitty/pull/25>`_

* Moved generic operations from `operations.hpp` to `algorithm.hpp`
  `#14 <https://github.com/msoeken/kitty/pull/14>`_

* Print truth tables as string: ``print_binary``, ``print_raw``, ``print_hex``
  `#14 <https://github.com/msoeken/kitty/pull/14>`_
  `#20 <https://github.com/msoeken/kitty/pull/20>`_
  `#22 <https://github.com/msoeken/kitty/pull/22>`_

* Added iterators to all truth tables: ``begin``, ``end``, ``rbegin``, ``rend``, ``cbegin``, ``cend``, ``crbegin``, ``crend``
  `#14 <https://github.com/msoeken/kitty/pull/14>`_

* Count number of ones in truth table: ``count_ones``
  `#21 <https://github.com/msoeken/kitty/pull/21>`_

* Find bits: ``find_first_one_bit``, ``find_last_one_bit``, ``find_first_bit_difference``, ``find_last_bit_difference``
  `#17 <https://github.com/msoeken/kitty/pull/17>`_

* Extend smaller truth tables to larger ones: ``extend_to``
  `#26 <https://github.com/msoeken/kitty/pull/26>`_

* Compute ESOP expressions: ``esop_from_optimum_pkrm``
  `#23 <https://github.com/msoeken/kitty/pull/23>`_

* New data structure for cubes used by ISOP and ESOP computation: ``cube``
  `#23 <https://github.com/msoeken/kitty/pull/23>`_

* NPN canonization heuristics: ``flip_swap_npn_canonization``, ``sifting_npn_canonization``
  `#27 <https://github.com/msoeken/kitty/pull/27>`_

* Spectral canonization: ``exact_spectral_canonization``, ``print_spectrum``
  `#19 <https://github.com/msoeken/kitty/pull/19>`_

v0.1 (October 22, 2017)
-----------------------

* Initial release
* Data structures ``static_truth_table`` and ``dynamic_truth_table``
  `#1 <https://github.com/msoeken/kitty/pull/1>`_

* Bit functions: ``set_bit``, ``get_bit``, ``clear_bit``, ``clear``
  `#1 <https://github.com/msoeken/kitty/pull/1>`_
  `#8 <https://github.com/msoeken/kitty/pull/8>`_

* Constructors: ``create_nth_var``, ``create_from_binary_string``, ``create_from_hex_string``, ``create_random``, ``create_from_cubes``, and ``create_majority``
  `#1 <https://github.com/msoeken/kitty/pull/1>`_
  `#4 <https://github.com/msoeken/kitty/pull/4>`_
  `#5 <https://github.com/msoeken/kitty/pull/5>`_
  `#9 <https://github.com/msoeken/kitty/pull/9>`_
  `#11 <https://github.com/msoeken/kitty/pull/11>`_

* Unary and binary operations: ``unary_not``, ``unary_not_if``, ``binary_and``, ``binary_or``, and ``binary_xor``
  `#2 <https://github.com/msoeken/kitty/pull/2>`_
  `#8 <https://github.com/msoeken/kitty/pull/8>`_

* Ternary operations: ``ternary_majority`` and ``ternary_ite``
  `#3 <https://github.com/msoeken/kitty/pull/3>`_

* Binary predicates: ``equal``, ``less_than``
  `#4 <https://github.com/msoeken/kitty/pull/4>`_
  `#8 <https://github.com/msoeken/kitty/pull/8>`_

* Predicates: ``has_var``, ``is_const0``
  `#11 <https://github.com/msoeken/kitty/pull/11>`_

* Operators: ``~``, ``&``, ``&=``, ``|``, ``|=``, ``^``, ``^=``, ``==``, ``<``
  `#8 <https://github.com/msoeken/kitty/pull/8>`_
  `#11 <https://github.com/msoeken/kitty/pull/11>`_

* Swap adjacent variables: ``swap_adjacent_inplace``, ``swap_adjacent``
  `#6 <https://github.com/msoeken/kitty/pull/6>`_

* Swap variables: ``swap_inplace``, ``swap``
  `#8 <https://github.com/msoeken/kitty/pull/8>`_

* Flip variable: ``flip_inplace``, ``flip``
  `#7 <https://github.com/msoeken/kitty/pull/7>`_

* Enumerate truth tables: ``next_inplace``, ``next``
  `#10 <https://github.com/msoeken/kitty/pull/10>`_

* Compute co-factors: ``cofactor0_inplace``, ``cofactor0``, ``cofactor1_inplace``, ``cofactor1``
  `#11 <https://github.com/msoeken/kitty/pull/11>`_

* Compute minimum base based on functional support: ``min_base_inplace``, ``expand_inplace``
  `#12 <https://github.com/msoeken/kitty/pull/12>`_

* Compute hash values for truth table
  `#13 <https://github.com/msoeken/kitty/pull/13>`_

* NPN canonization: ``exact_npn_canonization``, ``create_from_npn_config``
  `#8 <https://github.com/msoeken/kitty/pull/8>`_

* Compute ISOP representation: ``isop``
  `#11 <https://github.com/msoeken/kitty/pull/11>`_
