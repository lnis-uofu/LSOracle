/* alice: C++ command shell library
 * Copyright (C) 2017-2018  EPFL
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

/*
  \file store_api.hpp
  \brief API to customize behavior of store elements

  \author Mathias Soeken
*/

#pragma once

#include <exception>
#include <iostream>
#include <string>

#include <nlohmann/json.hpp>

#include "command.hpp"

namespace alice
{

class command;

/*! \brief Empty prototype class for store information

  You need to specialize this struct in order declare a new store type for the
  CLI.  In this specialization five ``static constexpr const char*`` variables
  must be defined:

  - ``key``: A unique key for internal storing in the ``environment``
  - ``option``: A long option name for commands (without dashes)
  - ``mnemonic``: A single character for short option (without dash, cannot be ``n`` or ``v``)
  - ``name``: A singular name that is used in help texts
  - ``name_plural``: A plural name that is used in help texts

  \verbatim embed:rst
      
      .. note::
      
        Make sure to specialize ``store_info`` inside the ``alice`` namespace.  You
        can use the :c:macro:`ALICE_ADD_STORE` macro instead of the partial
        specialization.  Also :c:macro:`ALICE_MAIN` will automatically pick up all
        stores that were defined using :c:macro:`ALICE_ADD_STORE`.

      Here is an example code to define a store type for a fictional type ``graph``:

      .. code-block:: c++
      
         namespace alice {
        
         template<>
         struct store_info<graph>
         {
           static constexpr const char* key = "graph";
           static constexpr const char* option = "graph";
           static constexpr const char* mnemonic = "g";
           static constexpr const char* name = "graph";
           static constexpr const char* plural_name = "graphs";
         };

         }

      You can then use this data structure as part of the CLI when listing its type in the
      declaration of ``cli``:

      .. code-block:: c++

         alice::cli<..., graph, ...> cli( "prefix" );

  \endverbatim
 */
template<typename StoreType>
struct store_info
{
};

/*! \brief Produce short one-line description of store element

  \verbatim embed:rst
      You can use :c:macro:`ALICE_DESCRIBE_STORE` to implement this function.
  \endverbatim

  \pram element Store element
*/
template<typename StoreType>
std::string to_string( StoreType const& element )
{
  (void)element;
  return "";
}

/*! \brief Routine to print a store element to an output stream

  \verbatim embed:rst
      This routine is called by the `print` command.  You can use
      :c:macro:`ALICE_PRINT_STORE` to implement this function.
  \endverbatim

  \param out Output stream \param element Store element
 */
template<typename StoreType>
void print( std::ostream& out, StoreType const& element )
{
  (void)element;
  out << std::endl;
}

/*! \brief Routine to print statistics of a store element to an output stream

  This routine is called by the `ps` command.

  \param out Output stream
  \param element Store element
*/
template<typename StoreType>
void print_statistics( std::ostream& out, StoreType const& element )
{
  (void)element;
  out << std::endl;
}

/*! \brief Statistics to log when calling `ps`

  \verbatim embed:rst
      This routine is called by the `ps` command, if logging is enabled.
  \endverbatim

  \param element Store element
*/
template<typename StoreType>
nlohmann::json log_statistics( StoreType const& element )
{
  (void)element;
  return nlohmann::json({});
}

/*! \brief Controls whether a store entry can read from a specific format

  If this function is overriden to return true, then also the function `read`
  must be impemented for the same store element type and format tag.

  \verbatim embed:rst
      You can use :c:macro:`ALICE_READ_FILE` to implement this function together
      with ``alice::read``.  However, if you need custom command line
      arguments, the macro cannot be used and one needs to specialize using these
      functions as described by the following example.

      .. code-block:: c++

         template<>
         bool can_read<std::string>( command& cmd )
         {
           cmd.add_flag( "--flag", "some flag" );
           cmd.add_option<std::string>( "--option", "an option stored in a string" );
         }

         template<>
         std::string read<std::string>( const std::string& filename, const command& cmd )
         {
           auto flag = cmd.is_set( "flag" );
           auto option = cmd.option_value<std::string>( "option" );

           // read the file and return a string...
         }
  \endverbatim

  \param cmd Mutable reference to command, e.g., to add custom options
*/
template<typename StoreType, typename Tag>
bool can_read( command& cmd )
{
  (void)cmd;
  return false;
}

/*! \brief Reads from a format and returns store element

  This function must be enabled by overriding the `can_read` function for the
  same store element type and format tag.  See `can_read` for more details and
  an example.

  The `read` function may throw an exception.  In this case, no new element
  is added to the store.  Anything can be thrown, but if a `std::string` is
  thrown, this string is used to output an error message to the error stream.

  \param filename Filename to read from
  \param cmd Reference to command, e.g., to check whether custom options are set
*/
template<typename StoreType, typename Tag>
StoreType read( const std::string& filename, const command& cmd )
{
  (void)filename;
  (void)cmd;
  throw std::runtime_error( "[e] unimplemented function" );
}

/*! \brief Controls whether a store entry can write to a specific format

  If this function is overriden to return true, then also the function
  `write` must be impemented for the same store element type and
  format tag.  See `can_read` for an example which can easily be adapted
  for `can_write` and `write`.

  \param cmd Mutable reference to command, e.g., to add custom options
*/
template<typename StoreType, typename Tag>
bool can_write( command& cmd )
{
  (void)cmd;
  return false;
}

/*! \brief Writes store element to a file format

  This function must be enabled by overriding the `can_write` function
  for the same store element type and format tag.

  \param element Store element to write
  \param filename Filename to write to
  \param cmd Reference to command, e.g., to check whether custom options are set
*/
template<typename StoreType, typename Tag>
void write( StoreType const& element, const std::string& filename, const command& cmd )
{
  (void)element;
  (void)filename;
  (void)cmd;
  throw std::runtime_error( "[e] unimplemented function" );
}

/*! \brief Writes store element to log file

  This function should be enabled by overriding the `can_write` function for the
  same store element type and format tag.

  \param element Store element to write
  \param os Output stream to write to
  \param cmd Reference to command, e.g., to check whether custom options are set
*/
template<typename StoreType, typename Tag>
void write( StoreType const& element, std::ostream& os, const command& cmd )
{
  (void)element;
  (void)os;
  (void)cmd;
  throw std::runtime_error( "[e] unimplemented function" );
}

/*! \brief Controls whether a store entry can be converted to an entry of a different store type

  If this function is overriden to return true, then also the function
  `convert` must be implemented for the same store types.

  \verbatim embed:rst
      You can use :c:macro:`ALICE_CONVERT` to implement this function together with ``convert``.
  \endverbatim
 */
template<typename SourceStoreType, typename DestStoreType>
bool can_convert()
{
  return false;
}

/*! \brief Converts a store entry into an entry of a different store type

  This function must be enabled by overriding the `can_convert`
  function for the same store element types.

  \verbatim embed:rst
      You can use :c:macro:`ALICE_CONVERT` to implement this function together with ``can_convert``.
  \endverbatim

  \param element Store element to convert
  \return Converted store element
*/
template<typename SourceStoreType, typename DestStoreType>
DestStoreType convert( SourceStoreType const& element )
{
  (void)element;
  throw std::runtime_error( "[e] unimplemented function" );
}

/*! \brief Controls whether a store element can be visualized

  If this function is overriden to be true, then also the function `show` mustbe
  implement for the same store type.  The command `show` allows to visualize a
  store element.  For this purpose a text file is written containing the visual
  representation, e.g., in terms of DOT or SVG (but other formats are possible).

  When implementing this function, it should return true and assign `extension`
  a default file extension for the representation format, which will be used to
  name temporary files.

  \param extension Default extension, without the dot (e.g., `"svg"`)
  \param cmd Mutable reference to command, e.g., to add custom options
*/
template<typename StoreType>
bool can_show( std::string& extension, command& cmd )
{
  (void)extension;
  (void)cmd;
  return false;
}

/*! \brief Generates the file to visualize a store element

  This function is function can be enabled by overriding the `can_show` function
  to return true.  It takes as parameter an output stream `out` to a file that
  is shown using a program by the `show` command.

  \param out Output stream
  \param element Store element to show
  \param cmd Reference to command, e.g., to check whether custom options are set
*/
template<typename StoreType>
void show( std::ostream& out, StoreType const& element, const command& cmd )
{
  (void)out;
  (void)element;
  (void)cmd;
}

/*! \brief Controls whether store element has specialized HTML output

  This function can be used to customize the output of the `print` in Python
  mode, when being used in Jupyter notebooks.  If this function returns true,
  the output of `html_repr` is used to create HTML output for a store element.

  Works only in Python mode.
*/
template<typename StoreType>
bool has_html_repr()
{
  return false;
}

/*! \brief Returns an HTML representation for a store element in Python mode

  This method enables to return a specialized HTML output for a store element
  when calling `print` as a function in Python mode.  This output can be used in
  environments such as Jupyter notebook.

  Works only in Python mode.

  \param element Store element
*/
template<typename StoreType>
std::string html_repr( StoreType const& element )
{
  (void)element;
  return "";
}
}
