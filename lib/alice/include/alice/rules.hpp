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
  \file rules.hpp
  \brief Some useful validity rules

  \author Mathias Soeken
*/

#pragma once

#include <fmt/format.h>

#include "command.hpp"
#include "store.hpp"

namespace alice
{
template<typename S>
command::rule has_store_element( const environment::ptr& env )
{
  auto constexpr name = store_info<S>::name;
  return { [&]() { return env->store<S>().current_index() >= 0; }, fmt::format( "no current {} available", name ) };
}

template<typename S>
command::rule has_store_element_if_set( const command& cmd, const environment::ptr& env, const std::string& argname )
{
  auto constexpr name = store_info<S>::name;
  return { [&cmd, &env, argname]() { return !cmd.is_set( argname ) || env->store<S>().current_index() >= 0; }, fmt::format( "no current {} available", name ) };
}
}
