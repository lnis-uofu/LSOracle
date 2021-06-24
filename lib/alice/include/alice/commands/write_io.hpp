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
  \file write_io.hpp
  \brief Generic command for writing I/O

  \author Mathias Soeken
*/

#pragma once

#include <string>
#include <vector>

#include <fmt/format.h>

#include "../command.hpp"

namespace alice
{

template<class Tag, class... S>
class write_io_command : public command
{
public:
  write_io_command( const environment::ptr& env, const std::string& name )
    : command( env, fmt::format( "Write {} file", name ) )
  {
    []( ... ) {}( add_write_io_option_helper<S>()... );
    if ( allowed_options.size() == 1u )
    {
      default_option = allowed_options.front();
    }

    add_option( "filename,--filename", filename, "filename" );
    add_flag( "--log", "write file contents to log instead of filename" );
  }

protected:
  rules validity_rules() const
  {
    rules rules;

    rules.push_back( {[this]() { return allowed_options.size() == 1 || env->has_default_option( allowed_options ) || exactly_one_true_helper( {is_set( store_info<S>::option )...} ); }, "exactly one store needs to be specified"} );

    return rules;
  }

  void execute()
  {
    if ( exactly_one_true_helper( {is_set( store_info<S>::option )...} ) )
    {
      env->set_default_option( "" );
    }
    []( ... ) {}( write_io_helper<S>()... );
  }

  nlohmann::json log() const
  {
    if ( is_set( "--log" ) )
    {
      return {{"contents", contents}};
    }
    else
    {
      return nullptr;
    }
  }

private:
  template<typename Store>
  int add_write_io_option_helper()
  {
    if ( can_write<Store, Tag>( *this ) )
    {
      constexpr auto option = store_info<Store>::option;

      allowed_options.push_back( option );
      add_option_helper<Store>( opts );
    }

    return 0;
  }

  template<typename Store>
  int write_io_helper()
  {
    constexpr auto option = store_info<Store>::option;
    constexpr auto name = store_info<Store>::name;

    if ( is_set( option ) || option == default_option || env->is_default_option( option ) )
    {
      if ( env->store<Store>().current_index() == -1 )
      {
        env->out() << "[w] no " << name << " selected in store" << std::endl;
        env->set_default_option( "" );
      }
      else if ( is_set( "--log" ) )
      {
        try
        {
          std::ostringstream os;
          write<Store, Tag>( env->store<Store>().current(), os, static_cast<command const&>( *this ) );
          contents = os.str();
        }
        catch ( ... )
        {
          env->out() << "[w] writing to log is not supported for this command" << std::endl;
        }
        env->set_default_option( option );
      }
      else
      {
        write<Store, Tag>( env->store<Store>().current(), filename, static_cast<command const&>( *this ) );
        env->set_default_option( option );
      }
    }
    return 0;
  }

private:
  std::string filename;
  std::string contents;
  std::vector<std::string> allowed_options;
  std::string default_option;
};
}
