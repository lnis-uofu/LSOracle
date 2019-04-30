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

  mockturtle::mig_network aig_to_mig(mockturtle::aig_network aig){
    mockturtle::mig_network mig;

    mockturtle::node_map<mockturtle::mig_network::signal, mockturtle::aig_network> node2new( aig );

    node2new[aig.get_node( aig.get_constant( false ) )] = mig.get_constant( false );
    if ( aig.get_node( aig.get_constant( true ) ) != aig.get_node( aig.get_constant( false ) ) ){

      node2new[aig.get_node( aig.get_constant( true ) )] = mig.get_constant( true );
    }

    aig.foreach_pi( [&]( auto n ) {
      node2new[n] = mig.create_pi();
    } );
        
    aig.foreach_node( [&]( auto n ) {
      if ( aig.is_constant( n ) || aig.is_pi( n ) || aig.is_ci( n ) || aig.is_ro( n ))
        return;

      std::vector<mockturtle::mig_network::signal> children;
      aig.foreach_fanin( n, [&]( auto const& f ) {
        children.push_back( aig.is_complemented( f ) ? mig.create_not( node2new[f] ) : node2new[f] );
      } );

      node2new[n] = mig.create_maj(mig.get_constant( false ), children.at(0), children.at(1));
          
    } );

    /* map primary outputs */
    aig.foreach_po( [&]( auto const& f ) {
      mig.create_po( aig.is_complemented( f ) ? mig.create_not( node2new[f] ) : node2new[f] );
    } );

    return mig;
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