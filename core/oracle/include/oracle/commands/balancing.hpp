/*!
  \file balancing.hpp
  \brief Balancing
  \author Max Austin
*/
#pragma once

#include <iostream>
#include <vector>
#include <set>

#include <mockturtle/networks/detail/foreach.hpp>
#include <mockturtle/traits.hpp>
#include <mockturtle/views/topo_view.hpp>

namespace oracle
{

  namespace detail
  {

    template<class Ntk>
    class balancing_impl
    {
      public:
        using storage = typename Ntk::storage;
        using node = typename Ntk::node;
        using signal = typename Ntk::signal;

      balancing_impl( Ntk& ntk )
          : ntk( ntk ) {}

      void run()
      {
        std::cout << "running\n";
        std::cout << "ntk size = " << ntk.size() << "\n";
        ntk.foreach_po([&](auto conn, auto i){
          std::cout << "PO: " << conn.index << "\n";
          struct sub_tree_node* min_delay = min_delay_clusters(conn);
          std::cout << "postorder = {";
          postorder(min_delay);
          std::cout << "}\n";
          std::cout << "bfs = {";
          bfs(min_delay);
          std::cout << "}\n";
          Ntk balanced_ntk;

          balanced_ntk = create_ntk(min_delay, balanced_ntk);
          // std::cout << "new network: {";
          // for( node curr_node : min_delay ){
          //   std::cout << curr_node << " ";
          // }
          // std::cout << "}\n";
          std::cout << "Done\n";
        });

      }

    private:

      struct sub_tree_node{
        uint32_t nodeIdx;
        uint32_t op;
        sub_tree_node *left;
        sub_tree_node *right;
      };

      

      // const bool compare_sub_trees( sub_tree_node* sub_tree1, sub_tree_node* sub_tree2 ){

      //   return get_depth(sub_tree1, 1) < get_depth(sub_tree2, 1);
      // }

      sub_tree_node* create_new_sub_tree( uint32_t nodeIdx ){
        sub_tree_node* sub_tree = (sub_tree_node*)malloc(sizeof(sub_tree_node));

        sub_tree->nodeIdx = nodeIdx;
        sub_tree->op = NULL;

        sub_tree->left = NULL;
        sub_tree->right = NULL;

        return sub_tree;
      }

      sub_tree_node* create_new_sub_tree( uint32_t op, sub_tree_node* left, sub_tree_node* right){
        sub_tree_node* sub_tree = (sub_tree_node*)malloc(sizeof(sub_tree_node));

        sub_tree->nodeIdx = 666;
        sub_tree->op = op;

        sub_tree->left = left;
        sub_tree->right = right;
        
        return sub_tree;
      }

      sub_tree_node* min_delay_clusters( signal curr_out ){
        std::cout << "Current root = " << curr_out.index << "\n";
        std::vector<sub_tree_node*> node_list;
        if(ntk.is_pi(ntk.get_node(curr_out))){
          return create_new_sub_tree(curr_out.index);
        }
        std::set<signal> clusters = cover(curr_out);
        for ( signal cluster : clusters ){
          std::cout << "current cluster = " << cluster.index << "\n";
          node_list.push_back(min_delay_clusters(cluster));
        }

        std::cout << "Q = {";
        for ( sub_tree_node* curr_tree : node_list ){
          std::cout << curr_tree->nodeIdx << " ";
        }
        std::cout << "}\n";

        while( node_list.size() > 1 ){

          struct {
            int get_depth (sub_tree_node* sub_tree, int level){
              if(sub_tree == NULL)
                return level;

              int leftlevel = get_depth(sub_tree->left, level + 1);
              int rightlevel = get_depth(sub_tree->right, level + 1);

              if(leftlevel > rightlevel)
                return leftlevel;
              else
                return rightlevel;
            }

            bool operator()(sub_tree_node* sub_tree1, sub_tree_node* sub_tree2){
              return get_depth(sub_tree1, 1) < get_depth(sub_tree2, 1);
            }
          } compare_sub_trees;

          std::sort(node_list.begin(), node_list.end(), compare_sub_trees);
          sub_tree_node* x = node_list.at(0);
          std::cout << "node X = " << x->nodeIdx << "\n";
          sub_tree_node* y = node_list.at(1);
          std::cout << "node Y = " << y->nodeIdx << "\n";
          node_list.erase(node_list.begin(), node_list.begin() + 2);
          sub_tree_node* sub_tree = create_new_sub_tree(ntk.is_complemented(curr_out), x, y);
          node_list.push_back(sub_tree);
          std::cout << "node created\n";
        }

        std::cout << "returning min_delay_clusters\n";
        return node_list.back();
        
      }

      std::set<signal> cover( signal curr_out ){
        std::set<signal> left_cluster;
        std::set<signal> right_cluster;

        std::set<signal> final_cluster;
        node curr_node = ntk.get_node(curr_out);
        std::cout << "run cluster on " << curr_out.index << " with polarity data: " << ntk.is_complemented(curr_out) << "\n";
        // final_cluster.insert(curr_node);

        signal left_child = ntk._storage->nodes[curr_node].children[0];
        signal right_child = ntk._storage->nodes[curr_node].children[1];

        if(ntk.is_complemented(curr_out) == ntk.is_complemented(left_child) && !ntk.is_pi(ntk.get_node(left_child)))
          left_cluster = cover(left_child);
        else
          left_cluster.insert(left_child);
        
        
        if(ntk.is_complemented(curr_out) == ntk.is_complemented(right_child) && !ntk.is_pi(ntk.get_node(right_child)))
          right_cluster = cover(right_child);
        else
          right_cluster.insert(right_child);
        
        
        std::set_union(left_cluster.begin(), left_cluster.end(), right_cluster.begin(), 
          right_cluster.end(), std::inserter(final_cluster, final_cluster.begin()));

        std::cout << "cluster nodes = {";
        for( signal dat_node : final_cluster ){
          std::cout << dat_node.index << " ";
        }
        std::cout << "}\n";
        return final_cluster;
      }

      void postorder (sub_tree_node* sub_tree ){
        if(sub_tree == NULL)
          return;

        postorder(sub_tree->left);
        postorder(sub_tree->right);
        std::cout << sub_tree->nodeIdx << " ";
      }

      void bfs (sub_tree_node* sub_tree){
        int level = 1;
        if(sub_tree == NULL)
          return;

        std::queue<sub_tree_node*> q;

        q.push(sub_tree);

        while(q.empty() == false){
          sub_tree_node* curr_tree = q.front();
          std::cout << curr_tree->nodeIdx << " with op " << curr_tree->op << " ";
          q.pop();
          if(curr_tree->left != NULL)
            q.push(curr_tree->left);
          if(curr_tree->right != NULL)
            q.push(curr_tree->right);
        }
      }

      Ntk create_ntk(sub_tree_node*, Ntk& balanced_ntk){
        
      }

    private:
      Ntk& ntk;
    };

  } /* namespace detail */

  /*! \brief AND balancing.
   *
   */
  template<class Ntk>
  void balancing( Ntk& ntk )
  {
    static_assert( mockturtle::is_network_type_v<Ntk>, "Ntk is not a network type" );
    static_assert( mockturtle::has_get_node_v<Ntk>, "Ntk does not implement the get_node method" );
    static_assert( mockturtle::has_size_v<Ntk>, "Ntk does not implement the size method" );
    static_assert( mockturtle::has_make_signal_v<Ntk>, "Ntk does not implement the make_signal method" );
    static_assert( mockturtle::has_foreach_gate_v<Ntk>, "Ntk does not implement the foreach_gate method" );
    static_assert( mockturtle::has_substitute_node_v<Ntk>, "Ntk does not implement the substitute_node method" );
    static_assert( mockturtle::has_clear_visited_v<Ntk>, "Ntk does not implement the clear_visited method" );
    static_assert( mockturtle::has_clear_values_v<Ntk>, "Ntk does not implement the clear_values method" );
    static_assert( mockturtle::has_fanout_size_v<Ntk>, "Ntk does not implement the fanout_size method" );
    static_assert( mockturtle::has_set_value_v<Ntk>, "Ntk does not implement the set_value method" );
    static_assert( mockturtle::has_foreach_node_v<Ntk>, "Ntk does not implement the foreach_node method" );

    std::cout << "Doing balancing implementation\n";
    detail::balancing_impl<Ntk> p( ntk );
    p.run();

  }

} /* namespace oracle */
