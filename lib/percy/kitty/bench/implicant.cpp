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

#include <benchmark/benchmark.h>

#include <kitty/constructors.hpp>
#include <kitty/dynamic_truth_table.hpp>
#include <kitty/implicant.hpp>

using namespace kitty;

void BM_minterms( benchmark::State& state )
{
  dynamic_truth_table tt( state.range( 0 ) );
  create_majority( tt );

  while ( state.KeepRunning() )
  {
    get_minterms( tt );
  }
}

void BM_jbuddies( benchmark::State& state )
{
  dynamic_truth_table tt( state.range( 0 ) );
  create_majority( tt );

  const auto minterms = get_minterms( tt );

  while ( state.KeepRunning() )
  {
    for ( auto i = 0; i < state.range( 0 ); ++i )
    {
      get_jbuddies( minterms, 0 );
    }
  }
}

BENCHMARK( BM_minterms )->Arg( 9 )->Arg( 11 )->Arg( 13 )->Arg( 15 );
BENCHMARK( BM_jbuddies )->Arg( 9 )->Arg( 11 )->Arg( 13 )->Arg( 15 );

BENCHMARK_MAIN()
