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

#include <kitty/static_truth_table.hpp>
#include <kitty/implicant.hpp>
#include <kitty/print.hpp>

#include "utility.hpp"

using namespace kitty;

class ImplicantTest : public kitty::testing::Test
{
};

TEST_F( ImplicantTest, get_minterms )
{
  auto minterms = get_minterms( from_hex<3>( "e8") );
  EXPECT_EQ( minterms, ( std::vector<uint32_t>{3u, 5u, 6u, 7u} ) );
}

TEST_F( ImplicantTest, get_jbuddies )
{
  auto minterms = get_minterms( from_hex<3>( "e8" ) );

  auto buddies = get_jbuddies( minterms, 0 );
  EXPECT_EQ( buddies.size(), 1u );
  EXPECT_EQ( *buddies.front().first, 6u );
  EXPECT_EQ( *buddies.front().second, 7u );

  buddies = get_jbuddies( minterms, 1 );
  EXPECT_EQ( buddies.size(), 1u );
  EXPECT_EQ( *buddies.front().first, 5u );
  EXPECT_EQ( *buddies.front().second, 7u );

  buddies = get_jbuddies( minterms, 2 );
  EXPECT_EQ( buddies.size(), 1u );
  EXPECT_EQ( *buddies.front().first, 3u );
  EXPECT_EQ( *buddies.front().second, 7u );
}

TEST_F( ImplicantTest, prime_implicants_morreale )
{
  for ( auto i = 0u; i < 100u; ++i )
  {
    static_truth_table<4> func;
    create_random( func );
    const auto cubes = get_prime_implicants_morreale( func );

    auto tt = func.construct();
    create_from_cubes( tt, cubes );
    EXPECT_EQ( func, tt );
  }

  for ( auto n = 1u; n < 8u; ++n )
  {
    for ( auto i = 0u; i < 10u; ++i )
    {
      dynamic_truth_table func( n );
      create_random( func );
      const auto cubes = get_prime_implicants_morreale( func );

      auto tt = func.construct();
      create_from_cubes( tt, cubes );
      EXPECT_EQ( func, tt );
    }
  }
}
