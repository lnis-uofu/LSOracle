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
#include <sstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include <kitty/cube.hpp>

#include "utility.hpp"

using namespace kitty;

class CubeTest : public kitty::testing::Test
{
protected:
  std::string to_string( const cube& c, uint8_t num_vars )
  {
    std::stringstream ss;
    c.print( num_vars, ss );
    return ss.str();
  }
};

TEST_F( CubeTest, constructing_and_printing )
{
  EXPECT_EQ( to_string( cube(), 2 ), "--" );
  EXPECT_EQ( to_string( cube( 0, 2 ), 2 ), "-0" );
}

TEST_F( CubeTest, construction_from_string )
{
  EXPECT_EQ( to_string( cube( "010" ), 3 ), "010" );
  EXPECT_EQ( to_string( cube( "-1-" ), 3 ), "-1-" );
  EXPECT_EQ( to_string( cube( "*00" ), 3 ), "-00" );

  EXPECT_EQ( to_string( cube( "" ), 0 ), "" );
  EXPECT_EQ( to_string( cube( std::string( 32, '1' ) ), 32 ), std::string( 32, '1' ) ); 
  EXPECT_EQ( to_string( cube( std::string( 33, '1' ) ), 32 ), std::string( 32, '1' ) ); 
}

TEST_F( CubeTest, distance )
{
  EXPECT_EQ( cube().distance( cube( 0, 2 ) ), 1 );
}

TEST_F( CubeTest, pos_neg_cube )
{
  for ( auto i = 0; i <= 32; ++i )
  {
    EXPECT_EQ( to_string( cube::pos_cube( i ), i ), std::string( i, '1' ) );
    EXPECT_EQ( to_string( cube::neg_cube( i ), i ), std::string( i, '0' ) );
  }
}

TEST_F( CubeTest, set_get_clear_cube )
{
  for ( auto i = 1; i <= 32; ++i )
  {
    auto c = cube::neg_cube( i );
    for ( auto j = 0; j < i; ++j )
    {
      EXPECT_EQ( c.get_bit( j ), false );
      c.set_bit( j );
      EXPECT_EQ( c.get_bit( j ), true );
      c.clear_bit( j );
      EXPECT_EQ( c.get_bit( j ), false );

      EXPECT_EQ( c.get_mask( j ), true );
      c.clear_mask( j );
      EXPECT_EQ( c.get_mask( j ), false );
      c.set_mask( j );
      EXPECT_EQ( c.get_mask( j ), true );
    }
  }
}
