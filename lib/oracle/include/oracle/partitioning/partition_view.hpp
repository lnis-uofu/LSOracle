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
#include <unordered_set>
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

            this->clear_visited();
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
            //std::cout << "Window view: Computing PIs\n";
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
           // add_roots( ntk );

            for (auto n : pivots){

                auto sig = this->make_signal(n);
                if(ntk.is_complemented(sig)){
                    sig = ntk.create_not(sig);
                }
                _roots.push_back(sig);
            }

            /* restore visited */
            //std::cout << "Window view: Restoring\n";
            for ( auto const& n : _nodes )
            {
                this->set_visited( n, 0 );
            }
            //std::cout << "Window view done\n";
        }

        inline auto size() const { return static_cast<uint32_t>( _nodes.size() ); }
        inline auto num_pis() const { return _num_leaves; }
        inline auto num_pos() const { return _roots.size(); }
        inline auto num_gates() const { 
            //std::cout << "_nodes.size() = " << _nodes.size() << "\n";
            //std::cout << "_num_leaves = " << _num_leaves << "\n";
            //std::cout << "_num_constants = " << _num_constants << "\n";
            return _nodes.size() - _num_leaves - _num_constants;
        }

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

        uint32_t fanout_size( node const& n ) const
        {
            return _fanout_size.at( node_to_index(n) );
        }

        std::vector<signal> get_children( int nodeIdx ) const
        {
            return _children[nodeIdx];
        }


    private:
//        signal add_node( node const& n )
//        {
//            std::cout << "Window view: Adding node\n";
//            _node_to_index[n] = _nodes.size();
//            _visited[_node_to_index[n]] = 0;
//            _nodes.push_back( n );
//            _nodes_lut.insert(n);
//
//            auto fanout_counter = 0;
//            for(int i = 0; i < this->_storage->nodes[n].children.size(); i++){
//                auto f = this->_storage->nodes[n].children[i];
//                auto search = _nodes_lut.find(n);
//                if ( search != _nodes_lut.end() )
//                {
//                    fanout_counter++;
//                }
//            }
//            _fanout_size.push_back( fanout_counter );
//            //std::cout << _nodes.size() << " added node of index = " << this->make_signal(n).index << "\n";
//            return this->make_signal(n);
//        }

      void add_node( node const& n )
      {
          _node_to_index[n] = _nodes.size();
          _nodes.push_back( n );
          _nodes_lut.insert(n);

          auto fanout_counter = 0;
          this->foreach_fanin( n, [&]( const auto& f ) {
            if ( _nodes_lut.find(this->get_node( f ) ) != _nodes_lut.end() )
            {
                fanout_counter++;
            }
          });
          _fanout_size.push_back( fanout_counter );
      }

        void traverse( node const& n )
        {
            //std::cout << "Window view: Traversing graph\n";
            if ( this->visited( n ) == 1 )
                return;

            this->foreach_fanin( n, [&]( const auto& f ) {
                traverse( this->get_node( f ) );
            } );

            add_node( n );
            // std::cout << "adding node " << this->node_to_index(n) << "\n";
            this->set_visited( n, 1 );
        }

        void update_fanin( Ntk const& ntk )
        {
            foreach_gate ([&]( auto node ){
                std::vector<signal> children;
                int nodeIdx = node_to_index(node);
                // std::cout << "nodeIdx = " << nodeIdx << "\n";
                for(auto const& child : ntk._storage->nodes[node].children){
                    auto orig_node = ntk.index_to_node(child.index);
                    auto part_node = node_to_index(orig_node);
                    auto sig = ntk.make_signal(part_node);
                    if(child.data & 1){
                        sig = ntk.create_not(sig);
                    }
                    children.push_back(sig);
                }
                auto s = signal(node, 0);
                // std::cout << "adding children of " << s.index << "\n";
                _children[nodeIdx] = children;
            });
        }

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

        //void add_roots( Ntk const& ntk )
        //{

         //   for (auto n : pivots)
           //     _roots.push_back(n);
            /* compute po nodes */
//            std::vector<node> pos;
//            std::unordered_set<node> _pos_lut;
//            ntk.foreach_po( [&]( auto const& s ){
//              _pos_lut.insert(ntk.get_node( s ));
//              pos.push_back( ntk.get_node( s ) );
//            });
//
//            std::cout << "Window view: Compute window outputs\n";
//            /* compute window outputs */
//            for ( const auto& n : _nodes )
//            {
//                //if ( ntk.is_constant( n ) || ntk.is_pi( n ) || ntk.is_ro(n)) continue;
//                if(ntk.is_ro(n)) continue;
//                int nodeIdx = node_to_index(n);
//                auto s = this->make_signal( n );
//                auto o = signal(nodeIdx, 0);
//                //std::cout << "First search\n";
//                if ( _pos_lut.find(n) != _pos_lut.end() && _roots_lut.find(s) == _roots_lut.end())
//                {
//                    std::cout << "root at " << _roots.size() << " = " << s.index << "\n";
//                    _roots_lut.insert(s);
//                    _roots.push_back(s);
//                    continue;
//                }
//
//                mockturtle::fanout_view fanout{ntk};
//                //std::cout << "Foreach fanout\n";
//                fanout.foreach_fanout(n, [&](auto const &p) {
//                    if (!ntk.is_constant(n) && !is_pi(n) && !ntk.is_ro(n)) {
//                        auto s = this->make_signal(n);
//                        auto o = signal(nodeIdx, 0);
//                        //std::cout << "Second search\n";
//                        if (_nodes_lut.find(p) == _nodes_lut.end() && _roots_lut.find(s) == _roots_lut.end()) {
//                            //if (  )
//                            //{
//                            std::cout << "root at " << _roots.size() << " = " << s.index << "\n";
//                            _roots.push_back(s);
//                            _roots_lut.insert(s);
//                            //_outputs.push_back(o);
//                            return false;
//                            //}
//                        }
//                    }
//                    return true;
//                });
//            }
        //}
        // void add_roots( Ntk const& ntk )
        // {
        //     // std::cout << "add roots\n";
        //     /* compute po nodes */
        //     std::vector<node> pos;
        //     ntk.foreach_po( [&]( auto const& s ){
        //         pos.push_back( ntk.get_node( s ) );
        //     });
        //     /* compute partition outputs */
        //     for ( const auto& n : _nodes )
        //     {
        //         int nodeIdx = node_to_index(n);
        //         // std::cout << "nodeIdx = " << node_to_index(n) << "\n";
        //         if ( std::find( pos.begin(), pos.end(), n ) != pos.end() )
        //         {
        //             auto s = this->make_signal( n );
        //             auto o = signal(nodeIdx, 0);
        //             if ( std::find( _roots.begin(), _roots.end(), s ) == _roots.end() )
        //             {
        //                 // std::cout << "pushing " << s.index << " to _roots\n";
        //                 _roots.push_back( s );
        //                 // std::cout << "pushing " << o.index << " to _outputs\n";
        //                 _outputs.push_back( o );
        //             }
        //             continue;
        //         }
        //         mockturtle::fanout_view fanout{ntk};
        //         fanout.foreach_fanout( n, [&]( auto const& p ){
        //             if(!is_constant(n) && !is_pi(n))
        //             {
        //                 if ( std::find( _nodes.begin(), _nodes.end(), p ) == _nodes.end() )
        //                 {
        //                     auto s = this->make_signal( n );
        //                     auto o = signal(nodeIdx, 0);
        //                     if ( std::find( _roots.begin(), _roots.end(), s ) == _roots.end() )
        //                     {
        //                         // std::cout << "pushing " << s.index << " to _roots\n";
        //                         _roots.push_back( s );
        //                         // std::cout << "pushing " << o.index << " to _outputs\n";
        //                         _outputs.push_back( o );
        //                         return false;
        //                     }
        //                 }
        //                 return true;
        //             }
                    
        //         });
        //     }
        // }

//        void check_po(){
//            for(int i = 0; i < _outputs.size(); i++){
//                // std::cout << "o
//                utput = " << _outputs.at(i).index << "\n";
//                foreach_fanin( get_node(_outputs.at(i)), [&]( auto const& f ) {
//                    // std::cout << "fanin = " << f.index << "\n";
//                });
//            }
//        }

    public:
        unsigned _num_constants{1};
        unsigned _num_leaves{0};
        Ntk const* ntk;
        std::vector<node> _nodes;
        std::unordered_map<int, uint32_t> _visited;
        std::unordered_map<int, std::vector<signal>> _children;
        std::unordered_map<node, uint32_t> _node_to_index;
        std::vector<signal> _outputs;
        std::vector<signal> _roots;
        std::vector<unsigned> _fanout_size;

        std::unordered_set<node>   _nodes_lut;
        std::unordered_set<signal> _roots_lut;
    };

} /* namespace oracle */