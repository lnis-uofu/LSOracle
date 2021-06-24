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
  \file help.hpp
  \brief Shows help

  \author Mathias Soeken
*/

#pragma once

#include <algorithm>
#include <fstream>
#include <iostream>
#include <vector>

#include "../command.hpp"
#include "../detail/utils.hpp"

namespace alice
{

class help_command : public command
{
public:
  explicit help_command( const environment::ptr& env ) : command( env, "Shows help" )
  {
    add_flag( "-d,--detailed", "show command descriptions" );
    add_option( "-s,--search", search, "search for commands in help descriptions" );
    add_option( "--docs", filename, "create output for documentation into filename (use - for stdout)" );
  }

protected:
  void execute()
  {
    if ( is_set( "-s" ) )
    {
      search_command();
    }
    else if ( is_set( "--docs" ) )
    {
      print_docs();
    }
    else
    {
      print_commands();
    }
  }

private:
  void search_command()
  {
    for ( const auto& command : env->commands() )
    {
      auto text = command.second->opts.help();
      std::string::iterator it;
      std::string::size_type pos = 0;

      /* split text */
      auto lines = detail::split( text, "\n" );

      const auto pred = []( char ch1, char ch2 ) {
        return std::toupper( ch1 ) == std::toupper( ch2 );
      };

      std::string output;
      for ( auto& line : lines )
      {
        auto matched = false;
        while ( ( it = std::search( line.begin() + pos, line.end(), search.begin(), search.end(), pred ) ) != line.end() )
        {
          matched = true;
          pos = std::distance( line.begin(), it );

          std::string found( it, it + search.size() );

          line.replace( pos, search.size(), "\033[1;32m" + found + "\033[0m" );
          pos += 15 + search.size();
        }

        if ( matched )
        {
          output += line + "\n";
        }
      }

      if ( !output.empty() )
      {
        env->out() << "[i] found match in command \033[1;34m" << command.first << "\033[0m" << std::endl
                   << output << std::endl
                   << std::endl;
      }
    }
  }

  void print_commands()
  {
    for ( auto& p : env->_categories )
    {
      env->out() << p.first << " commands:" << std::endl;

      std::sort( p.second.begin(), p.second.end() );

      if ( is_set( "-d" ) )
      {
        for ( const auto& name : p.second )
        {
          env->out() << fmt::format( " {:<17} : {}", name, env->commands().at( name )->caption() ) << std::endl;
        }
        env->out() << std::endl;
      }
      else
      {
        auto counter = 0;
        env->out() << " ";

        for ( const auto& name : p.second )
        {
          if ( counter > 0 && ( counter % 4 == 0 ) )
          {
            env->out() << std::endl
                       << " ";
          }
          env->out() << fmt::format( "{:<17}", name );
          ++counter;
        }
        env->out() << std::endl
                   << std::endl;
      }
    }
  }

  void print_docs()
  {
    std::streambuf* buf{nullptr};
    std::ofstream of;

    if ( filename == "-" )
    {
      buf = env->out().rdbuf();
    }
    else
    {
      of.open( filename );
      buf = of.rdbuf();
    }
    std::ostream os( buf );

    os << "Commands\n"
       << "========\n\n";

    for ( auto& p : env->_categories )
    {
      os << p.first << " commands\n"
         << std::string( p.first.size() + 9, '-' ) << "\n\n";

      for ( const auto& name : p.second )
      {
        os << name << "\n"
           << std::string( name.size(), '~' ) << "\n";
        os << "\n";
        os << env->commands().at( name )->opts.help( 50, "", true ) << "\n\n----\n\n";
      }
    }
  }

private:
  std::string search;
  std::string filename;
};
} // namespace alice
