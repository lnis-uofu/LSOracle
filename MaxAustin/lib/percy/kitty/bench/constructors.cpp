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

#include <kitty/kitty.hpp>

using namespace kitty;

template<int NumVars>
void BM_create_majority_static( benchmark::State& state )
{
  while ( state.KeepRunning() )
  {
    static_truth_table<NumVars> tt;
    create_majority( tt );
  }
}

void BM_create_majority_dynamic( benchmark::State& state )
{
  while ( state.KeepRunning() )
  {
    dynamic_truth_table tt( state.range( 0 ) );
    create_majority( tt );
  }
}

template<int NumVars>
void BM_create_nth_var_static( benchmark::State& state )
{
  while ( state.KeepRunning() )
  {
    static_truth_table<NumVars> tt;
    create_nth_var( tt, state.range( 0 ) );
  }
}

static void ARGS_create_nth_var_dynamic( benchmark::internal::Benchmark* b )
{
  for ( auto num_vars = 5; num_vars <= 9; num_vars += 2 )
  {
    for ( auto var_index = 0; var_index < num_vars; var_index += 2 )
    {
      b->Args( {num_vars, var_index} );
    }
  }
}

void BM_create_nth_var_dynamic( benchmark::State& state )
{
  while ( state.KeepRunning() )
  {
    dynamic_truth_table tt( state.range( 0 ) );
    create_nth_var( tt, state.range( 1 ) );
  }
}

BENCHMARK_TEMPLATE( BM_create_majority_static, 5 );
BENCHMARK_TEMPLATE( BM_create_majority_static, 7 );
BENCHMARK_TEMPLATE( BM_create_majority_static, 9 );
BENCHMARK( BM_create_majority_dynamic )->Arg( 5 )->Arg( 7 )->Arg( 9 );

BENCHMARK_TEMPLATE( BM_create_nth_var_static, 5 )->Arg( 0 )->Arg( 2 )->Arg( 4 );
BENCHMARK_TEMPLATE( BM_create_nth_var_static, 7 )->Arg( 0 )->Arg( 2 )->Arg( 4 )->Arg( 6 );
BENCHMARK_TEMPLATE( BM_create_nth_var_static, 9 )->Arg( 0 )->Arg( 2 )->Arg( 4 )->Arg( 6 )->Arg( 8 );
BENCHMARK( BM_create_nth_var_dynamic )->Apply( ARGS_create_nth_var_dynamic );

BENCHMARK_MAIN()
