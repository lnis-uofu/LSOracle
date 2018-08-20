Store API
=========

.. doc_overview_table:: namespacealice
   :column: Function

   to_string
   print
   print_statistics
   log_statistics
   can_read
   read
   can_write
   write
   can_convert
   convert
   can_show
   show
   has_html_repr
   html_repr

Declaring a new store type
--------------------------

.. doxygenstruct:: alice::store_info

Customizing store functions
---------------------------

.. doxygenfunction:: alice::to_string
.. doxygenfunction:: alice::print
.. doxygenfunction:: alice::print_statistics
.. doxygenfunction:: alice::log_statistics
.. doxygenfunction:: alice::can_read
.. doxygenfunction:: alice::read
.. doxygenfunction:: alice::can_write
.. doxygenfunction:: alice::write(StoreType const&, const std::string&, const command&)
.. doxygenfunction:: alice::write(StoreType const&, std::ostream&, const command&)
.. doxygenfunction:: alice::can_convert
.. doxygenfunction:: alice::convert
.. doxygenfunction:: alice::can_show
.. doxygenfunction:: alice::show
.. doxygenfunction:: alice::has_html_repr
.. doxygenfunction:: alice::html_repr
