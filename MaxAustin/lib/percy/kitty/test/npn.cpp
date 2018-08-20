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

#include <iostream>
#include <string>
#include <vector>

#include <kitty/constructors.hpp>
#include <kitty/npn.hpp>
#include <kitty/static_truth_table.hpp>

#include "utility.hpp"

using namespace kitty;

class NPNTest : public kitty::testing::Test
{
protected:
  template<int NumVars>
  void check_npn( const std::string& hex_func, const std::string& hex_expect ) const
  {
    const auto tt = from_hex<NumVars>( hex_func );
    const auto res = exact_npn_canonization( tt );
    EXPECT_EQ( std::get<0>( res ), from_hex<NumVars>( hex_expect ) );
    EXPECT_EQ( create_from_npn_config( res ), tt );
  }
};

TEST_F( NPNTest, random_functions_exact )
{
  using pair_vec_t = std::vector<std::pair<std::string, std::string>>;

  for ( const auto& p : pair_vec_t{{"0", "0"}, {"1", "0"}} )
  {
    check_npn<0>( p.first, p.second );
  }

  for ( const auto& p : pair_vec_t{{"0", "0"}, {"1", "1"}, {"2", "1"}, {"3", "0"}} )
  {
    check_npn<1>( p.first, p.second );
  }

  for ( const auto& p : pair_vec_t{{"0", "0"}, {"1", "1"}, {"2", "1"}, {"3", "3"}, {"4", "1"}, {"5", "3"}, {"6", "6"}, {"7", "1"}, {"8", "1"}, {"9", "6"}, {"a", "3"}, {"b", "1"}, {"c", "3"}, {"d", "1"}, {"e", "1"}, {"f", "0"}} )
  {
    check_npn<2>( p.first, p.second );
  }

  check_npn<3>( "5b", "19" );
  check_npn<3>( "dc", "07" );
  check_npn<3>( "2e", "1b" );

  check_npn<4>( "cafe", "011b" );
  check_npn<6>( "17cad20f55459c3f", "001b674bdf9ca7c3" );
}

TEST_F( NPNTest, random_functions_flip_swap )
{
  kitty::static_truth_table<8> tt;

  for ( auto i = 0; i < 1000; ++i )
  {
    create_random( tt );
    const auto res = flip_swap_npn_canonization( tt );
    EXPECT_EQ( create_from_npn_config( res ), tt );
  }
}

TEST_F( NPNTest, random_functions_sifting )
{
  kitty::static_truth_table<8> tt;

  for ( auto i = 0; i < 1000; ++i )
  {
    create_random( tt );
    const auto res = sifting_npn_canonization( tt );
    EXPECT_EQ( create_from_npn_config( res ), tt );
  }
}
