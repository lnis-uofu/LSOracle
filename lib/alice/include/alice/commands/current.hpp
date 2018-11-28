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
  \file current.hpp
  \brief Switches current data structure

  \author Mathias Soeken
*/

#pragma once

#include "../command.hpp"

namespace alice
{

template<class... S>
class current_command : public command
{
public:
  explicit current_command( const environment::ptr& env )
      : command( env, "Switches current data structure" )
  {
    add_option( "index,--index", index, "new index" );

    []( ... ) {}( add_option_helper<S>( opts )... );
  }

protected:
  rules validity_rules() const
  {
    rules rules;

    rules.push_back( {[this]() { (void)this; return env->has_default_option() || exactly_one_true_helper<bool>( {is_set( store_info<S>::option )...} ); }, "exactly one store needs to be specified"} );

    return rules;
  }

  void execute()
  {
    []( ... ) {}( set_current_index<S>()... );
  }

private:
  template<typename Store>
  int set_current_index()
  {
    constexpr auto option = store_info<Store>::option;

    if ( ( is_set( option ) || env->is_default_option( option ) ) && index < store<Store>().size() )
    {
      store<Store>().set_current_index( index );
      env->set_default_option( option );
    }
    return 0;
  }

private:
  unsigned index;
};
}
