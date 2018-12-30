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
void BM_bitwise_and_lambda_static( benchmark::State& state )
{
  while ( state.KeepRunning() )
  {
    static_truth_table<NumVars> tt1, tt2;
    binary_operation( tt1, tt2, []( const auto& a, const auto& b ) { return a & b; } );
  }
}

template<int NumVars>
void BM_bitwise_and_std_static( benchmark::State& state )
{
  while ( state.KeepRunning() )
  {
    static_truth_table<NumVars> tt1, tt2;
    binary_operation( tt1, tt2, std::bit_and<uint64_t>() );
  }
}

template<int NumVars>
void BM_bitwise_and_func_static( benchmark::State& state )
{
  while ( state.KeepRunning() )
  {
    static_truth_table<NumVars> tt1, tt2;
    binary_and( tt1, tt2 );
  }
}

template<int NumVars>
void BM_bitwise_majority_func_static( benchmark::State& state )
{
  while ( state.KeepRunning() )
  {
    static_truth_table<NumVars> tt1, tt2, tt3;
    ternary_majority( tt1, tt2, tt3 );
  }
}

void BM_bitwise_and_lambda_dynamic( benchmark::State& state )
{
  while ( state.KeepRunning() )
  {
    dynamic_truth_table tt1( state.range( 0 ) ), tt2( state.range( 0 ) );
    binary_operation( tt1, tt2, []( const auto& a, const auto& b ) { return a & b; } );
  }
}

void BM_bitwise_and_std_dynamic( benchmark::State& state )
{
  while ( state.KeepRunning() )
  {
    dynamic_truth_table tt1( state.range( 0 ) ), tt2( state.range( 0 ) );
    binary_operation( tt1, tt2, std::bit_and<uint64_t>() );
  }
}

void BM_bitwise_and_func_dynamic( benchmark::State& state )
{
  while ( state.KeepRunning() )
  {
    dynamic_truth_table tt1( state.range( 0 ) ), tt2( state.range( 0 ) );
    binary_and( tt1, tt2 );
  }
}

void BM_bitwise_majority_func_dynamic( benchmark::State& state )
{
  while ( state.KeepRunning() )
  {
    dynamic_truth_table tt1( state.range( 0 ) ), tt2( state.range( 0 ) ), tt3( state.range( 0 ) );
    ternary_majority( tt1, tt2, tt3 );
  }
}

void BM_unary_not_ite_dynamic( benchmark::State& state )
{
  while ( state.KeepRunning() )
  {
    dynamic_truth_table tt( state.range( 0 ) );
    const auto tt_new = !get_bit( tt, 0 ) ? unary_not( tt ) : tt;
  }
}

void BM_unary_not_if_dynamic( benchmark::State& state )
{
  while ( state.KeepRunning() )
  {
    dynamic_truth_table tt( state.range( 0 ) );
    const auto tt_new = unary_not_if( tt, get_bit( tt, 0 ) );
  }
}

BENCHMARK_TEMPLATE( BM_bitwise_and_lambda_static, 5 );
BENCHMARK_TEMPLATE( BM_bitwise_and_lambda_static, 7 );
BENCHMARK_TEMPLATE( BM_bitwise_and_lambda_static, 9 );
BENCHMARK_TEMPLATE( BM_bitwise_and_std_static, 5 );
BENCHMARK_TEMPLATE( BM_bitwise_and_std_static, 7 );
BENCHMARK_TEMPLATE( BM_bitwise_and_std_static, 9 );
BENCHMARK_TEMPLATE( BM_bitwise_and_func_static, 5 );
BENCHMARK_TEMPLATE( BM_bitwise_and_func_static, 7 );
BENCHMARK_TEMPLATE( BM_bitwise_and_func_static, 9 );
BENCHMARK_TEMPLATE( BM_bitwise_majority_func_static, 5 );
BENCHMARK_TEMPLATE( BM_bitwise_majority_func_static, 7 );
BENCHMARK_TEMPLATE( BM_bitwise_majority_func_static, 9 );

BENCHMARK( BM_bitwise_and_lambda_dynamic )->Arg( 5 )->Arg( 7 )->Arg( 9 );
BENCHMARK( BM_bitwise_and_std_dynamic )->Arg( 5 )->Arg( 7 )->Arg( 9 );
BENCHMARK( BM_bitwise_and_func_dynamic )->Arg( 5 )->Arg( 7 )->Arg( 9 );
BENCHMARK( BM_bitwise_majority_func_dynamic )->Arg( 5 )->Arg( 7 )->Arg( 9 );

BENCHMARK( BM_unary_not_ite_dynamic )->Arg( 5 )->Arg( 7 )->Arg( 9 );
BENCHMARK( BM_unary_not_if_dynamic )->Arg( 5 )->Arg( 7 )->Arg( 9 );

BENCHMARK_MAIN()
