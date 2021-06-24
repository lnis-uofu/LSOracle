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
  \file read_io.hpp
  \brief Generic command for reading I/O

  \author Mathias Soeken
*/

#pragma once

#include <string>
#include <vector>

#include <fmt/format.h>

#include "../command.hpp"
#include "../detail/utils.hpp"
#include "../validators.hpp"

namespace alice
{

template<class Tag, class... S>
class read_io_command : public command
{
public:
  read_io_command( const environment::ptr& env, const std::string& name )
      : command( env, fmt::format( "Read {} file", name ) )
  {
    []( ... ) {}( add_read_io_option_helper<S>()... );
    if ( allowed_options.size() == 1u )
    {
      default_option = allowed_options.front();
    }

    add_option( "filename,--filename", filenames, "one or multiple filenames" )->check( ExistingFileWordExp )->required();
    add_flag( "-n,--new", "create new store entry" );
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
    bool multiple{filenames.size() > 1};
    for ( const auto& filename : filenames )
    {
      []( ... ) {}( read_io_helper<S>( multiple, detail::word_exp_filename( filename ) )... );
    }
  }

private:
  template<typename Store>
  int add_read_io_option_helper()
  {
    if ( can_read<Store, Tag>( *this ) )
    {
      constexpr auto option = store_info<Store>::option;

      allowed_options.push_back( option );
      add_option_helper<Store>( opts );
    }

    return 0;
  }

  template<typename Store>
  int read_io_helper( bool multiple, const std::string& filename )
  {
    constexpr auto option = store_info<Store>::option;

    if ( is_set( option ) || option == default_option || env->is_default_option( option ) )
    {
      const auto names = detail::split( filename, " " );

      for ( const auto& name : names )
      {
        try
        {
          const auto element = read<Store, Tag>( name, static_cast<command const&>( *this ) );

          if ( multiple || names.size() > 1 || is_set( "new" ) || env->store<Store>().empty() )
          {
            env->store<Store>().extend();
          }

          env->store<Store>().current() = element;
        }
        catch ( const std::string& error )
        {
          env->err() << "[e] " << error << "\n";
        }
        catch ( ... )
        {
          /* do nothing, user should display error or warning in `read` function */
        }
      }

      env->set_default_option( option );
    }
    return 0;
  }

private:
  std::vector<std::string> filenames;
  std::vector<std::string> allowed_options;
  std::string default_option;
};
}
