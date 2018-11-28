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

#include <chrono>
#include <random>

#include <benchmark/benchmark.h>

#include <kitty/kitty.hpp>

using namespace kitty;

void BM_for_each_one_bit_naive( benchmark::State& state )
{
  dynamic_truth_table tt( state.range( 0 ) );

  std::default_random_engine gen( std::chrono::system_clock::now().time_since_epoch().count() );
  std::uniform_int_distribution<uint64_t> dist( 0ul, tt.num_bits() - 1 );

  while ( state.KeepRunning() )
  {
    clear( tt );

    for ( auto i = 0u; i < 500u; ++i )
    {
      set_bit( tt, dist( gen ) );
    }
    for_each_one_bit_naive( tt, []( auto ) {} );
  }
}

void BM_for_each_one_bit_jump( benchmark::State& state )
{
  dynamic_truth_table tt( state.range( 0 ) );

  std::default_random_engine gen( std::chrono::system_clock::now().time_since_epoch().count() );
  std::uniform_int_distribution<uint64_t> dist( 0ul, tt.num_bits() - 1 );

  while ( state.KeepRunning() )
  {
    clear( tt );
    for ( auto i = 0u; i < 500u; ++i )
    {
      set_bit( tt, dist( gen ) );
    }
    for_each_one_bit_jump( tt, []( auto ) {} );
  }
}

BENCHMARK( BM_for_each_one_bit_naive )->Arg( 9 )->Arg( 11 )->Arg( 13 )->Arg( 15 )->Arg( 17 )->Arg( 19 )->Arg( 21 )->Arg( 23 )->Arg( 25 )->Arg( 27 );
BENCHMARK( BM_for_each_one_bit_jump )->Arg( 9 )->Arg( 11 )->Arg( 13 )->Arg( 15 )->Arg( 17 )->Arg( 19 )->Arg( 21 )->Arg( 23 )->Arg( 25 )->Arg( 27 );

BENCHMARK_MAIN()
