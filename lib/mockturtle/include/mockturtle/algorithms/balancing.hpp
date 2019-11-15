/* mockturtle: C++ logic network library
 * Copyright (C) 2018-2019  EPFL
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

/*!
  \file balancing.hpp
  \brief Balancing
  \author Mathias Soeken
*/

#pragma once

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <stack>
#include <unordered_set>
#include <vector>

#include <fmt/format.h>

#include "../traits.hpp"
#include "../utils/node_map.hpp"
#include "../utils/stopwatch.hpp"

namespace mockturtle
{

struct balancing_params
{
  /*! \brief Update levels after each node substitution.
   *
   * Enabling this flag yields higher precision, but also makes the algorithm
   * slower.
   */
  bool update_levels{true};

  /*! \brief Be verbose. */
  bool verbose{false};
};

struct balancing_stats
{
  /*! \brief Total time. */
  stopwatch<>::duration time_total{0};

  /*! \brief Prints report. */
  void report() const
  {
    std::cout << fmt::format( "[i] total time = {:>5.2f} secs\n", to_seconds( time_total ) );
  }
};

namespace detail
{

template<class Ntk>
class balancing_impl
{
public:
  explicit balancing_impl( Ntk& ntk, balancing_params const& ps, balancing_stats& st ) : ntk( ntk ), ps( ps ), st( st ) {}

  void run()
  {
    stopwatch t( st.time_total );
  
    const auto orig_gates = ntk.num_gates();
    ntk.foreach_gate( [&]( auto const& n, auto i ) {
      if ( i >= orig_gates )
        return false;
      if ( !ntk.is_on_critical_path( n ) )
        return true;

      if constexpr ( has_is_and_v<Ntk> )
      {
        if ( ntk.is_and( n ) )
        {
          restructure_and( n );
        }
      }

      if constexpr ( has_is_xor_v<Ntk> )
      {
        if ( ntk.is_xor( n ) )
        {
        }
      }
      return true;
    } );
  }

private:
  template<bool enabled = has_is_and_v<Ntk>, typename = std::enable_if_t<std::is_same_v<Ntk, Ntk> && enabled>>
  void restructure_and( node<Ntk> const& n )
  {
    unordered_node_map<std::unordered_set<signal<Ntk>>, Ntk> trans_fanin( ntk );

    std::function<std::unordered_set<signal<Ntk>>( node<Ntk> const& )> rec = [&]( auto const& n ) -> std::unordered_set<signal<Ntk>> {
      if ( trans_fanin.has( n ) )
      {
        return trans_fanin[n];
      }

      std::unordered_set<signal<Ntk>> fanin;
      ntk.foreach_fanin( n, [&]( auto const& f ) {
        if ( !ntk.is_and( ntk.get_node( f ) ) || ntk.is_complemented( f ) )
        {
          fanin.insert( f );
        }
        else
        {
          auto gate_fanin = rec( ntk.get_node( f ) );
          std::copy( gate_fanin.begin(), gate_fanin.end(), std::insert_iterator( fanin, fanin.begin() ) );
        }
      } );

      return trans_fanin[n] = fanin;
    };

    const auto& fanin = rec( n );

    std::vector<uint32_t> levels;
    std::vector<std::pair<uint32_t, signal<Ntk>>> level_pairs;
    for ( auto const& s : fanin )
    {
      levels.push_back( ntk.level( ntk.get_node( s ) ) );
      level_pairs.emplace_back( ntk.level( ntk.get_node( s ) ), s );
    }
    std::sort( levels.begin(), levels.end() );
    std::sort( level_pairs.begin(), level_pairs.end(), []( auto const& p1, auto const& p2 ) { return p1.first < p2.first; } );
    const auto old_level = ntk.level( n );
    const auto new_level = depth_from_arrival_times( levels );
    if ( old_level > new_level )
    {
      if ( ps.verbose )
      {
        std::cout << fmt::format( "[i]  old depth = {}, new depth = {}\n", old_level, new_level );
      }
      ntk.substitute_node( n, create_nary_and( level_pairs ) );
      if ( ps.update_levels )
      {
        ntk.update_levels();
      }
    }
  }

  uint32_t depth_from_arrival_times( std::vector<uint32_t> levels )
  {
    // TODO make number of fanin parameter

    // we assume that levels is sorted
    // and that levels has at least one element
    auto it = levels.begin();
    auto dist = 0u;
    while ( std::distance( it, levels.end() ) > 1u )
    {
      auto lvl = *( it + 1 ) + 1; // get level from second input + 1
      levels.insert( std::upper_bound( it, levels.end(), lvl ), lvl );
      dist += 2u;
      it = levels.begin() + dist;
    }
    return *it;
  }

  signal<Ntk> create_nary_and( std::vector<std::pair<uint32_t, signal<Ntk>>> levels )
  {
    auto it = levels.begin();
    auto dist = 0u;
    while ( std::distance( it, levels.end() ) > 1u )
    {
      auto lvl = ( it + 1 )->first + 1;
      auto f = ntk.create_and( it->second, ( it + 1 )->second );
      auto p = std::make_pair( lvl, f );
      levels.insert(
          std::upper_bound( it, levels.end(), p, []( auto const& p1, auto const& p2 ) { return p1.first < p2.first; } ),
          p );
      dist += 2u;
      it = levels.begin() + dist;
    }
    return it->second;
  }

private:
  Ntk& ntk;
  balancing_params const& ps;
  balancing_stats& st;
};

} // namespace detail

template<class Ntk>
void balancing( Ntk& ntk, balancing_params const& ps = {}, balancing_stats *pst = nullptr )
{
  static_assert( is_network_type_v<Ntk>, "Ntk is not a network type" );
  static_assert( has_get_node_v<Ntk>, "Ntk does not implement the get_node method" );
  static_assert( has_is_complemented_v<Ntk>, "Ntk does not implement the is_complemented method" );
  static_assert( has_foreach_gate_v<Ntk>, "Ntk does not implement the foreach_gate method" );
  static_assert( has_level_v<Ntk>, "Ntk does not implement the level method" );

  balancing_stats st;
  detail::balancing_impl<Ntk>( ntk, ps, st ).run();

  if ( ps.verbose )
  {
    st.report();
  }
  if ( pst )
  {
    *pst = st;
  }
}

} // namespace mockturtle