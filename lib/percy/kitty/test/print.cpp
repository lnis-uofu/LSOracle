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

#include <sstream>

#include <kitty/print.hpp>
#include <kitty/static_truth_table.hpp>

#include "utility.hpp"

using namespace kitty;

class PrintTest : public kitty::testing::Test
{
};

TEST_F( PrintTest, print_binary )
{
  EXPECT_EQ( to_binary( from_hex<0>( "0" ) ), "0" );
  EXPECT_EQ( to_binary( from_hex<0>( "1" ) ), "1" );
  EXPECT_EQ( to_binary( from_hex<1>( "1" ) ), "01" );
  EXPECT_EQ( to_binary( from_hex<1>( "2" ) ), "10" );
  EXPECT_EQ( to_binary( from_hex<2>( "8" ) ), "1000" );
  EXPECT_EQ( to_binary( from_hex<3>( "e8" ) ), "11101000" );
  EXPECT_EQ( to_binary( from_hex<7>( "fffefee8fee8e880fee8e880e8808000" ) ), "11111111111111101111111011101000111111101110100011101000100000001111111011101000111010001000000011101000100000001000000000000000" );
}

TEST_F( PrintTest, print_hex )
{
  EXPECT_EQ( to_hex( from_hex<0>( "0" ) ), "0" );
  EXPECT_EQ( to_hex( from_hex<0>( "1" ) ), "1" );
  EXPECT_EQ( to_hex( from_hex<1>( "1" ) ), "1" );
  EXPECT_EQ( to_hex( from_hex<1>( "2" ) ), "2" );
  EXPECT_EQ( to_hex( from_hex<2>( "8" ) ), "8" );
  EXPECT_EQ( to_hex( from_hex<3>( "e8" ) ), "e8" );
  EXPECT_EQ( to_hex( from_hex<7>( "fffefee8fee8e880fee8e880e8808000" ) ), "fffefee8fee8e880fee8e880e8808000" );
}

TEST_F( PrintTest, print_raw )
{
  static_truth_table<8> tt, tt2;

  for ( auto i = 0u; i < 100u; ++i )
  {
    create_random( tt );

    std::stringstream ss;
    print_raw( tt, ss );
    ss.seekg( 0 );

    create_from_raw( tt2, ss );

    EXPECT_EQ( tt, tt2 );
  }
}
