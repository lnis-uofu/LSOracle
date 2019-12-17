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
  \file print.hpp
  \brief Prints current data structure

  \author Mathias Soeken
*/

#pragma once

#include <sstream>

#include "../command.hpp"

namespace alice
{

template<class... S>
class print_command : public command
{
public:
  explicit print_command( const environment::ptr& env ) : command( env, "Prints current data structure" )
  {
    []( ... ) {}( add_option_helper<S>( opts )... );
  }

protected:
  rules validity_rules() const
  {
    return {
        {[this]() { (void)this; return env->has_default_option() || exactly_one_true_helper<bool>( {is_set( store_info<S>::option )...} ); }, "exactly one store needs to be specified"}};
  }

  void execute()
  {
#if !defined ALICE_PYTHON
    []( ... ) {}( print_store<S>()... );
#endif
  }

#if defined ALICE_PYTHON
  nlohmann::json log() const
  {
    nlohmann::json map;
    []( ... ) {}( log_store<S>( map )... );
    return map;
  }
#endif

private:
  template<typename Store>
  int print_store()
  {
    constexpr auto option = store_info<Store>::option;
    constexpr auto name = store_info<Store>::name;

    if ( is_set( option ) || env->is_default_option( option ) )
    {
      if ( store<Store>().current_index() == -1 )
      {
        env->out() << "[w] no " << name << " in store" << std::endl;
        env->set_default_option( "" );
      }
      else
      {
        print<Store>( env->out(), store<Store>().current() );
        env->set_default_option( option );
      }
    }
    return 0;
  }

  template<typename Store>
  int log_store( nlohmann::json& map ) const
  {
    constexpr auto option = store_info<Store>::option;
    constexpr auto name = store_info<Store>::name;

    if ( is_set( option ) || env->is_default_option( option ) )
    {
      if ( store<Store>().current_index() == -1 )
      {
        map["__repr__"] = fmt::format( "[w] no {} in store", name );
      }
      else
      {
        std::stringstream strs;
        print<Store>( strs, store<Store>().current() );
        map["__repr__"] = strs.str();

        if ( has_html_repr<Store>() )
        {
          map["_repr_html_"] = html_repr<Store>( store<Store>().current() );
        }
      }
    }
    return 0;
  }
};
}
