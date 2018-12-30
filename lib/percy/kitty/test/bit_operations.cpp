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

#include <kitty/bit_operations.hpp>
#include <kitty/dynamic_truth_table.hpp>
#include <kitty/static_truth_table.hpp>

#include "utility.hpp"

using namespace kitty;

class BitOperationsTest : public kitty::testing::Test
{
};

template<typename NumVars>
class BitOperationsTestT : public ::testing::Test
{
};

typedef ::testing::Types<static_truth_table<5>, static_truth_table<7>, static_truth_table<9>> truth_table_types;
TYPED_TEST_CASE( BitOperationsTestT, truth_table_types );

TYPED_TEST( BitOperationsTestT, all_initially_zero )
{
  TypeParam tt_s;

  for ( auto i = 0u; i < tt_s.num_bits(); ++i )
  {
    EXPECT_EQ( get_bit( tt_s, i ), 0u );
  }

  dynamic_truth_table tt_d( tt_s.num_vars() );

  for ( auto i = 0u; i < tt_d.num_bits(); ++i )
  {
    EXPECT_EQ( get_bit( tt_d, i ), 0u );
  }
}

TYPED_TEST( BitOperationsTestT, set_get_clear )
{
  TypeParam tt_s;

  for ( auto i = 0u; i < tt_s.num_bits(); ++i )
  {
    set_bit( tt_s, i );
    EXPECT_EQ( get_bit( tt_s, i ), 1u );
    clear_bit( tt_s, i );
    EXPECT_EQ( get_bit( tt_s, i ), 0u );
  }

  dynamic_truth_table tt_d( tt_s.num_vars() );

  for ( auto i = 0u; i < tt_d.num_bits(); ++i )
  {
    set_bit( tt_d, i );
    EXPECT_EQ( get_bit( tt_d, i ), 1u );
    clear_bit( tt_d, i );
    EXPECT_EQ( get_bit( tt_d, i ), 0u );
  }
}

TEST_F( BitOperationsTest, find_first_bit )
{
  EXPECT_EQ( find_first_one_bit( from_hex<3>( "00" ) ), -1 );
  EXPECT_EQ( find_first_one_bit( ~from_hex<3>( "00" ) ), 0 );

  EXPECT_EQ( find_first_one_bit( nth<6>( 0 ) ), 1 );
  EXPECT_EQ( find_first_one_bit( nth<6>( 1 ) ), 2 );
  EXPECT_EQ( find_first_one_bit( nth<6>( 2 ) ), 4 );
  EXPECT_EQ( find_first_one_bit( nth<6>( 3 ) ), 8 );
  EXPECT_EQ( find_first_one_bit( nth<6>( 4 ) ), 16 );
  EXPECT_EQ( find_first_one_bit( nth<6>( 5 ) ), 32 );
  EXPECT_EQ( find_first_one_bit( nth<7>( 6 ) ), 64 );
  EXPECT_EQ( find_first_one_bit( nth<8>( 7 ) ), 128 );
}

TEST_F( BitOperationsTest, find_last_bit )
{
  EXPECT_EQ( find_last_one_bit( from_hex<3>( "00" ) ), -1 );
  EXPECT_EQ( find_last_one_bit( ~from_hex<3>( "00" ) ), 7 );

  EXPECT_EQ( find_last_one_bit( nth<6>( 0 ) ), 63 );
  EXPECT_EQ( find_last_one_bit( nth<6>( 1 ) ), 63 );
  EXPECT_EQ( find_last_one_bit( nth<6>( 2 ) ), 63 );
  EXPECT_EQ( find_last_one_bit( nth<6>( 3 ) ), 63 );
  EXPECT_EQ( find_last_one_bit( nth<6>( 4 ) ), 63 );
  EXPECT_EQ( find_last_one_bit( nth<6>( 5 ) ), 63 );
  EXPECT_EQ( find_last_one_bit( nth<7>( 6 ) ), 127 );
  EXPECT_EQ( find_last_one_bit( nth<8>( 7 ) ), 255 );
}

TEST_F( BitOperationsTest, find_first_bit_difference )
{
  EXPECT_EQ( find_first_bit_difference( nth<6>( 0 ), nth<6>( 0 ) ), find_first_one_bit( nth<6>( 0 ) ^ nth<6>( 0 ) ) );
  EXPECT_EQ( find_first_bit_difference( nth<6>( 1 ), nth<6>( 0 ) ), find_first_one_bit( nth<6>( 1 ) ^ nth<6>( 0 ) ) );
  EXPECT_EQ( find_first_bit_difference( nth<6>( 2 ), nth<6>( 1 ) ), find_first_one_bit( nth<6>( 2 ) ^ nth<6>( 1 ) ) );
  EXPECT_EQ( find_first_bit_difference( nth<6>( 3 ), nth<6>( 2 ) ), find_first_one_bit( nth<6>( 3 ) ^ nth<6>( 2 ) ) );
  EXPECT_EQ( find_first_bit_difference( nth<6>( 4 ), nth<6>( 3 ) ), find_first_one_bit( nth<6>( 4 ) ^ nth<6>( 3 ) ) );
  EXPECT_EQ( find_first_bit_difference( nth<6>( 5 ), nth<6>( 4 ) ), find_first_one_bit( nth<6>( 5 ) ^ nth<6>( 4 ) ) );
  EXPECT_EQ( find_first_bit_difference( nth<7>( 6 ), nth<7>( 5 ) ), find_first_one_bit( nth<7>( 6 ) ^ nth<7>( 5 ) ) );
  EXPECT_EQ( find_first_bit_difference( nth<8>( 7 ), nth<8>( 6 ) ), find_first_one_bit( nth<8>( 7 ) ^ nth<8>( 6 ) ) );
}

TEST_F( BitOperationsTest, find_last_bit_difference )
{
  EXPECT_EQ( find_last_bit_difference( nth<6>( 0 ), nth<6>( 0 ) ), find_last_one_bit( nth<6>( 0 ) ^ nth<6>( 0 ) ) );
  EXPECT_EQ( find_last_bit_difference( nth<6>( 1 ), nth<6>( 0 ) ), find_last_one_bit( nth<6>( 1 ) ^ nth<6>( 0 ) ) );
  EXPECT_EQ( find_last_bit_difference( nth<6>( 2 ), nth<6>( 1 ) ), find_last_one_bit( nth<6>( 2 ) ^ nth<6>( 1 ) ) );
  EXPECT_EQ( find_last_bit_difference( nth<6>( 3 ), nth<6>( 2 ) ), find_last_one_bit( nth<6>( 3 ) ^ nth<6>( 2 ) ) );
  EXPECT_EQ( find_last_bit_difference( nth<6>( 4 ), nth<6>( 3 ) ), find_last_one_bit( nth<6>( 4 ) ^ nth<6>( 3 ) ) );
  EXPECT_EQ( find_last_bit_difference( nth<6>( 5 ), nth<6>( 4 ) ), find_last_one_bit( nth<6>( 5 ) ^ nth<6>( 4 ) ) );
  EXPECT_EQ( find_last_bit_difference( nth<7>( 6 ), nth<7>( 5 ) ), find_last_one_bit( nth<7>( 6 ) ^ nth<7>( 5 ) ) );
  EXPECT_EQ( find_last_bit_difference( nth<8>( 7 ), nth<8>( 6 ) ), find_last_one_bit( nth<8>( 7 ) ^ nth<8>( 6 ) ) );
}

TEST_F( BitOperationsTest, count_ones_small )
{
  static_truth_table<5> tt;

  for ( auto i = 0u; i < 100u; ++i )
  {
    create_random( tt );

    auto ctr = 0u;
    for ( uint64_t i = 0u; i < tt.num_bits(); ++i )
    {
      ctr += get_bit( tt, i );
    }

    EXPECT_EQ( ctr, count_ones( tt ) );
  }
}

TEST_F( BitOperationsTest, count_ones_large )
{
  static_truth_table<9> tt;

  for ( auto i = 0u; i < 100u; ++i )
  {
    create_random( tt );

    auto ctr = 0u;
    for ( uint64_t i = 0u; i < tt.num_bits(); ++i )
    {
      ctr += get_bit( tt, i );
    }

    EXPECT_EQ( ctr, count_ones( tt ) );
  }
}
