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
#include <unordered_set>

#include <kitty/affine.hpp>
#include <kitty/constructors.hpp>
#include <kitty/hash.hpp>
#include <kitty/print.hpp>
#include <kitty/static_truth_table.hpp>

#include "utility.hpp"

using namespace kitty;

template<typename TT>
using class_func_t = std::function<TT(const TT&)>;

class AffineTest : public kitty::testing::Test
{
protected:
  template<int NumVars>
  uint64_t canonization( const class_func_t<static_truth_table<NumVars>>& class_func ) const
  {
    std::unordered_set<static_truth_table<NumVars>, hash<static_truth_table<NumVars>>> funcs;

    for ( auto i = 0u; i < ( uint64_t( 1 ) << ( 1 << NumVars ) ); ++i )
    {
      static_truth_table<NumVars> tt;
      create_from_words( tt, &i, &i + 1 );
      funcs.insert( class_func( tt ) );
    }

    return funcs.size();
  }

  uint64_t canonization( unsigned num_vars, const class_func_t<dynamic_truth_table>& class_func ) const
  {
    std::unordered_set<dynamic_truth_table, hash<dynamic_truth_table>> funcs;

    for ( auto i = 0u; i < ( uint64_t( 1 ) << ( 1 << num_vars ) ); ++i )
    {
      dynamic_truth_table tt( num_vars );
      create_from_words( tt, &i, &i + 1 );
      funcs.insert( class_func( tt ) );
    }

    return funcs.size();
  }
};

TEST_F( AffineTest, count_linear_static )
{
  EXPECT_EQ( canonization<2>( exact_linear_canonization<static_truth_table<2>> ), 8u );
  EXPECT_EQ( canonization<3>( exact_linear_canonization<static_truth_table<3>> ), 20u );
}

TEST_F( AffineTest, count_linear_output_static )
{
  EXPECT_EQ( canonization<2>( exact_linear_output_canonization<static_truth_table<2>> ), 4u );
  EXPECT_EQ( canonization<3>( exact_linear_output_canonization<static_truth_table<3>> ), 10u );
}

TEST_F( AffineTest, count_affine_static )
{
  EXPECT_EQ( canonization<2>( exact_affine_canonization<static_truth_table<2>> ), 5u );
  EXPECT_EQ( canonization<3>( exact_affine_canonization<static_truth_table<3>> ), 10u );
}

TEST_F( AffineTest, count_affine_output_static )
{
  EXPECT_EQ( canonization<2>( exact_affine_output_canonization<static_truth_table<2>> ), 3u );
  EXPECT_EQ( canonization<3>( exact_affine_output_canonization<static_truth_table<3>> ), 6u );
}

TEST_F( AffineTest, count_affine_output_dynamic )
{
  EXPECT_EQ( canonization( 2u, exact_affine_output_canonization<dynamic_truth_table> ), 3u );
  EXPECT_EQ( canonization( 3u, exact_affine_output_canonization<dynamic_truth_table> ), 6u );
}

TEST_F( AffineTest, all_two_affine_dynamic )
{
  std::unordered_set<dynamic_truth_table, hash<dynamic_truth_table>> functions;

  for ( auto i = 0; i < 16; ++i )
  {
    dynamic_truth_table tt( 2 );
    create_from_words( tt, &i, &i + 1 );

    const auto cls = exact_affine_output_canonization( tt );

    functions.insert( cls );
  }

  EXPECT_EQ( functions.size(), 3u );

  EXPECT_TRUE( functions.count( from_hex( 2, "0" ) ) );
  EXPECT_TRUE( functions.count( from_hex( 2, "1" ) ) );
  EXPECT_TRUE( functions.count( from_hex( 2, "3" ) ) );
}

TEST_F( AffineTest, all_three_affine_static )
{
  std::unordered_set<static_truth_table<3>, hash<static_truth_table<3>>> functions;

  for ( auto i = 0; i < ( 1 << 8 ); ++i )
  {
    static_truth_table<3> tt;
    create_from_words( tt, &i, &i + 1 );

    const auto cls = exact_affine_output_canonization( tt );

    functions.insert( cls );
  }

  EXPECT_TRUE( functions.count( from_hex<3>( "00" ) ) );
  EXPECT_TRUE( functions.count( from_hex<3>( "01" ) ) );
  EXPECT_TRUE( functions.count( from_hex<3>( "03" ) ) );
  EXPECT_TRUE( functions.count( from_hex<3>( "07" ) ) );
  EXPECT_TRUE( functions.count( from_hex<3>( "0f" ) ) );
  EXPECT_TRUE( functions.count( from_hex<3>( "17" ) ) );
}

TEST_F( AffineTest, all_three_affine_dynamic )
{
  std::unordered_set<dynamic_truth_table, hash<dynamic_truth_table>> functions;

  for ( auto i = 0; i < ( 1 << 8 ); ++i )
  {
    dynamic_truth_table tt( 3 );
    create_from_words( tt, &i, &i + 1 );

    const auto cls = exact_affine_output_canonization( tt );

    functions.insert( cls );
  }

  EXPECT_TRUE( functions.count( from_hex( 3, "00" ) ) );
  EXPECT_TRUE( functions.count( from_hex( 3, "01" ) ) );
  EXPECT_TRUE( functions.count( from_hex( 3, "03" ) ) );
  EXPECT_TRUE( functions.count( from_hex( 3, "07" ) ) );
  EXPECT_TRUE( functions.count( from_hex( 3, "0f" ) ) );
  EXPECT_TRUE( functions.count( from_hex( 3, "17" ) ) );
}

TEST_F( AffineTest, DISABLED_all_four_affine_static )
{
  std::unordered_set<static_truth_table<4>, hash<static_truth_table<4>>> functions;

  for ( auto i = 0; i < ( 1 << 16 ); ++i )
  {
    static_truth_table<4> tt;
    create_from_words( tt, &i, &i + 1 );

    const auto cls = exact_affine_output_canonization( tt );

    functions.insert( cls );
  }

  EXPECT_EQ( functions.size(), 18u );

  EXPECT_TRUE( functions.count( from_hex<4>( "0000" ) ) );
  EXPECT_TRUE( functions.count( from_hex<4>( "0001" ) ) );
  EXPECT_TRUE( functions.count( from_hex<4>( "0003" ) ) );
  EXPECT_TRUE( functions.count( from_hex<4>( "0007" ) ) );
  EXPECT_TRUE( functions.count( from_hex<4>( "000f" ) ) );
  EXPECT_TRUE( functions.count( from_hex<4>( "0017" ) ) );
  EXPECT_TRUE( functions.count( from_hex<4>( "001f" ) ) );
  EXPECT_TRUE( functions.count( from_hex<4>( "003f" ) ) );
  EXPECT_TRUE( functions.count( from_hex<4>( "007f" ) ) );
  EXPECT_TRUE( functions.count( from_hex<4>( "00ff" ) ) );
  EXPECT_TRUE( functions.count( from_hex<4>( "0117" ) ) );
  EXPECT_TRUE( functions.count( from_hex<4>( "011f" ) ) );
  EXPECT_TRUE( functions.count( from_hex<4>( "013f" ) ) );
  EXPECT_TRUE( functions.count( from_hex<4>( "017f" ) ) );
  EXPECT_TRUE( functions.count( from_hex<4>( "033f" ) ) );
  EXPECT_TRUE( functions.count( from_hex<4>( "0356" ) ) );
  EXPECT_TRUE( functions.count( from_hex<4>( "0357" ) ) );
  EXPECT_TRUE( functions.count( from_hex<4>( "035f" ) ) );
}
