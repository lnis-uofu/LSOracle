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
  \file api.hpp
  \brief API macros

  \author Mathias Soeken
*/

#pragma once

#include <string>
#include <vector>

#include <fmt/format.h>

#include "detail/python.hpp"
#include "detail/utils.hpp"
#include "cli.hpp"

namespace alice
{

/*! \cond PRIVATE */
/* dynamic compile-time list construction based on https://stackoverflow.com/a/24092000 */
template<typename T, typename Tuple>
struct tuple_append;

template<typename T, typename... S>
struct tuple_append<T, std::tuple<S...>>
{
  using type = std::tuple<S..., T>;
};

template<typename T>
struct tuple_to_cli;

template<typename... S>
struct tuple_to_cli<std::tuple<S...>>
{
  using type = alice::cli<S...>;
};

template<typename T, int N>
struct list_maker_key : list_maker_key<T, N - 1> {};

template<typename T>
struct list_maker_key<T, 0> {};

#define _ALICE_START_LIST(name) \
  struct name##_list_maker; \
  std::tuple<> list_maker_helper_(list_maker_key<name##_list_maker, __COUNTER__>);

#define _ALICE_ADD_TO_LIST(name, added_type) \
  tuple_append<added_type, decltype( list_maker_helper_( list_maker_key<name##_list_maker, __COUNTER__>{} ) )>::type \
  list_maker_helper_(list_maker_key<name##_list_maker, __COUNTER__>);

#define _ALICE_END_LIST(name) \
  using name = decltype(list_maker_helper_(list_maker_key<name##_list_maker, __COUNTER__>{})); 
/*! \endcond */

/*! \brief Adds a store

  Adds a new store type alice.  The first parameter is a type, all other
  parameters are for program arguments and help strings in the general
  store-related commands.
  
  \param type A type to define the store
  \param _option Long option name to select store in general store commands
  \param _mnemonic Short option name (single character) to select store in general store commands (cannot be ``n`` or ``v``)
  \param _name Singular name used in help texts
  \param _name_plural Plural name used in help texts
*/
#define ALICE_ADD_STORE(type, _option, _mnemonic, _name, _name_plural) \
template<> \
struct store_info<type> \
{ \
  static constexpr const char* key = #type; \
  static constexpr const char* option = _option; \
  static constexpr const char* mnemonic = _mnemonic; \
  static constexpr const char* name = _name; \
  static constexpr const char* name_plural = _name_plural; \
}; \
_ALICE_ADD_TO_LIST(alice_stores, type)

/*! \cond PRIVATE */
/* some global data structure */
struct alice_globals
{
  static alice_globals& get()
  {
    static alice_globals instance;
    return instance;
  }

  std::vector<std::pair<std::string, std::string>> command_names;
  std::vector<std::string> read_tags, write_tags;
  std::vector<std::string> read_names, write_names;
};
/*! \endcond */

template<typename CLI, typename Tuple, std::size_t Index>
struct insert_read_commands
{
  insert_read_commands( CLI& cli )
  {
    insert_read_commands<CLI, Tuple, Index - 1> irc( cli );

    using tag_type = std::tuple_element_t<Index - 1, Tuple>;
    cli.template insert_read_command<tag_type>( fmt::format( "read_{}", alice_globals::get().read_tags[Index - 1] ), alice_globals::get().read_names[Index - 1] );
  }
};

template<typename CLI, typename Tuple>
struct insert_read_commands<CLI, Tuple, 0>
{
  insert_read_commands( CLI& cli )
  {
    cli.set_category( "I/O" );
  }
};

template<typename CLI, typename Tuple, std::size_t Index>
struct insert_write_commands
{
  insert_write_commands( CLI& cli )
  {
    insert_write_commands<CLI, Tuple, Index - 1> irc( cli );

    using tag_type = std::tuple_element_t<Index - 1, Tuple>;
    cli.template insert_write_command<tag_type>( fmt::format( "write_{}", alice_globals::get().write_tags[Index - 1] ), alice_globals::get().write_names[Index - 1] );
  }
};

template<typename CLI, typename Tuple>
struct insert_write_commands<CLI, Tuple, 0>
{
  insert_write_commands( CLI& cli )
  {
    cli.set_category( "I/O" );
  }
};

/*! \brief Returns a one-line string to show when printing store contents

  This macro is used to return a string that is shown in the output of
  ``store``, when each store entry is listed with its index and a short one-line
  descriptiont text.

  The macro must be followed by a code block.

  \param type Store type
  \param element Reference to the store element
*/
#define ALICE_DESCRIBE_STORE(type, element) \
template<> \
inline std::string to_string<type>( type const& element )

/*! \brief Prints a store element to the terminal

  This macro is used to generate the code that is executed when calling
  ``print`` for a store.

  The macro must be followed by a code block.

  \param type Store type
  \param os Output stream (default is ``std::cout`` when in standalone mode)
  \param element Reference to the store element
*/
#define ALICE_PRINT_STORE(type, os, element) \
template<> \
inline void print<type>( std::ostream& os, type const& element )

/*! \brief Prints statistics about a store element to the terminal

  This macro is used to generate the output that is printed when calling ``ps``
  for a store.

  The macro must be followed by a code block.

  \param type Store type
  \param os Output stream (default is ``std::cout`` when in standalone mode)
  \param element Reference to the store element
*/
#define ALICE_PRINT_STORE_STATISTICS(type, os, element) \
template<> \
inline void print_statistics<type>( std::ostream& os, type const& element )

/*! \brief Prints statistics about a store element to the terminal

  This macro is used to generate the JSON object that is logged when calling
  `ps` for a store element.  The body must return an ``nlohmann::json`` object.

  The macro must be followed by a code block.

  \param type Store type
  \param element Reference to the store element
*/
#define ALICE_LOG_STORE_STATISTICS(type, element) \
template<> \
inline nlohmann::json log_statistics<type>( type const& element )

/*! \brief Read from a file into a store

  This macro adds an implementation for reading from a file into a store.
  Different file types may be supported, which are indexed using the tag.

  The macro must be followed by a code block.
  
  \param type Store type
  \param tag File tag
  \param filename Filename
  \param cmd Reference to the command line interface of the command
*/
#define ALICE_READ_FILE(type, tag, filename, cmd) \
template<> \
inline bool can_read<type, io_##tag##_tag_t>( command& ) { return true; } \
template<> \
inline type read<type, io_##tag##_tag_t>( const std::string& filename, const command& cmd )

/*! \brief Write to a file from a store

  This macro adds an implementation for writing to a file from a store.
  Different file types may be supported, which are indexed using the tag.

  The macro must be followed by a code block.
  
  \param type Store type
  \param tag File tag
  \param element Reference to the store element
  \param filename Filename
  \param cmd Reference to the command line interface of the command
*/
#define ALICE_WRITE_FILE(type, tag, element, filename, cmd) \
template<> \
inline bool can_write<type, io_##tag##_tag_t>( command& ) { return true; } \
template<> \
inline void write<type, io_##tag##_tag_t>( type const& element, const std::string& filename, const command& cmd )

/*! \brief Registers a file type to alice

  Calling this macro will mainly cause the addition of two commands
  ``read_<tag>`` and ``write_<tag>`` to alice to read from files and write to
  files.  The actual implementation is done using ``ALICE_READ_FILE`` and
  ``ALICE_WRITE_FILE`` which will also associate store types to file tags.

  \param tag File tag
  \param name Name that is used for help strings
*/
#define ALICE_ADD_FILE_TYPE(tag, name) \
struct io_##tag##_tag_t \
{ \
  io_##tag##_tag_t() \
  { \
    if ( std::find( alice_globals::get().read_tags.begin(), alice_globals::get().read_tags.end(), #tag ) != alice_globals::get().read_tags.end() ) return; \
    alice_globals::get().read_tags.push_back(#tag); \
    alice_globals::get().read_names.push_back(name); \
    alice_globals::get().write_tags.push_back(#tag); \
    alice_globals::get().write_names.push_back(name); \
  } \
}; \
io_##tag##_tag_t _##tag##_tag; \
_ALICE_ADD_TO_LIST(alice_read_tags, io_##tag##_tag_t) \
_ALICE_ADD_TO_LIST(alice_write_tags, io_##tag##_tag_t)

/*! \brief Registers a read-only file type to alice

  Like ``ALICE_ADD_FILE_TYPE`` but only adds ``read_<tag>``.

  \param tag File tag
  \param name Name that is used for help strings
*/
#define ALICE_ADD_FILE_TYPE_READ_ONLY(tag, name) \
struct io_##tag##_tag_t \
{ \
  io_##tag##_tag_t() \
  { \
    if ( std::find( alice_globals::get().read_tags.begin(), alice_globals::get().read_tags.end(), #tag ) != alice_globals::get().read_tags.end() ) return; \
    alice_globals::get().read_tags.push_back(#tag); \
    alice_globals::get().read_names.push_back(name); \
  } \
}; \
io_##tag##_tag_t _##tag##_tag; \
_ALICE_ADD_TO_LIST(alice_read_tags, io_##tag##_tag_t)

/*! \brief Registers a write-only file type to alice

  Like ``ALICE_ADD_FILE_TYPE`` but only adds ``write_<tag>``.

  \param tag File tag
  \param name Name that is used for help strings
*/
#define ALICE_ADD_FILE_TYPE_WRITE_ONLY(tag, name) \
struct io_##tag##_tag_t \
{ \
  io_##tag##_tag_t() \
  { \
    if ( std::find( alice_globals::get().write_tags.begin(), alice_globals::get().write_tags.end(), #tag ) != alice_globals::get().write_tags.end() ) return; \
    alice_globals::get().write_tags.push_back(#tag); \
    alice_globals::get().write_names.push_back(name); \
  } \
}; \
io_##tag##_tag_t _##tag##_tag; \
_ALICE_ADD_TO_LIST(alice_write_tags, io_##tag##_tag_t)

////////////////////////////////////////////////////////////////////////////////
// convert

/*! \brief Converts store element into another store element

  This macro adds an implementation for conversion of a store element of type
  ``from`` to a store element of type ``to``.  It causes a new option
  ``--<from>_to_<to>`` for the ``convert`` command.

  The macro must be followed by a code block.

  \param from Store type that should be converted from
  \param element Reference to the store element that should be converted
  \param to Store type that should be converted to
  \return New store element
*/
#define ALICE_CONVERT(from, element, to) \
template<> \
inline bool can_convert<from, to>() \
{ \
  return true; \
} \
template<> \
inline to convert<from, to>( from const& element )

////////////////////////////////////////////////////////////////////////////////
// show

/*! \brief Shows store element

  This macro adds an implementation to show a store element using the command
  ``show``.  It implements the store API functions ``can_show`` and ``show``.

  The macro must be followed by a code block.

  \param type Store type
  \param extension Default extension (for temporary filenames, without dot, e.g. ``"svg"``)
  \param os Output stream
  \param element Reference to the store element that should be shown
*/
#define ALICE_SHOW(type, extension, os, element) \
template<> \
inline bool can_show<type>( std::string& ext, command& ) \
{ \
  ext = extension; \
  return true; \
} \
template<> \
inline void show<type>( std::ostream& os, type const& element, const command& )

////////////////////////////////////////////////////////////////////////////////
// html_repr

/*! \brief Generates HTML output for a store element

  This macro is only needed when the shell is used as a Python module inside an
  environment such as Jupyter notebooks.  Then a specialized output can be
  configured for a store element when calling the `print` method on it.  It
  implements the store API functions `has_html_repr` and `html_repr`.

  The macro must be followed by a code block.

  \param type Store type
  \param element Reference to the current store element
*/
#define ALICE_STORE_HTML(type, element) \
template<> \
inline bool has_html_repr<type>() { return true; } \
template<> \
std::string html_repr<type>( type const& element )

////////////////////////////////////////////////////////////////////////////////
// commands

template<typename CLI, typename Tuple, std::size_t Index>
struct insert_commands
{
  insert_commands( CLI& cli )
  {
    insert_commands<CLI, Tuple, Index - 1> ic( cli );

    using command_type = std::tuple_element_t<Index - 1, Tuple>;
    cli.set_category( alice_globals::get().command_names[Index - 1].second );
    cli.insert_command( alice_globals::get().command_names[Index - 1].first, std::make_shared<command_type>( cli.env ) );
  }
};

template<typename CLI, typename Tuple>
struct insert_commands<CLI, Tuple, 0>
{
  insert_commands( CLI& )
  {
  }
};

#define _ALICE_COMMAND_INIT(name, category) \
struct name##_command_init \
{ \
  name##_command_init() \
  { \
    alice_globals::get().command_names.emplace_back(#name, category); \
  } \
}; \
name##_command_init _##name##_command_init;

/*! \brief Add and implements a simple command

  Unline ``ALICE_ADD_COMMAND``, this macro can be used to also implement a
  simple command.  However, it allows only to implement the code of the execute
  function, and therefore no customization of command arguments, validators, and
  logging is possible.

  The macro must be followed by a code block.

  \param name Name of the command
  \param category Category of the command (as shown in ``help``)
  \param description Short description of the command (as shown in ``help``)
*/
#define ALICE_COMMAND(name, category, description) \
_ALICE_COMMAND_INIT(name, category) \
class name##_command; \
_ALICE_ADD_TO_LIST(alice_commands, name##_command) \
class name##_command : public command \
{ \
public: \
  explicit name##_command( const environment::ptr& env ) : command( env, description ) {} \
protected: \
  void execute(); \
}; \
void name##_command::execute()

/*! \brief Add a command

  This macro adds a command to the shell interface.  When this macro is called,
  a class of name ``<name>_command`` must have been defined that inherits from
  ``alice::command`` or some of its subclasses.

  The command is accessible from the shell interface using ``name``.  In Python
  mode, the module will contain a function ``name``.

  \param name Name of the command
  \param category Category of the command (as shown in ``help``)
 */
#define ALICE_ADD_COMMAND(name, category) \
_ALICE_COMMAND_INIT(name, category) \
_ALICE_ADD_TO_LIST(alice_commands, name##_command)

/*! \cond PRIVATE */
#define ALICE_INIT \
_ALICE_START_LIST( alice_stores ) \
_ALICE_START_LIST( alice_commands ) \
_ALICE_START_LIST( alice_read_tags ) \
_ALICE_START_LIST( alice_write_tags )

#define _ALICE_MAIN_BODY(prefix) \
  using namespace alice; \
  _ALICE_END_LIST( alice_stores ) \
  _ALICE_END_LIST( alice_commands ) \
  _ALICE_END_LIST( alice_read_tags ) \
  _ALICE_END_LIST( alice_write_tags ) \
  \
  using cli_t = tuple_to_cli<alice_stores>::type; \
  cli_t cli( #prefix ); \
  \
  insert_read_commands<cli_t, alice_read_tags, std::tuple_size<alice_read_tags>::value> irc( cli ); \
  insert_write_commands<cli_t, alice_write_tags, std::tuple_size<alice_write_tags>::value> iwc( cli ); \
  insert_commands<cli_t, alice_commands, std::tuple_size<alice_commands>::value> ic( cli );
/*! \endcond */

#if defined ALICE_PYTHON
#define ALICE_MAIN(prefix) \
PYBIND11_MODULE(prefix, m) \
{ \
  _ALICE_MAIN_BODY(prefix) \
  alice::detail::create_python_module( cli, m ); \
}
#elif defined ALICE_CINTERFACE
#define ALICE_MAIN(prefix) \
using namespace alice; \
_ALICE_END_LIST( alice_stores ) \
_ALICE_END_LIST( alice_commands ) \
_ALICE_END_LIST( alice_read_tags ) \
_ALICE_END_LIST( alice_write_tags ) \
\
using cli_t = tuple_to_cli<alice_stores>::type; \
\
extern "C" { \
  void* prefix##_create() { \
    auto cli = new cli_t( #prefix ); \
    insert_read_commands<cli_t, alice_read_tags, std::tuple_size<alice_read_tags>::value> irc( *cli ); \
    insert_write_commands<cli_t, alice_write_tags, std::tuple_size<alice_write_tags>::value> iwc( *cli ); \
    insert_commands<cli_t, alice_commands, std::tuple_size<alice_commands>::value> ic( *cli ); \
    return reinterpret_cast<void*>( cli ); \
  } \
  \
  void prefix##_delete( void* p ) { \
    auto cli = reinterpret_cast<cli_t*>( p ); \
    delete cli; \
  } \
  \
  int prefix##_command( void* p, const char *cmd, char* log, size_t size ) { \
    auto cli = reinterpret_cast<cli_t*>( p ); \
    auto vline = detail::split_with_quotes<' '>( cmd ); \
    const auto it = cli->env->commands().find( vline.front() ); \
    if ( it != cli->env->commands().end() ) \
    { \
      const auto result = it->second->run( vline ); \
      if ( result ) \
      { \
        const auto json = it->second->log(); \
        if ( log && !json.is_null() ) { \
          const auto dump = json.dump(); \
          strncpy( log, dump.c_str(), size ); \
          return dump.size() + 1; \
        } \
        return 0; \
      } \
    } \
    return -1; \
  } \
}
#else
/*! \brief Alice main routine

  The use of this macro is two-fold depending on whether alice is used for a
  stand-alone application or for creating a Python library:
  
  - In stand-alone application mode, this starts the interactive shell which
    accepts commands and replaces the C++ ``main`` method.  The prefix in the
    shell will be taken from the first argument.
  - In Python mode, this method will create a Python module with name
    ``prefix``.

  \param prefix Shell prefix or python module name (depending on mode)
 */
#define ALICE_MAIN(prefix) \
int main( int argc, char ** argv ) \
{ \
  _ALICE_MAIN_BODY(prefix) \
  return cli.run( argc, argv ); \
}
#endif

ALICE_INIT

}
