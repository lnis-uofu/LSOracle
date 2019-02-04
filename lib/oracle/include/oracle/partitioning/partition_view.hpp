/* mockturtle: C++ logic network library
 * Copyright (C) 2018  EPFL
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
  \file window_view.hpp
  \brief Implements an isolated view on a window in a network
  \author Heinz Riener
*/

#pragma once

#include <algorithm>
#include <cstdint>
#include <unordered_map>
#include <vector>
#include <set>
#include <cassert>

#include <mockturtle/traits.hpp>
#include <mockturtle/networks/detail/foreach.hpp>
#include <mockturtle/views/fanout_view.hpp>

namespace oracle
{

/*! \brief Implements an isolated view on a window in a network.
 *
 */
    // template<typename Ntk, bool window_interface = has_traverse_v<Ntk>>
    // class window_view
    // {
    // };

    template<typename Ntk>
    class partition_view : public Ntk
    {
    public:
        using storage = typename Ntk::storage;
        using node = typename Ntk::node;
        using signal = typename Ntk::signal;

    public:

        partition_view(){
            add_node(get_node(get_constant(false)));
        }

        explicit partition_view( Ntk const& ntk, std::set<node> const& leaves, std::set<node> const& pivots, bool auto_extend = true )
                : Ntk( ntk )
        {
            static_assert( mockturtle::is_network_type_v<Ntk>, "Ntk is not a network type" );
            static_assert( mockturtle::has_set_visited_v<Ntk>, "Ntk does not implement the set_visited method" );
            static_assert( mockturtle::has_visited_v<Ntk>, "Ntk does not implement the visited method" );
            static_assert( mockturtle::has_get_node_v<Ntk>, "Ntk does not implement the get_node method" );
            static_assert( mockturtle::has_get_constant_v<Ntk>, "Ntk does not implement the get_constant method" );
            static_assert( mockturtle::has_is_constant_v<Ntk>, "Ntk does not implement the is_constant method" );
            static_assert( mockturtle::has_make_signal_v<Ntk>, "Ntk does not implement the make_signal method" );

            /* constants */
            add_node( this->get_node( this->get_constant( false ) ) );
            this->set_visited( this->get_node( this->get_constant( false ) ), 1 );
            if ( this->get_node( this->get_constant( true ) ) != this->get_node( this->get_constant( false ) ) )
            {
                add_node( this->get_node( this->get_constant( true ) ) );
                this->set_visited( this->get_node( this->get_constant( true ) ), 1 );
                ++_num_constants;
            }

            /* primary inputs */
            for ( auto const& leaf : leaves )
            {
                if ( this->visited( leaf ) == 1 )
                    continue;

                add_node( leaf );
                this->set_visited( leaf, 1 );
                ++_num_leaves;
            }

            for ( auto const& p : pivots )
            {
                traverse( p );
            }

            // if ( auto_extend )
            // {
            //     extend( ntk );
            // }

            // update_fanin( ntk );
            add_roots( ntk );

            /* restore visited */
            for ( auto const& n : _nodes )
            {
                this->set_visited( n, 0 );
            }
        }

        inline auto size() const { return static_cast<uint32_t>( _nodes.size() ); }
        inline signal create_pi( std::string const& name = {} )
          {
            std::cout << "part_view create pi\n";
            (void)name;
            const auto index = _nodes.size();
            auto& node = _nodes.emplace_back();
            ++_num_leaves;
            return {index, 0};
          } 
        inline uint32_t create_po( signal const& f, std::string const& name = {} )
          {
            (void)name;

            auto const po_index = _roots.size();
            _roots.emplace_back( f.index, f.complement );
            return po_index;
          }
        inline signal create_not( signal const& a )
          {
            return !a;
          }
        inline signal get_constant( bool value ) const
          {
            return {0, static_cast<uint64_t>( value ? 1 : 0 )};
          }
        inline node get_node( signal const& f ) const
          {
            return f.index;
          }
        inline signal create_and( signal a, signal b )
          {
            /* order inputs */
            if ( a.index > b.index )
            {
              std::swap( a, b );
            }

            /* trivial cases */
            if ( a.index == b.index )
            {
              return ( a.complement == b.complement ) ? a : get_constant( false );
            }
            else if ( a.index == 0 )
            {
              return a.complement ? b : get_constant( false );
            }

            node new_node;
            new_node.children[0] = a;
            new_node.children[1] = b;

            const auto index = _nodes.size();

            if ( index >= .9 * _nodes.capacity() )
            {
              _nodes.reserve( static_cast<uint64_t>( 3.1415f * index ) );
            }

            _nodes.push_back( new_node );

            return {index, 0};
          }
        inline signal clone_node( mockturtle::aig_network const& other, node const& source, std::vector<signal> const& children )
          {
            (void)other;
            (void)source;
            assert( children.size() == 2u );
            return create_and( children[0u], children[1u] );
          }
        inline signal clone_node( mockturtle::mig_network const& other, node const& source, std::vector<signal> const& children )
          {
            (void)other;
            (void)source;
            assert( children.size() == 3u );
            return create_maj( children[0u], children[1u], children[2u] );
          }
        inline auto num_pis() const { return _num_leaves; }
        inline auto num_pos() const { return _roots.size(); }
        inline auto num_gates() const { return _nodes.size() - _num_leaves - _num_constants; }

        inline auto node_to_index( const node& n ) const { return _node_to_index.at( n ); }
        inline auto index_to_node( uint32_t index ) const { return _nodes[index]; }

        inline bool is_pi( node const& pi ) const
        {
            const auto beg = _nodes.begin() + _num_constants;
            return std::find( beg, beg + _num_leaves, pi ) != beg + _num_leaves;
        }

        inline bool is_ci( node const& pi ) const
        {
            const auto beg = _nodes.begin() + _num_constants;
            return std::find( beg, beg + _num_leaves, pi ) != beg + _num_leaves;
        }

        inline bool is_constant( node const& c ) const
        {
            const auto beg = _nodes.begin();
            return std::find( beg, beg + _num_constants, c ) != beg + _num_constants;
        }

        template<typename Fn>
        void foreach_pi( Fn&& fn ) const
        {
            mockturtle::detail::foreach_element( _nodes.begin() + _num_constants, _nodes.begin() + _num_constants + _num_leaves, fn );
        }

        template<typename Fn>
        void foreach_po( Fn&& fn ) const
        {
            mockturtle::detail::foreach_element( _roots.begin(), _roots.end(), fn );
        }

        template<typename Fn>
        void foreach_node( Fn&& fn ) const
        {
            mockturtle::detail::foreach_element( _nodes.begin(), _nodes.end(), fn );
        }

        template<typename Fn>
        void foreach_gate( Fn&& fn ) const
        {
            mockturtle::detail::foreach_element( _nodes.begin() + _num_constants + _num_leaves, _nodes.end(), fn );
        }

        // template<typename Fn>
        // void foreach_fanin(node const& n, Fn&& fn) const
        // {
        //     detail::foreach_element( _children[n].begin(), _children[n].end(), fn );
        // }

        uint32_t fanout_size( node const& n ) const
        {
            return _fanout_size.at( node_to_index(n) );
        }

        // void substitute_node( node const& old_node, signal const& new_signal )
        // {
        //     std::cout << "partition view sub node\n";
        //     /* find all parents from old_node */
        //     for ( auto& n : _nodes )
        //     {
        //         for ( auto& child : n.children )
        //         {
        //             if ( child.index == old_node )
        //             {
        //                 child.index = new_signal.index;
        //                 child.weight ^= new_signal.complement;

        //                 // increment fan-in of new node
        //                 _nodes[new_signal.index].data[0].h1++;
        //             }
        //         }
        //     }

        //     /* check outputs */
        //     for ( auto& output : _roots )
        //     {
        //         if ( output.index == old_node )
        //         {
        //             output.index = new_signal.index;
        //             output.weight ^= new_signal.complement;

        //             // increment fan-in of new node
        //             _nodes[new_signal.index].data[0].h1++;
        //         }
        //     }

        //     // reset fan-in of old node
        //     _nodes[old_node].data[0].h1 = 0;
        // }


    private:
        void add_node( node const& n )
        {
            
            _node_to_index[n] = _nodes.size();
            _nodes.push_back( n );

            auto fanout_counter = 0;
            for(int i = 0; i < this->_storage->nodes[n].children.size(); i++){
                auto f = this->_storage->nodes[n].children[i];
                if ( std::find( _nodes.begin(), _nodes.end(), this->get_node( f ) ) != _nodes.end() )
                {
                    fanout_counter++;
                }
            }
            _fanout_size.push_back( fanout_counter );
        }

        void traverse( node const& n )
        {
            
            if ( this->visited( n ) == 1 )
                return;

            for(int i = 0; i < this->_storage->nodes[n].children.size(); i++){
                auto f = this->_storage->nodes[n].children[i];
                traverse( this->get_node( f ) );
            }

            add_node( n );
            this->set_visited( n, 1 );
        }

        // void update_fanin( Ntk const& ntk )
        // {
        //     std::cout << "update_fanin\n";
        //     foreach_gate ([&]( auto node ){
        //         std::vector<signal> children;
        //         int nodeIdx = node_to_index(node);
        //         std::cout << "nodeIdx = " << nodeIdx << "\n";
        //         for(auto const& child : ntk._storage->nodes[node].children){
        //             std::cout << "original child = " << child.index << "\n";
        //             std::cout << "original child data = " << child.data << "\n";
        //             auto orig_node = ntk.index_to_node(child.index);
        //             auto part_node = node_to_index(orig_node);
        //             auto sig = ntk.make_signal(part_node);
        //             if(child.data & 1){
        //                 std::cout << "orig child inverted\n";
        //                 sig = ntk.create_not(sig);
        //             }
        //             std::cout << "child in partition = " << sig.index << "\n";
        //             std::cout << "child in partition data = " << sig.data << "\n";
        //             children.push_back(sig);
        //         }
        //         _children[node] = children;
        //     });
        // }

        // void extend( Ntk const& ntk )
        // {
        //     std::set<node> new_nodes;
        //     do
        //     {
        //         new_nodes.clear();
        //         for ( const auto& n : _nodes )
        //         {
        //             ntk.foreach_fanout( n, [&]( auto const& p ){
        //                 /* skip node if it is already in _nodes */
        //                 if ( std::find( _nodes.begin(), _nodes.end(), p ) != _nodes.end() ) return;

        //                 auto all_children_in_nodes = true;

        //                 ntk.foreach_fanin( p, [&]( auto const& s ){
        //                     auto const& child = ntk.get_node( s );
        //                     if ( std::find( _nodes.begin(), _nodes.end(), child ) == _nodes.end() )
        //                     {
        //                         all_children_in_nodes = false;
        //                         return false;
        //                     }
        //                     return true;
        //                 });

        //                 if ( all_children_in_nodes )
        //                 {
        //                     assert( p != 0 );
        //                     assert( !is_pi( p ) );
        //                     new_nodes.insert( p );
        //                 }
        //             });
        //         }

        //         for ( const auto& n : new_nodes )
        //         {
        //             add_node( n );
        //         }
        //     } while ( !new_nodes.empty() );
        // }

        void add_roots( Ntk const& ntk )
        {
            /* compute po nodes */
            std::vector<node> pos;
            ntk.foreach_po( [&]( auto const& s ){
                pos.push_back( ntk.get_node( s ) );
            });
            /* compute partition outputs */
            for ( const auto& n : _nodes )
            {
                if ( std::find( pos.begin(), pos.end(), n ) != pos.end() )
                {
                    auto s = this->make_signal( n );
                    if ( std::find( _roots.begin(), _roots.end(), s ) == _roots.end() )
                    {
                        _roots.push_back( s );
                    }
                    continue;
                }
                mockturtle::fanout_view fanout{ntk};
                fanout.foreach_fanout( n, [&]( auto const& p ){
                    if(!is_constant(n))
                    {
                        if ( std::find( _nodes.begin(), _nodes.end(), p ) == _nodes.end() )
                        {
                            auto s = this->make_signal( n );
                            if ( std::find( _roots.begin(), _roots.end(), s ) == _roots.end() )
                            {
                                _roots.push_back( s );
                                return false;
                            }
                        }
                        return true;
                    }
                    
                });
            }
        }

    public:
        unsigned _num_constants{1};
        unsigned _num_leaves{0};
        std::vector<node> _nodes;
        std::unordered_map<node, std::vector<signal>> _children;
        std::unordered_map<node, uint32_t> _node_to_index;
        std::vector<signal> _roots;
        std::vector<unsigned> _fanout_size;
    };

} /* namespace oracle */