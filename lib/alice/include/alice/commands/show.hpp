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
  \file ps.hpp
  \brief Print statistics

  \author Mathias Soeken
*/

#pragma once

#ifndef _WIN32
#include <unistd.h>
#endif

#include <cstdlib>
#include <cstdio>
#include <string>
#include <unordered_map>

#include <fmt/format.h>

#include "../command.hpp"

namespace alice
{

template<class... S>
class show_command : public command
{
public:
  explicit show_command( const environment::ptr& env )
    : command( env, "Show store entry" )
  {
    []( ... ) {}( add_show_option<S>()... );
    if ( option_count != 1u )
    {
      default_option.clear();
    }

    add_option( "--filename,filename", filename, "filename for visual representation (temporary file by default)" );
    add_option( "--program", program, "program to open file", true );
    add_flag( "--silent", "do not open file" );
    add_flag( "--delete", "delete file after showing (program must run in foreground)" );
  }

protected:
  rules validity_rules() const
  {
    rules rules;

    rules.push_back( {[this]() { return option_count == 1 || env->has_default_option() || exactly_one_true_helper<bool>( {is_set( store_info<S>::option )...} ); }, "exactly one store needs to be specified"} );

    return rules;
  }

  void execute()
  {
    [](...){}( show_store<S>()... );
  }

private:
  template<typename Store>
  int add_show_option()
  {
    std::string extension;
    if ( can_show<Store>( extension, *this ) )
    {
      constexpr auto option = store_info<Store>::option;

      option_count++;
      default_option = option;
      add_option_helper<Store>( opts );

      extensions[option] = extension;
    }
    return 0;
  }

  template<typename Store>
  int show_store()
  {
    constexpr auto option = store_info<Store>::option;
    constexpr auto name = store_info<Store>::name;

    if ( is_set( option ) || default_option == option || env->is_default_option( option ) )
    {
      if ( store<Store>().current_index() == -1 )
      {
        env->out() << "[w] no " << name << " in store" << std::endl;
        env->set_default_option( "" );
      }
      else
      {
        if ( !is_set( "filename" ) )
        {
#if _WIN32
          filename = fmt::format( "{}.{}", std::tmpnam( nullptr ), extensions.at( option ) );
#else
          char* tmpname = new char[18 + extensions.at( option ).size()];
          sprintf( tmpname, "/tmp/aliceXXXXXX.%s", extensions.at( option ).c_str() );
          mkstemps( tmpname, extensions.at( option ).size() + 1 );
          filename = tmpname;
          delete[] tmpname;
#endif
        }

        std::ofstream os( filename.c_str(), std::ofstream::out );
        show<Store>( os, store<Store>().current(), *this );
        os.close();

        if ( !is_set( "silent" ) )
        {
          std::system( fmt::format( program, filename ).c_str() );
        }

        if ( is_set( "delete" ) )
        {
          std::remove( filename.c_str() );
        }

        env->set_default_option( option );
      }
    }

    return 0;
  }

private:
  std::string filename;
#ifdef __APPLE__
  std::string program = "open {}";
#elif _WIN32
  std::string program = "start {}";
#else
  std::string program = "xdg-open {}";
#endif
  std::unordered_map<std::string, std::string> extensions;
  unsigned option_count = 0u;
  std::string default_option;
};

}
