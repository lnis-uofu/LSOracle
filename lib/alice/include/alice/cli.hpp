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
  \file cli.hpp
  \brief The main entry method for the CLI

  \author Mathias Soeken
*/

#pragma once

#include <chrono>
#include <fstream>
#include <memory>
#include <regex>
#include <string>
#include <vector>

#include <CLI11.hpp>
#include <fmt/format.h>

#include "command.hpp"
#include "detail/logging.hpp"
#include "readline.hpp"

#include "commands/alias.hpp"
#include "commands/convert.hpp"
#include "commands/current.hpp"
#include "commands/help.hpp"
#include "commands/print.hpp"
#include "commands/ps.hpp"
#include "commands/quit.hpp"
#include "commands/read_io.hpp"
#include "commands/set.hpp"
#include "commands/show.hpp"
#include "commands/store.hpp"
#include "commands/write_io.hpp"

namespace alice
{

/*! \brief CLI main class

  The stores of a CLI are passed as type arguments to ``cli``.  For example, if
  the CLI has stores for Graphs and Trees which are handled by classes ``graph``
  and ``tree``, respectively, the class instantiation is ``cli<graph, tree>``.
*/
template<class... S>
class cli
{
public:
  /*! \brief Default constructor

    Initializes the CLI with a prefix that is used as a command prefix in
    stand-alone application mode and as a module name when build as Python
    module.

    The constructor will add the default commands to the CLI.  If no store type
    is specified, then no store-related command will be added.

    \param prefix Either command prefix or module name (depending on build mode)
  */
  explicit cli( const std::string& prefix )
      : env( std::make_shared<environment>() ),
        prefix( prefix ),
        opts( std::make_shared<CLI::App>() )
  {
    /* for each type in S ... */
    []( ... ) {}( ( env->add_store<S>(), 0 )... );

    set_category( "General" );
    insert_command( "alias", std::make_shared<alias_command>( env ) );
    insert_command( "help", std::make_shared<help_command>( env ) );
    insert_command( "quit", std::make_shared<quit_command>( env ) );
    insert_command( "set", std::make_shared<set_command>( env ) );

    if ( sizeof...( S ) )
    {
      insert_command( "convert", std::make_shared<convert_command<S...>>( env ) );
      insert_command( "current", std::make_shared<current_command<S...>>( env ) );
      insert_command( "print", std::make_shared<print_command<S...>>( env ) );
      insert_command( "ps", std::make_shared<ps_command<S...>>( env ) );
      insert_command( "show", std::make_shared<show_command<S...>>( env ) );
      insert_command( "store", std::make_shared<store_command<S...>>( env ) );
    }

    opts->add_option( "-c,--command", command, "process semicolon-separated list of commands" );
    opts->add_option( "-f,--filename", file, "process file with new-line separated list of commands" );
    opts->add_flag( "-e,--echo", "echo the command if read from command line or file" );
    opts->add_flag( "-n,--counter", "show a counter in the prefix" );
    opts->add_flag( "-i,--interactive", "continue in interactive mode after processing commands (in command or file mode)" );
    opts->add_option( "-l,--log", logname, "logs the execution and stores many statistical information" );
  }

  /*! \brief Sets the current category

    This category will be used as category for all commands that are added
    afterwards, until this method is called again with a different argument.

    The categories are used in the ``help`` command to organize the commands.

    The macros :c:macro:`ALICE_COMMAND` and :c:macro:`ALICE_ADD_COMMAND` will
    automatically call this method.

    \param _category Category name
  */
  void set_category( const std::string& _category )
  {
    category = _category;
  }

  /*! \brief Inserts a command

    Inserts a command (as a shared pointer) to the CLI.

    The macro :c:macro:`ALICE_ADD_COMMAND` will automatically call this method
    with a convention that a command with name ``<name>`` must be called
    ``<name>_command``.

    \param name Name of the command
    \param cmd Shared pointer to a command instance
  */
  void insert_command( const std::string& name, const std::shared_ptr<command>& cmd )
  {
    env->_categories[category].push_back( name );
    env->_commands[name] = cmd;
  }

  /*! \brief Inserts a read command

    Inserts a read command for a given file tag.  The name of the command can be
    arbitrary but the default convention is to prefix it with ``read_``.  The
    macro :c:macro:`ALICE_ADD_FILE_TYPE` together :c:macro:`ALICE_READ_FILE`
    will automatically add a read command called ``read_<tagname>``.

    \param name Name of the command
    \param label Label for the file type (used in help string)
  */
  template<typename Tag>
  void insert_read_command( const std::string& name, const std::string& label )
  {
    insert_command( name, std::make_shared<read_io_command<Tag, S...>>( env, label ) );
  }

  /*! \brief Inserts a write command

    Inserts a writ command for a given file tag.  The name of the command can be
    arbitrary but the default convention is to prefix it with ``write_``.  The
    macro :c:macro:`ALICE_ADD_FILE_TYPE` together :c:macro:`ALICE_WRITE_FILE`
    will automatically add a write command called ``write_<tagname>``.

    \param name Name of the command
    \param label Label for the file type (used in help string)
  */
  template<typename Tag>
  void insert_write_command( const std::string& name, const std::string& label )
  {
    insert_command( name, std::make_shared<write_io_command<Tag, S...>>( env, label ) );
  }

  /*! \brief Runs the shell

    This function is only used if the CLI is used in stand-alone mode, not when
    used as Python module.  The values ``argc`` and ``argv`` can be taken from
    the ``main`` function.  For some flags, such as ``-f`` and ``-c``, the CLI
    will read commands from a file or the command line, respectively, and then
    stop (unless flag ``-i`` is set).  Otherwise, the CLI will enter a loop that
    accepts commands as user inputer.

    \param argc Number of arguments (incl. program name, like ``argc`` in ``main``)
    \param argv Argument values (like ``argv`` in ``main``)
  */
  int run( int argc, char** argv )
  {
    try
    {
      opts->parse( argc, argv );
    }
    catch ( const CLI::CallForHelp& e )
    {
      env->out() << opts->help();
      return 1;
    }
    catch ( const CLI::ParseError& e )
    {
      env->out() << "[e] " << e.what() << std::endl;
      return 2;
    }

    read_aliases();

    if ( opts->count( "-l" ) )
    {
      env->log = true;
      env->logger.start( logname );
    }

    if ( opts->count( "-c" ) )
    {
      auto split = detail::split_with_quotes<';'>( command );

      std::string batch_string;
      std::string abc_opts;
      for ( auto& line : split )
      {
        detail::trim( line );

        if ( opts->count( "-e" ) )
        {
          env->out() << get_prefix() << line << std::endl;
        }
        if ( !execute_line( preprocess_alias( line ) ) )
        {
          return 1;
        }

        if ( env->quit )
        {
          break;
        }
      }

      // cleanup to prevent memory leak
      env->_categories.clear();
      env->_commands.clear();
    }
    else if ( opts->count( "-f" ) )
    {
      process_file( file, opts->count( "-e" ) );

      if ( !opts->count( "-i" ) )
      {
        env->quit = true;
      }
    }

    if ( ( !opts->count( "-c" ) && !opts->count( "-f" ) ) || ( !env->quit && opts->count( "-i" ) ) )
    {
      auto& rl = readline_wrapper::instance();
      rl.init( env );

      std::string line;
      while ( !env->quit && rl.read_command_line( get_prefix(), line ) )
      {
        execute_line( preprocess_alias( line ) );
        rl.add_to_history( line );
      }
    }

    if ( env->log )
    {
      env->logger.stop();
    }

    return 0;
  }

  /*! \ifcond PRIVATE */
private:
  bool execute_line( const std::string& line )
  {
    /* at this point we can assume that line is trimmed */

    /* ignore comments and empty lines */
    if ( line.empty() || line[0] == '#' )
    {
      return false;
    }

    /* split commands if line contains a semi-colon */
    const auto lines = detail::split_with_quotes<';'>( line );

    /* if more than one command is detected recurse on each part */
    if ( lines.size() > 1u )
    {
      auto result = true;

      for ( const auto& cline : lines )
      {
        result = result && execute_line( preprocess_alias( cline ) );
      }

      return result;
    }

    /* escape to shell */
    if ( line[0] == '!' )
    {
      const auto now = std::chrono::system_clock::now();
      const auto result = detail::execute_program( line.substr( 1u ) );

      env->out() << result.second;

      if ( !result.second.empty() && result.second.back() != '\n' )
      {
        env->out() << '%' << std::endl;
      }

      if ( env->log )
      {
        nlohmann::json log = {{"status", result.first}, {"output", result.second}};
        env->logger.log( log, line, now );
      }

      return true;
    }

    /* read commands from file */
    if ( line[0] == '<' )
    {
      auto filename = line.substr( 1u );
      detail::trim( filename );
      process_file( filename, opts->count( "-e" ) );
      return true;
    }

    auto vline = detail::split_with_quotes<' '>( line );

    const auto it = env->commands().find( vline.front() );
    if ( it != env->commands().end() )
    {
      const auto now = std::chrono::system_clock::now();
      const auto result = it->second->run( vline );

      if ( result && env->log )
      {
        env->logger.log( it->second->log(), line, now );
      }

      return result;
    }
    else
    {
      env->err() << "[e] unknown command: " << vline.front() << std::endl;
      return false;
    }

    return true;
  }

  bool process_file( const std::string& filename, bool echo, bool error_on_not_found = true )
  {
    std::ifstream in( filename.c_str(), std::ifstream::in );

    if ( !in.good() )
    {
      if ( error_on_not_found )
      {
        env->out() << "[e] file " << filename << " not found" << std::endl;
      }
      return true;
    }

    std::string line;

    while ( getline( in, line ) )
    {
      detail::trim( line );

      if ( echo )
      {
        env->out() << get_prefix() << line << std::endl;
      }

      execute_line( preprocess_alias( line ) );

      if ( env->quit )
      {
        /* quit */
        return true;
      }
    }

    /* do not quit */
    return false;
  }

  std::string get_prefix()
  {
    std::string r = prefix;

    if ( env->has_default_option() )
    {
      r += fmt::format( " [{}]", env->default_option() );
    }
    if ( opts->count( "-n" ) )
    {
      r += fmt::format( " {}", counter++ );
    }
    return r + "> ";
  }

  void read_aliases()
  {
#if defined(ALICE_ENV_HOME)
#define ALICE_STRINGIFY(x) #x
#define ALICE_TOSTRING(x) ALICE_STRINGIFY(x)
    std::string env = ALICE_TOSTRING( ALICE_ENV_HOME );
    if ( env.empty() ) { return;}

    if ( auto* path = std::getenv( env.c_str() ) )
    {
      std::string alias_path = fmt::format( "{}/alias", path );
      process_file( alias_path, false, false );
    }
#endif
  }

  std::string preprocess_alias( const std::string& line )
  {
    std::smatch m;

    for ( const auto& p : env->aliases() )
    {
      if ( std::regex_match( line, m, std::regex( p.first ) ) )
      {
        std::vector<std::string> matches( m.size() - 1u );

        for ( auto i = 0u; i < matches.size(); ++i )
        {
          matches[i] = std::string( m[i + 1] );
        }

        const auto str = detail::trim_copy( detail::format_with_vector( p.second, matches ) );
        return preprocess_alias( str );
      }
    }

    return line;
  }

public:
  environment::ptr env;

private:
  std::string prefix;
  std::shared_ptr<CLI::App> opts;
  std::string category;

  std::string command, file, logname;

  unsigned counter{1u};
  /*! \endcond */
};
}
