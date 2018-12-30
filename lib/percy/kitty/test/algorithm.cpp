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

#include <algorithm>
#include <queue>

#include <kitty/algorithm.hpp>
#include <kitty/dynamic_truth_table.hpp>
#include <kitty/static_truth_table.hpp>

#include "utility.hpp"

using namespace kitty;

class AlgorithmTest : public kitty::testing::Test
{
};

TEST_F( AlgorithmTest, for_each_static_large )
{
  static_truth_table<9> tt;
  create_majority( tt );

  std::deque<uint64_t> blocks;
  for_each_block( tt, [&blocks]( auto block ) { blocks.push_back( block ); } );
  EXPECT_TRUE( std::equal( tt._bits.begin(), tt._bits.end(), blocks.begin(), blocks.end() ) );

  blocks.clear();
  for_each_block_reversed( tt, [&blocks]( auto block ) { blocks.push_front( block ); } );
  EXPECT_TRUE( std::equal( tt._bits.begin(), tt._bits.end(), blocks.begin(), blocks.end() ) );
}

TEST_F( AlgorithmTest, for_each_dynamic_large )
{
  dynamic_truth_table tt( 9 );
  create_majority( tt );

  std::deque<uint64_t> blocks;
  for_each_block( tt, [&blocks]( auto block ) { blocks.push_back( block ); } );
  EXPECT_TRUE( std::equal( tt._bits.begin(), tt._bits.end(), blocks.begin(), blocks.end() ) );

  blocks.clear();
  for_each_block_reversed( tt, [&blocks]( auto block ) { blocks.push_front( block ); } );
  EXPECT_TRUE( std::equal( tt._bits.begin(), tt._bits.end(), blocks.begin(), blocks.end() ) );
}

TEST_F( AlgorithmTest, for_each_static_small )
{
  static_truth_table<5> tt;
  create_majority( tt );

  std::deque<uint64_t> blocks;
  for_each_block( tt, [&blocks]( auto block ) { blocks.push_back( block ); } );
  EXPECT_TRUE( blocks.size() == 1 );
  EXPECT_EQ( blocks.front(), tt._bits );

  blocks.clear();
  for_each_block_reversed( tt, [&blocks]( auto block ) { blocks.push_front( block ); } );
  EXPECT_TRUE( blocks.size() == 1 );
  EXPECT_EQ( blocks.front(), tt._bits );
}

TEST_F( AlgorithmTest, for_each_dynamic_small )
{
  dynamic_truth_table tt( 5 );
  create_majority( tt );

  std::deque<uint64_t> blocks;
  for_each_block( tt, [&blocks]( auto block ) { blocks.push_back( block ); } );
  EXPECT_TRUE( blocks.size() == 1 );
  EXPECT_EQ( blocks.front(), tt._bits.front() );

  blocks.clear();
  for_each_block_reversed( tt, [&blocks]( auto block ) { blocks.push_front( block ); } );
  EXPECT_TRUE( blocks.size() == 1 );
  EXPECT_EQ( blocks.front(), tt._bits.front() );
}

TEST_F( AlgorithmTest, for_each_one_bit_static_small )
{
  static_truth_table<3> tt;
  create_majority( tt );

  std::vector<uint64_t> bits;
  for_each_one_bit( tt, [&bits]( auto bit ) { bits.push_back( bit ); } );
  EXPECT_EQ( bits, std::vector<uint64_t>( {3, 5, 6, 7} ) );
}

TEST_F( AlgorithmTest, for_each_one_bit_dynamic_small )
{
  dynamic_truth_table tt( 3 );
  create_majority( tt );

  std::vector<uint64_t> bits;
  for_each_one_bit( tt, [&bits]( auto bit ) { bits.push_back( bit ); } );
  EXPECT_EQ( bits, std::vector<uint64_t>( {3, 5, 6, 7} ) );
}

TEST_F( AlgorithmTest, for_each_one_bit_static_large )
{
  static_truth_table<9> tt, tt2;
  create_majority( tt );

  for_each_one_bit( tt, [&tt2]( auto bit ) { set_bit( tt2, bit ); } );

  EXPECT_EQ( tt, tt2 );
}

TEST_F( AlgorithmTest, for_each_one_bit_dynamic_large )
{
  dynamic_truth_table tt( 9 ), tt2( 9 );
  create_majority( tt );

  for_each_one_bit( tt, [&tt2]( auto bit ) { set_bit( tt2, bit ); } );

  EXPECT_EQ( tt, tt2 );
}

