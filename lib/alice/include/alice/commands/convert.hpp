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
  \file convert.hpp
  \brief Convert store element into element of a different store

  \author Mathias Soeken
*/

#pragma once

#include "../command.hpp"

namespace alice
{

template<typename D, typename S>
int add_combination_helper_inner( CLI::App& opts )
{
  if ( can_convert<S, D>() )
  {
    constexpr auto source_option = store_info<S>::option;
    constexpr auto dest_option = store_info<D>::option;

    constexpr auto source_name = store_info<S>::name;
    constexpr auto dest_name = store_info<D>::name;

    opts.add_flag( fmt::format( "--{}_to_{}", source_option, dest_option ),
                   fmt::format( "convert {} to {}", source_name, dest_name ) );
  }
  return 0;
};

template<typename D, class... S>
int add_combination_helper( CLI::App& opts )
{
  []( ... ) {}( add_combination_helper_inner<D, S>( opts )... );
  return 0;
};

template<typename D, class S>
int convert_helper_inner( const environment::ptr& env, const command& cmd )
{
  if ( can_convert<S, D>() )
  {
    constexpr auto source_option = store_info<S>::option;
    constexpr auto dest_option = store_info<D>::option;

    if ( cmd.is_set( fmt::format( "{}_to_{}", source_option, dest_option ) ) )
    {
      constexpr auto source_name = store_info<S>::name;
      const auto& source_store = env->store<S>();

      if ( source_store.current_index() == -1 )
      {
        std::cout << fmt::format( "[w] there is no {} to convert from", source_name ) << std::endl;
        return 0;
      }

      auto& dest_store = env->store<D>();
      dest_store.extend();
      dest_store.current() = convert<S, D>( source_store.current() );
    }
  }
  return 0;
}

template<typename D, class... S>
int convert_helper( const environment::ptr& env, const command& cmd )
{
  []( ... ) {}( convert_helper_inner<D, S>( env, cmd )... );
  return 0;
}

template<class... S>
class convert_command : public command
{
public:
  explicit convert_command( const environment::ptr& env )
      : command( env, "Convert store element into element of a different store" )
  {
    []( ... ) {}( add_combination_helper<S, S...>( opts )... );
  }

protected:
  void execute()
  {
    []( ... ) {}( convert_helper<S, S...>( env, *this )... );
  }
};
}
