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
  \file store.hpp
  \brief Store management

  \author Mathias Soeken
*/

#pragma once

#include <vector>

#include <fmt/format.h>

#include "../command.hpp"

namespace alice
{

template<class... S>
class store_command : public command
{
public:
  explicit store_command( const environment::ptr& env )
      : command( env, "Store management" )
  {
    add_flag( "--show", "show contents" );
    add_flag( "--clear", "clear contents" );
    add_flag( "--pop", "pop current element" );

    []( ... ) {}( add_option_helper<S>( opts )... );
  }

protected:
  rules validity_rules() const
  {
    return {
        {[this]() { return static_cast<unsigned>( is_set( "show" ) ) + static_cast<unsigned>( is_set( "clear" ) ) <= 1u; }, "only one operation can be specified"},
        {[this]() { (void)this; return env->has_default_option() || any_true_helper<bool>( {is_set( store_info<S>::option )...} ); }, "no store has been specified"}};
  }

  void execute()
  {
    if ( is_set( "show" ) || ( !is_set( "clear" ) && !( is_set( "pop" ) ) ) )
    {
      []( ... ) {}( show_store<S>()... );
    }
    else if ( is_set( "clear" ) )
    {
      []( ... ) {}( clear_store<S>()... );
    }
    else if ( is_set( "pop" ) )
    {
      []( ... ) {}( pop_store<S>()... );
    }
  }

  nlohmann::json log() const
  {
    nlohmann::json map;
    []( ... ) {}( log_store<S>( map )... );
    return map;
  }

private:
  template<typename Store>
  int show_store()
  {
    constexpr auto option = store_info<Store>::option;
    constexpr auto name = store_info<Store>::name;
    constexpr auto name_plural = store_info<Store>::name_plural;

    const auto& _store = store<Store>();

    if ( is_set( option ) || env->is_default_option( option ) )
    {
      if ( _store.empty() )
      {
        env->out() << fmt::format( "[w] no {} in store", name ) << std::endl;
      }
      else
      {
        env->out() << fmt::format( "[i] {} in store:", name_plural ) << std::endl;
        auto index = 0;
        for ( const auto& element : _store.data() )
        {
          env->out() << fmt::format( "  {} {:2}: ", ( _store.current_index() == index ? '*' : ' ' ), index );
          env->out() << to_string<Store>( element ) << std::endl;
          ++index;
        }
      }

      env->set_default_option( option );
    }

    return 0;
  }

  template<typename Store>
  int clear_store()
  {
    constexpr auto option = store_info<Store>::option;

    if ( is_set( option ) || env->is_default_option( option ) )
    {
      store<Store>().clear();
      env->set_default_option( option );
    }
    return 0;
  }

  template<typename Store>
  int pop_store()
  {
    constexpr auto option = store_info<Store>::option;

    if ( is_set( option ) || env->is_default_option( option ) )
    {
      store<Store>().pop_current();
      env->set_default_option( option );
    }
    return 0;
  }

  template<typename Store>
  int log_store( nlohmann::json& map ) const
  {
    constexpr auto option = store_info<Store>::option;

    if ( is_set( option ) || env->is_default_option( option ) )
    {
      map[option] = store<Store>().current_index();
    }
    return 0;
  }
};
}
