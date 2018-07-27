#include <lorina/lorina.hpp>
#include <alice/alice.hpp>
#include <kitty/kitty.hpp>
#include <mockturtle/mockturtle.hpp>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <fmt/format.h>
#include <stdio.h>
#include <stdlib.h>
#include <aig/gia/gia.h>
#include <aig/gia/giaAig.h>
#include <base/wlc/wlc.h>

namespace alice{

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

	/* Adds And-inverter graphs (Mockturtle type aig_network) as store element type to
	 * alice.
	 *
	 * One can access AIGs in general store commands using the long --aig flag or
	 * the short -a flag.
	 */
	ALICE_ADD_STORE( mockturtle::aig_network, "aig", "a", "AIG", "AIGs" )

	/* Implements the short string to describe a store element in store -a */
	ALICE_DESCRIBE_STORE( mockturtle::aig_network, aig ){

	  const auto name = "aig_placeholder";
	  const auto pi_num = aig.num_pis();
	  const auto po_num = aig.num_pos();
	  return fmt::format( "{} i/o = {}/{}", name, pi_num, po_num );
	}//end aig_network describe store

	ALICE_LOG_STORE_STATISTICS( mockturtle::aig_network, aig){

	  return {
	      {"inputs", aig.num_pis()},
	      {"outputs", aig.num_pos()},
	      {"nodes", aig.size()},
	      {"gates", aig.num_gates()}};
	}//end aig_network log store statistics

	/* Implements the functionality of ps -b */
	ALICE_PRINT_STORE_STATISTICS( mockturtle::aig_network, os, aig ){

	  os << "inputs: " << aig.num_pis() << std::endl;
	  os << "outputs: " << aig.num_pos() << std::endl;
	  os << "nodes: " << aig.size() << std::endl;
	  os << "gates: " << aig.num_gates() << std::endl;
	}//end aig_network print store statistics

	ALICE_ADD_STORE(mockturtle::klut_network, "bench", "b", "BENCH", "BENCHes")

	ALICE_DESCRIBE_STORE(mockturtle::klut_network, bench){
	  const auto name = "bench_placeholder";
	  const auto pi_num = bench.num_pis();
	  const auto po_num = bench.num_pos();
	  return fmt::format( "{} i/o = {}/{}", name, pi_num, po_num );
	}//end klut_network describe store

	ALICE_LOG_STORE_STATISTICS( mockturtle::klut_network, bench){

	  return {
	      {"inputs", bench.num_pis()},
	      {"outputs", bench.num_pos()},
	      {"nodes", bench.size()},
	      {"gates", bench.num_gates()}};
	}//end klut_network log store statistics

	/* Implements the functionality of ps -b */
	ALICE_PRINT_STORE_STATISTICS( mockturtle::klut_network, os, bench ){

	  os << "inputs: " << bench.num_pis() << std::endl;
	  os << "outputs: " << bench.num_pos() << std::endl;
	  os << "nodes: " << bench.size() << std::endl;
	  os << "gates: " << bench.num_gates() << std::endl;
	}//end klut_network print store statistics

	/*Reads an blif file and stores the CBA network in a store*/
	ALICE_COMMAND( get_blif, "Input", "Uses the lorina library to read in a blif file" ){

		std::cout << "Not yet fully implemented\n";

		/*std::string filename = "";
		std::cout << "Enter blif filename: ";
		std::cin >> filename;
		read_blif(filename, blif());*/
		/*TODO check to make sure that the file entered is valid and add store*/	
	}//end get_blif

	/*Reads an aig file and stores the AIG network in a store*/
	ALICE_COMMAND( get_aig, "Input", "Uses the lorina library to read in an aig file" ){

		std::string filename = "";
		std::cout << "Enter aig filename: ";
		std::cin >> filename;

		if(checkExt(filename, "aig")){
			mockturtle::aig_network aig;
			lorina::read_aiger(filename, mockturtle::aiger_reader( aig ));
			std::cout << "AIG file = " << filename << " stored" << std::endl;
			store<mockturtle::aig_network>().extend() = aig;

			std::cout << "Child[0] for node 87 = " << aig._storage->nodes[aig.index_to_node(87)].children[0].index << "\n";
			std::cout << "Child[1] for node 87 = " << aig._storage->nodes[aig.index_to_node(87)].children[1].index << "\n";
			std::vector<std::vector<int>> initEdge;
			//Map of each node's respective connection indeces 
			std::map<int, std::vector<int>> connections;

			std::cout << "AIG size = " << aig.size() << "\n";
			std::cout << "AIG input size = " << aig.num_pis() << "\n";
			std::cout << "AIG output size = " << aig.num_pos() << "\n";
			std::cout << "AIG gate size = " << aig.num_gates() << "\n";

			//Initially create edges for every connection in the network
			int i = 0;
			aig.foreach_node( [&]( auto node ) {

				
	  			int fanin = aig.fanin_size(node);

	  			//Used so the gate that goes to a specific output is mapped to the correct
	  			//output node
	  			int outputOffset = 1;
	  			//Add edges for the inputs and outputs
	  			if(fanin == 0){
	  				
	  				std::vector<int> edge;
	  				edge.push_back(aig.node_to_index(node));
	  				initEdge.push_back(edge);
	  				std::cout << "node " << i << " = " << aig.node_to_index(node) << "\n";
	  			}

	  			//Add each input to the gates as edges
	  			for(int j = 0; j < fanin; j++){
	  				
	  				std::vector<int> edge;
	  				int nodeIdx = aig.node_to_index(node);
	  				int childIdx = aig._storage->nodes[node].children[j].index;



	  				//For some reason the indeces for inputs and outputs are off by 1 in the data structure
	  				if(childIdx < (aig.num_pis() + aig.num_pos()))
	  					childIdx--;

	  				//The last gate node is actually the output and should instead be linked to the output node
	  				//which is at the end of the input and output list
	  				if(nodeIdx == (aig.size() - aig.num_pos())){
	  					nodeIdx = (aig.num_pis() + aig.num_pos() - outputOffset);
	  					outputOffset++;
	  				}
	  				
	  				edge.push_back(nodeIdx);
	  				edge.push_back(childIdx);
	  				initEdge.push_back(edge);
		  			std::cout << "node " << i << " child[" << j << "] = " << childIdx << "\n";
		  		}

	  			i++;
			} );

			//Create a list of connections for each node i.e. inputs and outputs
			int ndx = 0;
			while(ndx < initEdge.size()){

				int nodeKey = initEdge.at(ndx).front();
				int connNum = 0;
				std::vector<int> nodeConn;
				for(int i = 0; i < initEdge.size(); i++){

					//Find all instances where the nodeKey is an input to another node and add it to that node's connection vector
					if(std::find(initEdge.at(i).begin(),initEdge.at(i).end(), nodeKey) != initEdge.at(i).end()){

						for(int j = 0; j < initEdge.at(i).size(); j++){

							if(initEdge.at(i).at(j) != nodeKey){
								nodeConn.push_back(initEdge.at(i).at(j));
								connNum++;
							}
						}					
					}
				}

				//Map the vector of connection indeces to the nodeKey
				connections[nodeKey] = nodeConn;
				std::cout << connNum << " connections found for " << nodeKey << "\n";
				std::cout << "Connections = ";

				for(int i = 0; i < connections[nodeKey].size(); i++){
					 std::cout << connections[nodeKey].at(i) << " ";
				}

				std::cout << "\n";
				ndx++;
			}

			aig.foreach_node( [&]( auto node ) {

				int nodeNdx = aig.node_to_index(node);
				int fanin = aig.fanin_size(node);
				for(int i = 0; i < connections[nodeNdx].size(); i++){
					for(int j = 0; j < fanin; j++){

						int childIdx = aig._storage->nodes[node].children[j].index;
						//Decrement the child index if it represents an input or output because
						//the child indeces do not start at 0 in the aig_network data structure
						if(childIdx < (aig.num_pis() + aig.num_pos()))
							childIdx--;

			  			if(connections[nodeNdx].at(i) == childIdx)
			  				connections[nodeNdx].erase(connections[nodeNdx].begin() + i);
			  			
			  		}
			  	}

			  	aig.add_connections_network(connections);
			});
		}
		else{
			std::cout << filename << " is not a valid aig file\n";
		}
	}//end get_aig

	/*Reads a bench file and store the k-LUT network in a store*/
	ALICE_COMMAND( get_bench, "Input", "Uses the lorina library to read in a bench file" ){

		std::string filename = "";
		std::cout << "Enter bench filename: ";
		std::cin >> filename;

		if(checkExt(filename, "bench")){
			mockturtle::klut_network bench;
			lorina::read_bench(filename, mockturtle::bench_reader( bench ));
			std::cout << "Bench file = " << filename << " stored" << std::endl;
			store<mockturtle::klut_network>().extend() = bench;
		}
		else{
			std::cout << filename << " is not a valid bench file\n";
		}	
		
	}//end get_bench

	/*Adds the command "write_to_file" where you can specify what store you are writing a file out from*/
	class write_to_bench_command : public alice::command{

	public:
	  explicit write_to_bench_command( const environment::ptr& env )
	      : command( env, "Writes a stored file to an external bench file" ){

	    opts.add_option( "--filename,filename", filename, "bench file to write to" )->required();
	    add_flag("--aig,-a", "Write the stored AIG file to a bench file");
	    add_flag("--bench,-b", "Write the stored bench file to a bench file");
	  }

	protected:
	  void execute(){

	    if(is_set("aig")){
	    	mockturtle::aig_network aig = store<mockturtle::aig_network>().current();
		    mockturtle::write_bench(aig, filename);
		}
		else if(is_set("bench")){
			mockturtle::klut_network bench = store<mockturtle::klut_network>().current();
			mockturtle::write_bench(bench, filename);
		}
		else{
			std::cout << "Store to write out wasn't specified\n";
		}
	  }

	private:
	  std::string filename{};
	};

	ALICE_ADD_COMMAND(write_to_bench, "Output");


	/*Converts an AIG network to a k-LUT network and prints it out*/
	ALICE_COMMAND( lutMap, "Optimization", "Performs LUT mapping on an AIG network and prints out the bench file representation"){

		mockturtle::aig_network aig = store<mockturtle::aig_network>().current();
		mockturtle::mapping_view<mockturtle::aig_network, true> mapped_aig{aig};
		mockturtle::lut_mapping<mockturtle::mapping_view<mockturtle::aig_network, true>, true>(mapped_aig);
		const auto klut = mockturtle::collapse_mapped_network<mockturtle::klut_network>( mapped_aig );
		mockturtle::write_bench( aig, "lutMapActual.bench" );
	}

	ALICE_COMMAND( cut_e, "Optimization", "Performs Cut Enumeration on a stored AIG network"){
		auto aig = store<mockturtle::aig_network>().current();
		const mockturtle::network_cuts cuts = cut_enumeration( aig );
		//store<mockturtle::network_cuts>().extend() = cuts;
		aig.foreach_node( [&]( auto node ) {
  			std::cout << cuts.cuts( aig.node_to_index( node ) ) << "\n";
		} );
	}

	ALICE_COMMAND(hyperG, "Partitioning", "Output current stored AIG network in a hypergraph representation to be used by hMetis"){
				
		//Check to make sure that the user stores an AIG in the store before running this command
		if(!store<mockturtle::aig_network>().empty()){

			std::string filename = "";
			std::cout << "Enter .hpg filename to write to: ";
			std::cin >> filename;
			if(checkExt(filename, "hpg")){
				std::cout << "Writing to " << filename << std::endl;
			}
			else{
				std::cout << filename << " is not a valid hpg file\n";
			}

			std::ofstream output;
			output.open(filename);

			auto aig = store<mockturtle::aig_network>().current();		

			//Remove all children indeces from nodes so that the only connections remaining are outputs
			std::vector<std::vector<int>> hyperEdges;
			aig.foreach_node( [&]( auto node ) {
				int nodeNdx = aig.node_to_index(node);				

			  	//If a node has more than one output, add that net to the list of hyperedges
			  	if(aig._storage->connections[nodeNdx].size() > 0){

			  		std::vector<int> connection_to_add = aig._storage->connections[nodeNdx];
			  		//Add root node to the hyper edge
					connection_to_add .insert(aig._storage->connections[nodeNdx].begin(), nodeNdx);
					hyperEdges.push_back(connection_to_add);
				}

			} );
	  		
			//Write to the hypergraph file
	  		std::cout << "hyperEdges = \n";
	  		//Subtract the size by one because the output node is duplicated with one of the gate nodes
	  		output << hyperEdges.size() << " " << aig.size() - 1 << "\n";
	  		for(int i = 0; i < hyperEdges.size(); i++){
				for(int j = 0; j < hyperEdges.at(i).size(); j++){
					//Add 1 to the indeces because hMetis does not recognize hyperedges containing vertex 0
					std::cout << hyperEdges.at(i).at(j) + 1 << " ";
					output << hyperEdges.at(i).at(j) + 1 << " ";
				}
				std::cout << "\n";
				output << "\n";
			}

			output.close();
		}
		else{
			std::cout << "There is no AIG network stored\n";
		}
  		
	}

	ALICE_COMMAND(map_part, "Partitioning", "Map the AIG node connections to their respective partitions from hMetis"){

		std::string filename = "";
		std::cout << "Enter in partitioning filename: ";
		std::cin >> filename;
		std::cout << "\n";

		std::string line;
		std::ifstream input (filename);

		if(!store<mockturtle::aig_network>().empty()){
			auto aig = store<mockturtle::aig_network>().current();

			std::cout << "Example truth table\n";
			aig.foreach_node( [&]( auto node ) {
				const auto cubes = kitty::isop( aig.node_function(node));

				  /* print ISOP */
				  for ( auto cube : cubes )
				  {
				    cube.print( aig.node_function(node).num_vars() );
				    std::cout << "\n\n";
				  }
			});
			if(input.is_open()){
				int nodeNdx = 0;
				while ( getline (input,line) ){

					int partition = atoi(line.c_str());
			     	aig.add_to_partition(nodeNdx, partition);
			     	nodeNdx++;
			    }
			    input.close();

			    aig.map_partition_conn();
			}
			else{
				std::cout << "Unable to open file\n";
			}
			
		}
		else{
			std::cout << "There is no stored AIG network\n";
		}



	}

} // namespace alice

/* Main method for the Alice shell (with prefix) */
ALICE_MAIN( lstools )