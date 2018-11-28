/* kitty: C++ truth table library
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

#include <gtest/gtest.h>

#include <kitty/constructors.hpp>
#include <kitty/dynamic_truth_table.hpp>
#include <kitty/static_truth_table.hpp>

namespace kitty
{

namespace testing
{

class Test : public ::testing::Test
{
protected:
  template<int NumVars>
  inline static_truth_table<NumVars> nth( uint64_t var_index ) const
  {
    static_truth_table<NumVars> tt;
    create_nth_var( tt, var_index );
    return tt;
  }

  inline dynamic_truth_table nth( uint64_t num_vars, uint64_t var_index ) const
  {
    dynamic_truth_table tt( num_vars );
    create_nth_var( tt, var_index );
    return tt;
  }

  template<int NumVars>
  inline static_truth_table<NumVars> from_hex( const std::string& hex ) const
  {
    static_truth_table<NumVars> tt;
    create_from_hex_string( tt, hex );
    return tt;
  }

  inline dynamic_truth_table from_hex( uint64_t num_vars, const std::string& hex ) const
  {
    dynamic_truth_table tt( num_vars );
    create_from_hex_string( tt, hex );
    return tt;
  }
};
}
}
