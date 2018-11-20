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

#include <chrono>
#include <iostream>
#include <numeric>
#include <random>

#include <kitty/affine.hpp>
#include <kitty/constructors.hpp>
#include <kitty/permutation.hpp>
#include <kitty/print.hpp>
#include <kitty/static_truth_table.hpp>

using namespace kitty;

TEST( PermutationTest, small_permutations_static )
{
  static_truth_table<3> base;
  std::vector<uint32_t> perm{0u, 2u, 3u, 5u, 7u, 1u, 4u, 6u};

  const auto masks = compute_permutation_masks( base, perm );
  EXPECT_EQ( masks.size(), 5u );

  for ( auto i = 0u; i < 1000u; ++i )
  {
    auto f = base.construct();
    create_random( f );

    const auto f_p = permute_with_masks( f, masks );

    for ( auto i = 0u; i < perm.size(); ++i )
    {
      EXPECT_EQ( get_bit( f, perm[i] ), get_bit( f_p, i ) );
    }
  }
}

TEST( PermutationTest, random_permutation_masks )
{
  constexpr const auto n = 8u;

  /* create random truth table by random swaps */
  std::vector<uint32_t> perm( 1 << n );
  std::iota( perm.begin(), perm.end(), 0u );

  std::default_random_engine gen( std::chrono::system_clock::now().time_since_epoch().count() );
  std::uniform_int_distribution<std::size_t> dist( 0u, perm.size() - 1 );

  for ( auto i = 0u; i < perm.size(); ++i )
  {
    std::swap( perm[dist( gen )], perm[dist( gen )] );
  }

  static_truth_table<n> base;
  const auto masks = compute_permutation_masks( base, perm );
  EXPECT_EQ( masks.size(), 2 * n - 1 );

  for ( auto i = 0u; i < 10u; ++i )
  {
    auto f = base.construct();
    create_random( f );

    const auto f_p = permute_with_masks( f, masks );

    for ( auto i = 0u; i < perm.size(); ++i )
    {
      EXPECT_EQ( get_bit( f, perm[i] ), get_bit( f_p, i ) );
    }
  }
}