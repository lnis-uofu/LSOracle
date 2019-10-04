#include <alice/alice.hpp>

#include <mockturtle/algorithms/cleanup.hpp>
#include <mockturtle/algorithms/cut_rewriting.hpp>
#include <mockturtle/algorithms/node_resynthesis.hpp>
#include <mockturtle/algorithms/node_resynthesis/akers.hpp>
#include <mockturtle/algorithms/node_resynthesis/direct.hpp>
#include <mockturtle/algorithms/node_resynthesis/mig_npn.hpp>
#include <mockturtle/algorithms/node_resynthesis/xag_npn.hpp>
#include <mockturtle/algorithms/mig_algebraic_rewriting.hpp>

#include <stdio.h>
#include <fstream>

#include <sys/stat.h>
#include <stdlib.h>


namespace alice
{   
  // using aig_names = mockturtle::mapping_view<mockturtle::names_view<mockturtle::aig_network>, true>;
  // using aig_ntk = std::shared_ptr<aig_names>;
  // using part_man_aig = oracle::partition_manager<aig_names>;
  // using part_man_aig_ntk = std::shared_ptr<part_man_aig>;

  // using mig_names = mockturtle::mapping_view<mockturtle::names_view<mockturtle::mig_network>, true>;
  // using mig_ntk = std::shared_ptr<mig_names>;
  // using part_man_mig = oracle::partition_manager<mig_names>;
  // using part_man_mig_ntk = std::shared_ptr<part_man_mig>;

  using aig_names = mockturtle::names_view<mockturtle::aig_network>;
  using aig_ntk = std::shared_ptr<aig_names>;
  using part_man_aig = oracle::partition_manager<aig_names>;
  using part_man_aig_ntk = std::shared_ptr<part_man_aig>;

  using mig_names = mockturtle::names_view<mockturtle::mig_network>;
  using mig_ntk = std::shared_ptr<mig_names>;
  using part_man_mig = oracle::partition_manager<mig_names>;
  using part_man_mig_ntk = std::shared_ptr<part_man_mig>;

  /***************************************************
    Truth Table
  ***************************************************/

  // Helper function to flip the bit
  char flip(char c){
    return (c == '0') ? '1' : '0';
  }

  std::string to_binary(int dec){

    std::string bin;
    while(dec != 0){
      bin = (dec % 2 == 0 ? "0":"1") + bin;
      dec /= 2;
    }
    return bin;
  }

  std::string graytoBinary(std::string gray){
    std::string binary = "";

    binary += gray[0];

    for (int i = 1; i < gray.length(); i++) {
      if (gray[i] == '0')
        binary += binary[i - 1];
      else
        binary += flip(binary[i - 1]);
    }

    return binary;
  }

  // Function to convert binary to decimal
  int binaryToDecimal(int n){

    int num = n;
    int dec_value = 0;

    // Initializing base value to 1, i.e 2^0
    int base = 1;

    int temp = num;
    while (temp){

      int last_digit = temp % 10;
      temp = temp/10;

      dec_value += last_digit*base;

      base = base*2;
    }

    return dec_value;
  }

  /***************************************************/

  /***************************************************
    Network conversion
  ***************************************************/
  mig_ntk aig_to_mig(aig_names aig, int skip_edge_min){

    using NtkSource = aig_names;
    using NtkDest = mig_names;
    mockturtle::mig_network ntk;
    NtkDest mig( ntk );

    mockturtle::node_map<mockturtle::mig_network::signal, mockturtle::aig_network> node2new( aig );

    node2new[aig.get_node( aig.get_constant( false ) )] = mig.get_constant( false );
    if ( aig.get_node( aig.get_constant( true ) ) != aig.get_node( aig.get_constant( false ) ) ){

      node2new[aig.get_node( aig.get_constant( true ) )] = mig.get_constant( true );
    }

    aig.foreach_pi( [&]( auto n ) {
      if(aig.is_ro(n)){
        mig._storage->data.latches.emplace_back(0);
      }
      node2new[n] = mig.create_pi();

      if constexpr ( mockturtle::has_has_name_v<NtkSource> && mockturtle::has_get_name_v<NtkSource> && mockturtle::has_set_name_v<NtkDest> )
      {
        if ( aig.has_name( aig.make_signal( n ) ) )
          mig.set_name( node2new[n], aig.get_name( aig.make_signal( n ) ) );
      }
    } );
        
    aig.foreach_node( [&]( auto n ) {
      if ( aig.is_constant( n ) || aig.is_pi( n ) || aig.is_ci( n ) || aig.is_ro( n ))
        return;

      std::vector<mockturtle::mig_network::signal> children;
      aig.foreach_fanin( n, [&]( auto const& f ) {
        children.push_back( aig.is_complemented( f ) ? mig.create_not( node2new[f] ) : node2new[f] );
      } );

      if(skip_edge_min == 1){
        node2new[n] = mig.create_maj_part(mig.get_constant( false ), children.at(0), children.at(1));
      }
      else{
        node2new[n] = mig.create_maj(mig.get_constant( false ), children.at(0), children.at(1));
      }
      
      if constexpr ( mockturtle::has_has_name_v<NtkSource> && mockturtle::has_get_name_v<NtkSource> && mockturtle::has_set_name_v<NtkDest> )
      {
        if ( aig.has_name( aig.make_signal( n ) ) )
          mig.set_name( node2new[n], aig.get_name( aig.make_signal( n ) ) );
      }
          
    } );

    /* map primary outputs */
    aig.foreach_po( [&]( auto const& f, auto index ) {
      mig.create_po( aig.is_complemented( f ) ? mig.create_not( node2new[f] ) : node2new[f] );

      if constexpr ( mockturtle::has_has_output_name_v<NtkSource> && mockturtle::has_get_output_name_v<NtkSource> && mockturtle::has_set_output_name_v<NtkDest> )
      {
        if ( aig.has_output_name( index ) )
        {
          mig.set_output_name( index, aig.get_output_name( index ) );
        }
      }
    } );

    return std::make_shared<mig_names>( mig );
  }

  mig_ntk part_to_mig(oracle::partition_view<mig_names> part, int skip_edge_min){
    mockturtle::mig_network mig;

    std::unordered_map<mockturtle::mig_network::node, mockturtle::mig_network::signal> node2new;

    node2new[part.get_node( part.get_constant( false ) )] = mig.get_constant( false );
    if ( part.get_node( part.get_constant( true ) ) != part.get_node( part.get_constant( false ) ) ){

      node2new[part.get_node( part.get_constant( true ) )] = mig.get_constant( true );
    }

    part.foreach_pi( [&]( auto n ) {
      // std::cout << "PI node = " << n << "\n";
      node2new[n] = mig.create_pi();
    } );
    // std::cout << "created all PIs\n";
        
    part.foreach_node( [&]( auto n ) {
      // std::cout << "Node = " << n << "\n";
      if ( part.is_constant( n ) || part.is_pi( n ) || part.is_ci( n ) || part.is_ro( n ))
        return;

      std::vector<mockturtle::mig_network::signal> children;
      // std::cout << "before foreach_fanin\n";
      part.foreach_fanin( n, [&]( auto const& f ) {
        // std::cout << "before pushing " << f.index << " to children\n";
        children.push_back( part.is_complemented( f ) ? mig.create_not( node2new[part.get_node(f)] ) : node2new[part.get_node(f)] );
        // std::cout << "after pushing to children\n";
      } );
      // std::cout << "after foreach_fanin\n";

      // if(children.size() == 0){
      //   node2new[n] = mig.create_pi();
      // }
      // else{
        if(skip_edge_min == 1){
          node2new[n] = mig.create_maj_part(children.at(0), children.at(1), children.at(2));
        }
        else{
          node2new[n] = mig.create_maj(children.at(0), children.at(1), children.at(2));
        }
      // }
      
      // std::cout << "created majority\n";
    } );
    // std::cout << "completed nodes\n";
    /* map primary outputs */
    part.foreach_po( [&]( auto const& f ) {
      mig.create_po( part.is_complemented( f ) ? mig.create_not( node2new[part.get_node(f)] ) : node2new[part.get_node(f)] );
    } );
    // std::cout << "created POs\n";

    return std::make_shared<mig_names>( mig );
  }

  aig_ntk mig_to_aig(mig_names mig){
    using NtkSource = mig_names;
    using NtkDest = aig_names;
    mockturtle::aig_network ntk;
    NtkDest aig( ntk );

    mockturtle::node_map<mockturtle::aig_network::signal, mockturtle::mig_network> node2new( mig );

    node2new[mig.get_node( mig.get_constant( false ) )] = aig.get_constant( false );
    if ( mig.get_node( mig.get_constant( true ) ) != mig.get_node( mig.get_constant( false ) ) ){

      node2new[mig.get_node( mig.get_constant( true ) )] = aig.get_constant( true );
    }

    mig.foreach_pi( [&]( auto n ) {
      node2new[n] = aig.create_pi();

      if constexpr ( mockturtle::has_has_name_v<NtkSource> && mockturtle::has_get_name_v<NtkSource> && mockturtle::has_set_name_v<NtkDest> )
      {
        if ( mig.has_name( mig.make_signal( n ) ) )
          aig.set_name( node2new[n], mig.get_name( mig.make_signal( n ) ) );
      }
    } );
    
    std::set<mockturtle::mig_network::node> nodes_to_change;    
    mig.foreach_node( [&]( auto n ) {

      if ( mig.is_constant( n ) || mig.is_pi( n ) || mig.is_ci( n ) || mig.is_ro( n ))
        return;

      std::vector<mockturtle::aig_network::signal> children;

      if(mig._storage->nodes[n].children[0].data != 0){
        mockturtle::mig_network::signal child1 = mig._storage->nodes[n].children[1];
        mockturtle::mig_network::signal child2 = mig._storage->nodes[n].children[2];
        children.push_back(mig.is_complemented( child1 ) ? node2new[child1] : aig.create_not( node2new[child1] ));
        children.push_back(mig.is_complemented( child2 ) ? node2new[child2] : aig.create_not( node2new[child2] ));
        nodes_to_change.insert(n);
      }
      else{
        for(int i = 1; i < mig._storage->nodes[n].children.size(); i++){
          auto node = mig.get_node(mig._storage->nodes[n].children[i]);
          mockturtle::mig_network::signal child = mig._storage->nodes[n].children[i];
          if(nodes_to_change.find(node) != nodes_to_change.end()){
            
            children.push_back(mig.is_complemented( child ) ? node2new[child] : aig.create_not( node2new[child] ));
          }
          else{
            
            children.push_back(mig.is_complemented( child ) ? aig.create_not( node2new[child] ): node2new[child] );
          }
        }
      }
      node2new[n] = aig.create_and(children.at(0), children.at(1));

      if constexpr ( mockturtle::has_has_name_v<NtkSource> && mockturtle::has_get_name_v<NtkSource> && mockturtle::has_set_name_v<NtkDest> )
      {
        if ( mig.has_name( mig.make_signal( n ) ) )
          aig.set_name( node2new[n], mig.get_name( mig.make_signal( n ) ) );
      }
          
    } );

    /* map primary outputs */
    mig.foreach_po( [&]( auto const& f, auto index ) {
      aig.create_po( mig.is_complemented( f ) ? aig.create_not( node2new[f] ) : node2new[f] );

      if constexpr ( mockturtle::has_has_output_name_v<NtkSource> && mockturtle::has_get_output_name_v<NtkSource> && mockturtle::has_set_output_name_v<NtkDest> )
      {
        if ( mig.has_output_name( index ) )
        {
          aig.set_output_name( index, mig.get_output_name( index ) );
        }
      }
    } );

    return std::make_shared<aig_names>( aig );
  }

  /***************************************************/

  /***************************************************
    Network helpers
  ***************************************************/

  bool is_in_vector(std::vector<int> vec, int nodeIdx){

    if(std::find(vec.begin(),vec.end(),nodeIdx) != vec.end())
      return true;
    else
      return false;
  }

  int get_output_index(mockturtle::aig_network aig, int nodeIdx){

    assert(aig.is_po(nodeIdx));

    for(int i = 0; i < aig._storage->outputs.size(); i++){
      if(aig._storage->outputs.at(i).index == nodeIdx){
        return i;

      }
    }
  }

  std::vector<int> get_output_indeces(mockturtle::aig_network aig, int nodeIdx){

    assert(aig.is_po(nodeIdx));
    std::vector<int> indeces;
    for(int i = 0; i < aig._storage->outputs.size(); i++){
      if(aig._storage->outputs.at(i).index == nodeIdx){
        indeces.push_back(i);
      }
    }
    return indeces;
  }//get_output_indeces()

  /***************************************************/

  int get_index(std::vector<int> index, int nodeIdx){

    std::vector<int>::iterator it = find(index.begin(), index.end(), nodeIdx);
    return std::distance(index.begin(), it);
  }   

  /* Checks to see if a file has a specified extension
   * 
   * params:
   * filename: The name of the file to check the extension of
   * ext: The extension that you are checking that filename has
   *
   */
  bool checkExt(std::string filename, std::string ext){

    if(filename.substr(filename.find_last_of(".") + 1) == ext) {
      return true;
    } else {
      return false;
    }
  }//end checkExt
}