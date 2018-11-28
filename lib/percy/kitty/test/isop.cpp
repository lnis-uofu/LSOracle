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

#include <cstdint>
#include <vector>

#include <gtest/gtest.h>

#include <kitty/constructors.hpp>
#include <kitty/isop.hpp>
#include <kitty/static_truth_table.hpp>

#include "utility.hpp"

using namespace kitty;

class IsopTest : public kitty::testing::Test
{
};

TEST_F( IsopTest, isop_for_small )
{
  const auto tt = from_hex<3>( "e8" );

  const auto cubes = isop( tt );

  EXPECT_EQ( cubes.size(), 3u );
  EXPECT_TRUE( std::find( std::begin( cubes ), std::end( cubes ), cube( 3, 3 ) ) != std::end( cubes ) );
  EXPECT_TRUE( std::find( std::begin( cubes ), std::end( cubes ), cube( 5, 5 ) ) != std::end( cubes ) );
  EXPECT_TRUE( std::find( std::begin( cubes ), std::end( cubes ), cube( 6, 6 ) ) != std::end( cubes ) );

  auto tt_copy = tt.construct();
  create_from_cubes( tt_copy, cubes );
  EXPECT_EQ( tt, tt_copy );
}

TEST_F( IsopTest, random_isop )
{
  static_truth_table<10> tt;

  for ( auto i = 0; i < 1000; ++i )
  {
    create_random( tt );
    const auto cubes = isop( tt );
    auto tt_copy = tt.construct();
    create_from_cubes( tt_copy, cubes );
    EXPECT_EQ( tt, tt_copy );
  }
}
