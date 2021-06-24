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
  \file readline.hpp
  \brief Different readline implementations

  \author Mathias Soeken
*/

#pragma once

#include <iostream>
#include <string>
#include <vector>

#include "command.hpp"
#include "detail/utils.hpp"

#if defined READLINE_USE_READLINE

namespace alice
{

#include <readline/history.h>
#include <readline/readline.h>

class readline_wrapper
{
public:
  static readline_wrapper& instance()
  {
    static readline_wrapper _instance;

    return _instance;
  }

  void init( const environment::ptr& env )
  {
    for ( const auto& p : env->commands() )
    {
      command_names.push_back( p.first );
    }
    rl_attempted_completion_function = readline_wrapper::readline_completion_s;
  }

  bool read_command_line( const std::string& prefix, std::string& line )
  {
    auto* cline = readline( prefix.c_str() );

    /* something went wrong? */
    if ( !cline )
    {
      return false;
    }

    line = cline;
    detail::trim( line );
    free( cline );

    return true;
  }

  void add_to_history( const std::string& line )
  {
    add_history( line.c_str() );
  }

private:
  static constexpr const char* HISTORY = "./.history";
  readline_wrapper()
  {
    read_history(HISTORY);
  }

  ~readline_wrapper()
  {
    write_history(HISTORY);
  }

  static char** readline_completion_s( const char* text, int start, int end )
  {
    (void)end;
    if ( start == 0 )
    {
      return rl_completion_matches( text, []( const char* text, int state ) { return instance().command_iterator( text, state ); } );
    }
    else
    {
      return nullptr;
    }
  }

  char* command_iterator( const char* text, int state )
  {
    static std::vector<std::string>::const_iterator it;

    if ( state == 0 )
    {
      it = command_names.begin();
    }

    while ( it != command_names.end() )
    {
      const auto& name = *it++;
      if ( name.find( text ) != std::string::npos )
      {
        char* completion = (char*)malloc( sizeof( char ) * ( name.size() + 1 ) );
        strcpy( completion, name.c_str() );
        return completion;
      }
    }

    return nullptr;
  }

private:
  std::vector<std::string> command_names;
};
}

#else

namespace alice
{

class readline_wrapper
{
public:
  static readline_wrapper& instance()
  {
    static readline_wrapper _instance;

    return _instance;
  }

  void init( const environment::ptr& env )
  {
    (void)env;
  }

  bool read_command_line( const std::string& prefix, std::string& line )
  {
    std::cout << prefix;
    std::flush( std::cout );
    if ( !getline( std::cin, line ) )
    {
      return false;
    }

    detail::trim( line );
    return true;
  }

  void add_to_history( const std::string& )
  {
  }
};
}

#endif
