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
	  				/*if(aig.num_pos() >= (aig.size() - nodeIdx)){
	  					int outputOffset = aig.size() - nodeIdx;
	  					nodeIdx = (aig.num_pis() + aig.num_pos() - outputOffset);
	  				}*/
	  				
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
			});

			aig.add_connections_network(connections);
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
				std::cout << "connections found for " << nodeNdx << "\n";
				std::cout << "Connections = ";

				for(int i = 0; i < aig._storage->connections[nodeNdx].size(); i++){
					 std::cout << aig._storage->connections[nodeNdx].at(i) << " ";
				}

				std::cout << "\n";
							
				std::cout << "Node = " << nodeNdx << "\n";
			  	//If a node has more than one output, add that net to the list of hyperedges
			  	if(aig._storage->connections[nodeNdx].size() > 0){
			  		std::cout << "HERE1\n";
			  		std::vector<int> connection_to_add = aig._storage->connections[nodeNdx];
			  		std::cout << "HERE2\n";
			  		//Add root node to the hyper edge
					connection_to_add.insert(connection_to_add.begin(), nodeNdx);
					std::cout << "HERE3\n";
					hyperEdges.push_back(connection_to_add);
					std::cout << "HERE4\n";
				}

			} );
	  		
	  		std::cout << "Got out\n";
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
				std::cout << "node " << aig.node_to_index(node) << " gate logic = 0x";
				auto func = aig.node_function(node);
				aig.foreach_fanin( node, [&]( auto const& conn, auto i ) {
					if ( aig.is_complemented( conn ) ) {
				        kitty::flip_inplace( func, i );
				    }
				});
				std::cout << kitty::to_hex(func) << "\n";
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

	bool is_in_vector(std::vector<int> vec, int nodeIdx){

		if(find(vec.begin(),vec.end(),nodeIdx) != vec.end())
			return true;
		else
			return false;
	}

	bool is_in_map(std::map<int, std::vector<int>> _map, int nodeIdx){

		if(_map.find(nodeIdx) != _map.end())
			return true;
		else 
			return false;
	}

	std::vector<int> get_logic_from_hex(std::string hex){

		std::cout << "Logic Hex VAlue = " << hex << "\n";
		int hexValue = atoi(hex.c_str());
		switch(hexValue){
			case 8:
				return {1, 1};
				break;
			
			case 1:
				return {0, 0};
				break;
			
			case 4:
				return {0, 1};
				break;
			
			case 2:
				return {1, 0};
				break; 
			
			default:
				std::cout << "Error in gate logic\n";
			
		}
	}

	std::vector<std::vector<int>> other_possible_outputs(std::vector<int> out){
		std::vector<std::vector<int>> possibles = {{0,0},{0,1},{1,0},{1,1}};

		for(int i = 0; i < possibles.size(); i++){

			if(possibles.at(i) == out){
				possibles.erase(possibles.begin() + i);
			}
		}

		return possibles;

	}

	void genTruth(mockturtle::aig_network aig, int nodeIdx, int wantedOut, int partition){

		auto node = aig.index_to_node(nodeIdx);

		std::cout << "\noutput node = " << nodeIdx << "\n\n";
		std::cout << "wantedOut = " << wantedOut << "\n";		

		if(is_in_vector(aig._storage->partitionInputs[partition], nodeIdx)){

			std::cout << "INPUT FOUND\n";

			if(wantedOut == 0){
				aig._storage->truth[partition][nodeIdx].push_back(0);
			}
			else{
				aig._storage->truth[partition][nodeIdx].push_back(1);
			}
		}
		else{

			auto func = aig.node_function(node);
			aig.foreach_fanin( node, [&]( auto const& conn, auto i ) {
				if ( aig.is_complemented( conn ) ) {
			        kitty::flip_inplace( func, i );
			    }
			});
			std::vector<int> out = get_logic_from_hex(kitty::to_hex(func));

			if(wantedOut == 0){
				std::vector<std::vector<int>> wantedOutputs = other_possible_outputs(out);

				if((aig._storage->truth[partition][nodeIdx].size() < wantedOutputs.size()) && aig._storage->truth[partition][nodeIdx].size() > 0){
					int sizeDiff = wantedOutputs.size() - aig._storage->truth[partition].size();
					for(int i = 0; i < sizeDiff; i++){
						aig._storage->truth[partition][nodeIdx].push_back(aig._storage->
							truth[partition][nodeIdx].at(aig._storage->truth[partition][nodeIdx].size() - 1));
					}
				}

				for(int i = 0; i < wantedOutputs.size(); i++){
					std::cout << "Wanted Output " << i << " = ";
					for(int j = 0; j < wantedOutputs.at(i).size(); j++){
						std::cout << wantedOutputs.at(i).at(j) << " ";
					}
					std::cout << "\n";
				}

				for(int i = 0; i < wantedOutputs.size(); i++){
					
					int child1Idx = aig._storage->nodes[node].children[0].index;

					if(child1Idx < (aig.num_pis() + aig.num_pos()))
	  					child1Idx--;

					int child2Idx = aig._storage->nodes[node].children[1].index ;

					if(child2Idx < (aig.num_pis() + aig.num_pos()))
	  					child2Idx--;

					std::cout << "\nNew nodeIdx child1 = " << child1Idx << "\n";
					std::cout << "New wanted out child1 = " << wantedOutputs.at(i).at(0) << "\n";
					std::cout << "New partition = " << partition << "\n\n";
					genTruth(aig, child1Idx,wantedOutputs.at(i).at(0), partition);
					std::cout << "\nNew nodeIdx child2 = " << child2Idx << "\n";
					std::cout << "New wanted out child2 = " << wantedOutputs.at(i).at(1) << "\n";
					std::cout << "New partition = " << partition << "\n\n";
					genTruth(aig, child2Idx,wantedOutputs.at(i).at(1), partition);
				}
			}
			else{
				int child1Idx = aig._storage->nodes[node].children[0].index;

				if(child1Idx < (aig.num_pis() + aig.num_pos()))
	  					child1Idx--;

				int child2Idx = aig._storage->nodes[node].children[1].index ;

				if(child2Idx < (aig.num_pis() + aig.num_pos()))
	  					child2Idx--;

				std::cout << "\nNew nodeIdx child1 = " << child1Idx << "\n";
				std::cout << "New wanted out child1 = " << out.at(0) << "\n";
				std::cout << "New nodeIdx child1 = " << partition << "\n\n";
				genTruth(aig, child1Idx,out.at(0), partition);
				std::cout << "\nNew nodeIdx child2 = " << child2Idx << "\n";
				std::cout << "New wanted out child2 = " << out.at(1) << "\n";
				std::cout << "New nodeIdx child2 = " << partition << "\n\n";
				genTruth(aig, child2Idx,out.at(1), partition);
			}
		}
	}


	ALICE_COMMAND(test_truth, "test", "Output the truth table using cut enumeration"){

		if(!store<mockturtle::aig_network>().empty()){
			auto aig = store<mockturtle::aig_network>().current();

			auto cuts = mockturtle::cut_enumeration<mockturtle::aig_network, true>( aig ); /* true enables truth table computation */
			aig.foreach_node( [&]( auto n ) {


			  const auto index = aig.node_to_index( n );
			 
			  for ( auto const& set : cuts.cuts( index ) )
			  {

			  	std::cout << "Cut " << *set
			                << " with truth table " << kitty::to_hex( cuts.truth_table( *set ) )
			                << "\n";
			  	/*
			    for ( auto const& cut : set )
			    {
			      std::cout << "Cut " << *cut
			                << " with truth table " << kitty::to_hex( cuts.truth_table( *cut ) )
			                << "\n";
			    }
			    */
			  }
			} );

		}
		else{
			std::cout << "No AIG network stored\n";
		}
	}

	ALICE_COMMAND(output_truth_table, "Output", "Output the truth table for each partition in blif format"){

		if(!store<mockturtle::aig_network>().empty()){
			auto aig = store<mockturtle::aig_network>().current();
			if(aig._storage->partitionMap.size() != 0){

				for(int i = 0; i < aig._storage->num_partitions; i++){

					aig.foreach_node( [&]( auto node ) {

						int nodeIdx = aig.node_to_index(node);
						if(aig._storage->partitionMap[nodeIdx] == i){

							std::map<int,std::vector<int>>::iterator it;
							for(int j = 0; j < aig._storage->partitionConn[i][nodeIdx].size(); j++){
																
								if((aig.is_po(node) || (!is_in_map(aig._storage->partitionConn[i], aig._storage->partitionConn[i][nodeIdx].at(j))))
								 && !is_in_vector(aig._storage->partitionOutputs[i],nodeIdx)){
									aig._storage->partitionOutputs[i].push_back(nodeIdx);
								}
								int child1Idx = aig._storage->nodes[node].children[0].index;
								int child2Idx = aig._storage->nodes[node].children[1].index;
								if(child1Idx < (aig.num_pis() + aig.num_pos()))
	  								child1Idx--;
	  							if(child2Idx < (aig.num_pis() + aig.num_pos()))
	  								child2Idx--;
								bool child1 = is_in_map(aig._storage->partitionConn[i], child1Idx);
								bool child2 = is_in_map(aig._storage->partitionConn[i], child2Idx);

								if((aig._storage->nodes[node].children.size() != 0) && ((child1 && !child2) || (!child1 && child2))){
									if(!child1 && !is_in_vector(aig._storage->partitionInputs[i],child1Idx))
										aig._storage->partitionInputs[i].push_back(child1Idx);
									else if(!child2 && !is_in_vector(aig._storage->partitionInputs[i],child2Idx))
										aig._storage->partitionInputs[i].push_back(child2Idx);
								}

								if( ( (!child1 && !child2) || aig.is_pi(node) ) && !is_in_vector(aig._storage->partitionInputs[i],nodeIdx)){

									aig._storage->partitionInputs[i].push_back(nodeIdx);
								}
							}
						}
					});
				}

				for(int i = 0; i < aig._storage->num_partitions; i++){

					std::cout << "Partition " << i << " Inputs: ";
					for(int j = 0; j < aig._storage->partitionInputs[i].size(); j++){
						std::cout << aig._storage->partitionInputs[i].at(j) << " ";
					}
					std::cout << "\n";
					std::cout << "Partition " << i << " Outputs: ";
					for(int j = 0; j < aig._storage->partitionOutputs[i].size(); j++){
						std::cout << aig._storage->partitionOutputs[i].at(j) << " ";
					}
					std::cout << "\n";
				}

				for(int i = 0; i < aig._storage->num_partitions; i++){

					for(int j = 0; j < aig._storage->partitionOutputs[i].size(); j++){

						aig._storage->truth.clear();
						genTruth(aig, aig._storage->partitionOutputs[0].at(j), 1, i);

					}
				}

				for(int i = 0; i < aig._storage->num_partitions; i++){

					std::cout << "Partition " << 0 << " truth table logic\n";
					for(int j = 0; j < aig._storage->partitionOutputs[0].size(); j++){
						for(int k = 0; k < aig._storage->truth[0][j].size(); k++){
							std::cout << aig._storage->truth[0][j].at(k);
						}
						std::cout << " 1\n";
					}
				}

			}
			else{
				std::cout << "The partitions have not been mapped to the stored aig network\n";
			}
		}
		else{
			std::cout << "There is no stored AIG network\n";
		}
	}
	

} // namespace alice

/* Main method for the Alice shell (with prefix) */
ALICE_MAIN( lstools )