#include <lorina/lorina.hpp>
#include <alice/alice.hpp>
#include <kitty/kitty.hpp>
#include <mockturtle/mockturtle.hpp>

#include <mockturtle/views/mffc_view.hpp>
#include <iostream>
#include <string>
#include <algorithm>
#include <queue>
#include <unordered_map>
#include <vector>
#include <time.h>
#include <fmt/format.h>
#include <stdio.h>
#include <stdlib.h>
#include <aig/gia/gia.h>
#include <aig/gia/giaAig.h>
#include <base/wlc/wlc.h>

namespace alice{

	/* checkExt
	 * 
	 * @param filename: The name of the file to check the extension of
	 * @param ext: The extension that you are checking that filename has
	 *
	 * Checks to see if a file has a specified extension
	 */
	bool checkExt(std::string filename, std::string ext){

		if(filename.substr(filename.find_last_of(".") + 1) == ext) {
    		return true;
  		} else {
    		return false;
  		}
	}//checkExt()

	bool check_for_duplicates(std::vector<int> vec, int nodeIdx){

		std::vector<int>::iterator first_occurance = std::find(vec.begin(),vec.end(),nodeIdx);
		if(first_occurance != vec.end()){
			if(std::find(first_occurance,vec.end(),nodeIdx) != vec.end())
				return true;
			else
				return false;
		}
		else
			return false;
	}

	bool is_in_vector(std::vector<int> vec, int nodeIdx){

		if(std::find(vec.begin(),vec.end(),nodeIdx) != vec.end())
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

	int get_index(std::vector<int> index, int nodeIdx){

		std::vector<int>::iterator it = find(index.begin(), index.end(), nodeIdx);
		return std::distance(index.begin(), it);
	}

	/* add_to_truth
	 *
	 * @param aig: AIG network that the truth table is being made for
	 * @param nodeIdx: The index of the gate that you are adding the truth table of
	 * @param wantedOut: The output that you want the gate at nodeIdx to be
	 * @param output: The node that the truth table is being made for
	 * @param fork_flag: Can be one of four (4) values that are used to determine if a connection
	 *					 is forked and logic being added to the truth table needs to be restricted.
	 *					 0: No connection is forked
	 *					 1: The left child is forked
	 *					 2: The right child is forked
	 *					 3: Both children are forked
	 *
	 * Adds to a truth table for the output by adding the truth table needed for the logic output
	 * wanted for a given node(nodeIdx)
	 */
	void add_to_truth(mockturtle::aig_network aig, int nodeIdx, int wantedOut, int output, int fork_flag){

		auto node = aig.index_to_node(nodeIdx);

		std::cout << "\noutput node = " << nodeIdx << "\n\n";
		std::cout << "wantedOut = " << wantedOut << "\n";
		std::cout << "\n";

		int child1Idx = aig._storage->nodes[node].children[0].index;

		//Adjust children indeces if they are primary inputs
		if(child1Idx < (aig.num_pis() + aig.num_pos()))
	  		child1Idx--;

		int child2Idx = aig._storage->nodes[node].children[1].index ;

		if(child2Idx < (aig.num_pis() + aig.num_pos()))
	  		child2Idx--;
  		

		std::cout << "index = {";
		for(int i = 0; i < aig._storage->index[output].size(); i++){
			std::cout << aig._storage->index[output].at(i) << " ";
		}
		std::cout << "}\n";
		
		//Values that a forked connection MUST be for the truth table to be valid i.e. you can't have 2 different
		//values for the same wire at the same time
		int critical_val1;
		int critical_val2;

		if(wantedOut == 0){
			
			std::vector<std::vector<int>> offset_cpy;;
			int i = 0;

			while(i < aig._storage->tt[output].size()){

				//Copy the offset list so the original is not changed
				offset_cpy = aig._storage->offset[nodeIdx];
				//Check to make sure that you only affect rows of the truth table where the node's wanted output
				//matches in order to merge its onset or offset truth table to the respevtive output
				if(aig._storage->tt[output].at(i).at(get_index(aig._storage->index[output], nodeIdx)) == wantedOut){

					if(fork_flag == 1){	
						//Checks the value already stored for the child index
						//Since these values are added during a BFS, the value already there is more critical
						//to the output and therefore needs to be matched			
						critical_val1 = aig._storage->tt[output].at(i).at( get_index(aig._storage->index[output], child1Idx));
						
						//Erase from the offset truth table, logic combinations that don't match the critical value
						for(int j = 0; j < offset_cpy.size(); j++){
							if(offset_cpy.at(j).at(0) != critical_val1){
								offset_cpy.erase(offset_cpy.begin() + j);
								j--;
							}
						}
						//Only add values for the right child since the left child is already taken care of
						aig._storage->tt[output].at(i).at( get_index(aig._storage->index[output], child2Idx)) = offset_cpy.at(0).at(1);

						int idx = i + 1;
						//Copy the truth table down to add more combinations for the wanted output if needed
						for(int j = 1; j < offset_cpy.size(); j++){
							
							aig._storage->tt[output].insert(aig._storage->tt[output].begin() + idx, aig._storage->tt[output].at(i));

							aig._storage->tt[output].at(idx).at(get_index(aig._storage->index[output], child2Idx)) = offset_cpy.at(j).at(1);

							idx++;
						}
						i += offset_cpy.size();
					}
					else if(fork_flag == 2){
						//Checks the value already stored for the child index
						//Since these values are added during a BFS, the value already there is more critical
						//to the output and therefore needs to be matched	
						critical_val2 = aig._storage->tt[output].at(i).at( get_index(aig._storage->index[output], child2Idx));

						//Erase from the offset truth table, logic combinations that don't match the critical value
						for(int j = 0; j < offset_cpy.size(); j++){
							if(offset_cpy.at(j).at(1) != critical_val2){
								offset_cpy.erase(offset_cpy.begin() + j);
								j--;
							}
						}
						//Only add values for the left child since the right child is already taken care of
						aig._storage->tt[output].at(i).at( get_index(aig._storage->index[output], child1Idx)) = offset_cpy.at(0).at(0);

						int idx = i + 1;
						//Copy the truth table down to add more combinations for the wanted output if needed
						for(int j = 1; j < offset_cpy.size(); j++){
							
							aig._storage->tt[output].insert(aig._storage->tt[output].begin() + idx, aig._storage->tt[output].at(i));

							aig._storage->tt[output].at(idx).at(get_index(aig._storage->index[output], child1Idx)) = offset_cpy.at(j).at(0);

							idx++;
						}
						i += offset_cpy.size();
					}
					else if(fork_flag == 3){
						//Checks the value already stored for both of the children
						//Since these values are added during a BFS, the value already there is more critical
						//to the output and therefore needs to be matched	
						critical_val1 = aig._storage->tt[output].at(i).at( get_index(aig._storage->index[output], child1Idx));
						critical_val2 = aig._storage->tt[output].at(i).at( get_index(aig._storage->index[output], child2Idx));

						//Erase from the offset truth table, logic combinations that don't match both of the critical values
						for(int j = 0; j < offset_cpy.size(); j++){
							if(!((offset_cpy.at(j).at(0) == critical_val1) && (offset_cpy.at(j).at(1) == critical_val2))){
								offset_cpy.erase(offset_cpy.begin() + j);
								j--;
							}
						}

						//If the new offset truth table is empty now, the current node's is not valid and should 
						//be erased from the truth table
						if(offset_cpy.size() == 0){
							aig._storage->tt[output].erase(aig._storage->tt[output].begin() + i);
							i--;
						}
						
						i++;
					}
				
					else{

						aig._storage->tt[output].at(i).at( get_index(aig._storage->index[output], child1Idx)) = offset_cpy.at(0).at(0);
						aig._storage->tt[output].at(i).at( get_index(aig._storage->index[output], child2Idx)) = offset_cpy.at(0).at(1);
						int idx = i + 1;
						//Copy the truth table down to add more combinations for the wanted output if needed
						for(int j = 1; j < offset_cpy.size(); j++){
								
							aig._storage->tt[output].insert(aig._storage->tt[output].begin() + idx, aig._storage->tt[output].at(i));

							aig._storage->tt[output].at(idx).at(get_index(aig._storage->index[output], child1Idx)) = offset_cpy.at(j).at(0);
							aig._storage->tt[output].at(idx).at(get_index(aig._storage->index[output], child2Idx)) = offset_cpy.at(j).at(1);

							idx++;
						}
						i += offset_cpy.size();
					}
					
				}
				else{
					i++;
				}
			}//while

			//Update the wanted outputs for the children of the node
			for(int i = 0; i < offset_cpy.size(); i++){
				
				if(fork_flag == 1){
					if(!is_in_vector(aig._storage->wantedOut[child2Idx], offset_cpy.at(i).at(1)))
						aig._storage->wantedOut[child2Idx].push_back(offset_cpy.at(i).at(1));
				}
				else if(fork_flag == 2){
					if(!is_in_vector(aig._storage->wantedOut[child1Idx], offset_cpy.at(i).at(0)))
						aig._storage->wantedOut[child1Idx].push_back(offset_cpy.at(i).at(0));
				}
				else{
					if(!is_in_vector(aig._storage->wantedOut[child1Idx], offset_cpy.at(i).at(0)))
						aig._storage->wantedOut[child1Idx].push_back(offset_cpy.at(i).at(0));
					if(!is_in_vector(aig._storage->wantedOut[child2Idx], offset_cpy.at(i).at(1)))
						aig._storage->wantedOut[child2Idx].push_back(offset_cpy.at(i).at(1));	
				}			
			}
		}
		else{

			//Copy the onset list so the original is not changed
			std::vector<int> onset_cpy;
			for(int i = 0; i < aig._storage->tt[output].size(); i++){
				onset_cpy = aig._storage->onset[nodeIdx];

				//Check to make sure that you only affect rows of the truth table where the node's wanted output
				//matches in order to merge its onset or offset truth table to the respevtive output
				if(aig._storage->tt[output].at(i).at(get_index(aig._storage->index[output], nodeIdx)) == wantedOut){

					if(fork_flag == 1){
						critical_val1 = aig._storage->tt[output].at(i).at(get_index(aig._storage->index[output], child1Idx));
						//If the value for the left child in the onset doesn't match the critical value, erase that
						//row because the value for the node is not possible
						if(onset_cpy.at(0) != critical_val1){
							onset_cpy.erase(onset_cpy.begin());
							aig._storage->tt[output].erase(aig._storage->tt[output].begin() + i);
							i--;
						}
						else{
							aig._storage->tt[output].at(i).at(get_index(aig._storage->index[output], child1Idx)) = aig._storage->onset[nodeIdx].at(0);
							aig._storage->tt[output].at(i).at(get_index(aig._storage->index[output], child2Idx)) = aig._storage->onset[nodeIdx].at(1);
						}

					} 
					else if(fork_flag == 2){
						critical_val2= aig._storage->tt[output].at(i).at(get_index(aig._storage->index[output], child2Idx));
						//If the value for the right child in the onset doesn't match the critical value, erase that
						//row because the value for the node is not possible
						if(onset_cpy.at(1) != critical_val1){
							onset_cpy.erase(onset_cpy.begin());
							aig._storage->tt[output].erase(aig._storage->tt[output].begin() + i);
							i--;
						}
						else{						
							aig._storage->tt[output].at(i).at(get_index(aig._storage->index[output], child1Idx)) = aig._storage->onset[nodeIdx].at(0);
							aig._storage->tt[output].at(i).at(get_index(aig._storage->index[output], child2Idx)) = aig._storage->onset[nodeIdx].at(1);
						}

					}
					else if(fork_flag == 3){
						critical_val1 = aig._storage->tt[output].at(i).at(get_index(aig._storage->index[output], child1Idx));
						critical_val2 = aig._storage->tt[output].at(i).at(get_index(aig._storage->index[output], child2Idx));

						//If the value for the both children in the onset don't match the critical values, erase that
						//row because the value for the node is not possible
						if((onset_cpy.at(0) != critical_val1) || (onset_cpy.at(1) != critical_val2)){
							onset_cpy.erase(onset_cpy.begin());
							aig._storage->tt[output].erase(aig._storage->tt[output].begin() + i);
							i--;
						}
					}
					else{

						aig._storage->tt[output].at(i).at(get_index(aig._storage->index[output], child1Idx)) = aig._storage->onset[nodeIdx].at(0);
						aig._storage->tt[output].at(i).at(get_index(aig._storage->index[output], child2Idx)) = aig._storage->onset[nodeIdx].at(1);
					}
				}

			}

			//Update the wanted outputs for the children
			if(fork_flag == 0){
				if(!is_in_vector(aig._storage->wantedOut[child1Idx], onset_cpy.at(0)))
					aig._storage->wantedOut[child1Idx].push_back(onset_cpy.at(0));
				if(!is_in_vector(aig._storage->wantedOut[child2Idx], onset_cpy.at(1)))
					aig._storage->wantedOut[child2Idx].push_back(onset_cpy.at(1));
			}
			
		}
		std::cout << "index = {";
		for(int i = 0; i < aig._storage->index[output].size(); i++){
			std::cout << aig._storage->index[output].at(i) << " ";
		}
		std::cout << "}\n";

		for(int i = 0; i < aig._storage->tt[output].size(); i++){
			for(int j = 0; j < aig._storage->tt[output].at(i).size(); j++){
				std::cout << aig._storage->tt[output].at(i).at(j) << " ";
			}
			std::cout << "\n";
		}
	}//add_to_truth()


	/* genTruthBFS
	 *
	 * @param aig: AIG network that is being traversed
	 * @param output: Root node to begin traversal at (output that the truth table is being created for)
	 * @param wantedOut: The outputs wanted for a node to make the output high or low
	 * @param partition: The partition that the root node belongs to
	 *
	 * Performs a breadth first search of the network in order to keep nodes visited in topological order and merges
	 * each node's truth table to create a truth table for the specified output.
	 */
	void genTruthBFS(mockturtle::aig_network aig, int output, std::vector<int> wantedOut, int partition){

		std::queue<int> net_queue;
		std::map<int, bool> visited;
		//Set all nodes to be unvisited
		aig.foreach_node( [&]( auto node ) {
			visited[aig.node_to_index(node)] = false;
		});

		net_queue.push(output);
		visited[output] = true;
		
		while(!net_queue.empty()){

			int curr_node = net_queue.front();
			wantedOut = aig._storage->wantedOut[curr_node];
			net_queue.pop();
			auto node = aig.index_to_node(curr_node);

			//Flag to make sure that children are not added to the truth table more than once
			bool add_to_truth_flag = true;

			//Make sure that the BFS traversal does not go past the inputs of the partition
			if(!is_in_vector(aig._storage->partitionInputs[partition], curr_node)){

				for(int i = 0; i < aig._storage->nodes[node].children.size(); i++){

					int childIdx = aig._storage->nodes[node].children[i].index;
					bool is_valid = true;

					//Adjust the child index if it is a primary input
					if(childIdx < (aig.num_pis() + aig.num_pos())){
		  				childIdx--;
					}

					//Make sure a valid child index is found
					if(childIdx < 0){
						is_valid = false;
					}

					if(!visited[childIdx]){	

						if(is_valid){
									
							net_queue.push(childIdx);
							visited[childIdx] = true;

							if(add_to_truth_flag && !is_in_vector(aig._storage->partitionInputs[partition], curr_node)){

								//Add both children of a node to the truth table at once
								int child1Idx = aig._storage->nodes[node].children[0].index;
								int child2Idx = aig._storage->nodes[node].children[1].index;

								//Adjust children indeces if they are primary inputs
								if(child1Idx < (aig.num_pis() + aig.num_pos())){
					  				child1Idx--;
								}
								if(child2Idx < (aig.num_pis() + aig.num_pos())){
					  				child2Idx--;
								}

								int fork_flag;
								
								//The first child has not been visited but the second one has
								//The RIGHT child has a forked connection
								if(visited[child2Idx]){
									aig._storage->index[output].push_back(child1Idx);
									fork_flag = 2;
								}
								else{//Neither of the children have a forked connection
									aig._storage->index[output].push_back(child1Idx);
									aig._storage->index[output].push_back(child2Idx);
									fork_flag = 0;
								}							

								for(int j = 0; j < wantedOut.size(); j++){
					
									add_to_truth(aig, curr_node, wantedOut.at(j), output, fork_flag);
									//Make sure that no node and its children are added to the truth table more than
									//once								
									add_to_truth_flag = false;

								}
							}
						}

					}
					else{
						
						if(add_to_truth_flag && !is_in_vector(aig._storage->partitionInputs[partition], curr_node)){

							int child1Idx = aig._storage->nodes[node].children[0].index;
							int child2Idx = aig._storage->nodes[node].children[1].index;
							//Adjust child indeces if they are primary inputs
							if(child1Idx < (aig.num_pis() + aig.num_pos())){
				  				child1Idx--;
							}
							if(child2Idx < (aig.num_pis() + aig.num_pos())){
				  				child2Idx--;
							}

							int fork_flag;
							
							//If both children have been visited before
							//BOTH children have forked connections
							if(visited[child2Idx]){
								fork_flag = 3;
							}
							else{//Only the first (LEFT) child has a forked connection
								aig._storage->index[output].push_back(child2Idx);
								fork_flag = 1;
							}

							for(int j = 0; j < wantedOut.size(); j++){

								add_to_truth(aig, curr_node, wantedOut.at(j), output, fork_flag);					
								//Make sure that a node and its children are not added to the truth table more than once
								add_to_truth_flag = false;

							}
						}						
					}
				}
			}
		}
	}//genTruthBFS()

	void genTruth(mockturtle::aig_network aig, int nodeIdx, int wantedOut, int partition, int output){

		auto node = aig.index_to_node(nodeIdx);

		std::cout << "\noutput node = " << nodeIdx << "\n\n";
		std::cout << "wantedOut = " << wantedOut << "\n";		

		if(is_in_vector(aig._storage->partitionInputs[partition], nodeIdx)){

			std::cout << "INPUT FOUND\n";

		}
		else{

			int child1Idx = aig._storage->nodes[node].children[0].index;

			if(child1Idx < (aig.num_pis() + aig.num_pos()))
	  			child1Idx--;

			int child2Idx = aig._storage->nodes[node].children[1].index ;

			if(child2Idx < (aig.num_pis() + aig.num_pos()))
	  			child2Idx--;

	  		//std::replace(aig._storage->index[output].begin(), aig._storage->index[output].end(),nodeIdx, child1Idx);
	  		aig._storage->index[output].push_back(child1Idx);
			aig._storage->index[output].push_back(child2Idx);

			std::cout << "index = {";
			for(int i = 0; i < aig._storage->index[output].size(); i++){
				std::cout << aig._storage->index[output].at(i) << " ";
			}
			std::cout << "}\n";
			std::cout << "Child 1 = " << child1Idx << "\n";
			std::cout << "Child 2 = " << child2Idx << "\n";
			

			// auto func = aig.node_function(node);
			// aig.foreach_fanin( node, [&]( auto const& conn, auto i ) {
			// 	if ( aig.is_complemented( conn ) ) {
			//         kitty::flip_inplace( func, i );
			//     }
			// });
			// std::vector<int> out_int = get_logic_from_hex(kitty::to_hex(func));

			// std::vector<bool> out;
			// for(int i = 0; i < out_int.size(); i++){
			// 	out.push_back((bool)out_int.at(i));
			// }

			if(wantedOut == 0){

				//std::vector<std::vector<bool>> wantedOutputs = other_possible_outputs(out);

				int size = aig._storage->tt[output].size() * 3;
				for(int i = 0; i < size; i += 3){

					std::cout << "Set index: " << get_index(aig._storage->index[output], child1Idx) << " to " << aig._storage->offset[nodeIdx].at(0).at(0)/*wantedOutputs.at(0).at(0)*/ << "\n";
					std::cout << "Set index: " << get_index(aig._storage->index[output], child2Idx) << " to " << aig._storage->offset[nodeIdx].at(0).at(1)/*wantedOutputs.at(0).at(1)*/ << "\n";
					//Check for duplicates
					if(check_for_duplicates(aig._storage->index[output], nodeIdx)){
						std::cout << "Forked connection found\n";
					}
					else{
						std::cout << "No forked connection found\n";
					}
					aig._storage->tt[output].at(i).at( get_index(aig._storage->index[output], child1Idx)) = aig._storage->offset[nodeIdx].at(0).at(0);//wantedOutputs.at(0).at(0); 
					aig._storage->tt[output].at(i).at( get_index(aig._storage->index[output], child2Idx)) = aig._storage->offset[nodeIdx].at(0).at(1);//wantedOutputs.at(0).at(1); 
					

					
					int idx = i + 1;
					for(int j = 1; j < aig._storage->offset[nodeIdx].size();/*wantedOutputs.size();*/ j++){

						aig._storage->tt[output].insert(aig._storage->tt[output].begin() + idx, aig._storage->tt[output].at(i));

						std::cout << "Set index: " << get_index(aig._storage->index[output], child1Idx) << " to " << aig._storage->offset[nodeIdx].at(j).at(0)/*wantedOutputs.at(j).at(0)*/ << "\n";
						std::cout << "Set index: " << get_index(aig._storage->index[output], child2Idx) << " to " << aig._storage->offset[nodeIdx].at(j).at(1)/*wantedOutputs.at(j).at(1)*/ << "\n";

						aig._storage->tt[output].at(idx).at(get_index(aig._storage->index[output], child1Idx)) = aig._storage->offset[nodeIdx].at(j).at(0);/*wantedOutputs.at(j).at(0);*/
						//genTruth(aig, child1Idx, aig._storage->offset[nodeIdx].at(j).at(0)/*wantedOutputs.at(j).at(0)*/, partition, output); 
						aig._storage->tt[output].at(idx).at(get_index(aig._storage->index[output], child2Idx)) = aig._storage->offset[nodeIdx].at(j).at(1);/*wantedOutputs.at(0).at(0);*/
						//genTruth(aig, child2Idx, aig._storage->offset[nodeIdx].at(j).at(1)/*wantedOutputs.at(j).at(1)*/, partition, output); 

						idx++;
					}

				}
				for(int i = 1; i < aig._storage->offset[nodeIdx].size(); i++){
					std::cout << "3 child1Idx = " << child1Idx << "\n";
					genTruth(aig, child1Idx, aig._storage->offset[nodeIdx].at(i).at(0)/*wantedOutputs.at(j).at(0)*/, partition, output); 
					std::cout << "3 child2Idx = " << child2Idx << "\n";
					genTruth(aig, child2Idx, aig._storage->offset[nodeIdx].at(i).at(1)/*wantedOutputs.at(j).at(0)*/, partition, output); 
				}
				/*
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
				}*/
			}
			else{

				for(int i = 0; i < aig._storage->tt[output].size(); i ++){

					std::cout << "Set index: " << get_index(aig._storage->index[output], child1Idx) << " to " << aig._storage->onset[nodeIdx].at(0)/*wantedOutputs.at(j).at(0)*/ << "\n";
					std::cout << "Set index: " << get_index(aig._storage->index[output], child2Idx) << " to " << aig._storage->onset[nodeIdx].at(1)/*wantedOutputs.at(j).at(1)*/ << "\n";

					aig._storage->tt[output].at(i).at(get_index(aig._storage->index[output], child1Idx)) = aig._storage->onset[nodeIdx].at(0);/*wantedOutputs.at(j).at(0);*/
					//genTruth(aig, child1Idx, aig._storage->onset[nodeIdx].at(0)/*wantedOutputs.at(j).at(0)*/, partition, output); 
					aig._storage->tt[output].at(i).at(get_index(aig._storage->index[output], child2Idx)) = aig._storage->onset[nodeIdx].at(1);/*wantedOutputs.at(0).at(0);*/
					//genTruth(aig, child2Idx, aig._storage->onset[nodeIdx].at(1)/*wantedOutputs.at(j).at(1)*/, partition, output); 


				}

				std::cout << "1 child1Idx = " << child1Idx << "\n";
				genTruth(aig, child1Idx, aig._storage->onset[nodeIdx].at(0), partition, output); 
				std::cout << "1 child2Idx = " << child2Idx << "\n";
				genTruth(aig, child2Idx, aig._storage->onset[nodeIdx].at(1), partition, output); 
				

				/*
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
				genTruth(aig, child2Idx,out.at(1), partition);*/
			}
		}
	}

	/*
	 * Makes the stored truth table for a given output ready for writing to
	 * a blif file by deleting columns that don't correspond to inputs of 
	 * the given partition
	 */
	void finalize_truth(mockturtle::aig_network aig, int output, int partition){

		std::cout << "partition inputs = ";
		for(int i = 0; i < aig._storage->partitionInputs[partition].size(); i++){
			std::cout << aig._storage->partitionInputs[partition].at(i) << " ";
		}
		std::cout << "\n";

		std::cout << "tt size = " << aig._storage->tt[output].at(0).size() << "\n";
		std::cout << "index size = " << aig._storage->index[output].size() << "\n";
		if(aig._storage->tt[output].at(0).size() > aig._storage->index[output].size()){
			for(int i = 0; i < aig._storage->tt[output].size(); i++){
				for(int j = aig._storage->index[output].size(); j < aig._storage->tt[output].at(i).size(); j++){
					aig._storage->tt[output].at(i).erase(aig._storage->tt[output].at(i).begin() + j);
					j--;
					
				}
				
			}
		}

		for(int i = 0; i < aig._storage->index[output].size(); i++){

			if(!is_in_vector(aig._storage->partitionInputs[partition], aig._storage->index[output].at(i))){
				
				for(int j = 0; j < aig._storage->tt[output].size(); j++){
					aig._storage->tt[output].at(j).erase(aig._storage->tt[output].at(j).begin() + i);

				}
				aig._storage->index[output].erase(aig._storage->index[output].begin() + i);
				i--;
			}
		}

		for(int idx = 0; idx < aig._storage->tt[output].size(); idx++){
			std::cout << "tt.at(" << idx << ") ";
			for(int jdx = 0; jdx < aig._storage->tt[output].at(idx).size(); jdx++){
				 std::cout << aig._storage->tt[output].at(idx).at(jdx);
			}
			std::cout << "\n";
		}

		for(int i = 0; i < aig._storage->tt[output].size(); i++){
			for(int j = (i + 1); j < aig._storage->tt[output].size(); j++){
				if(aig._storage->tt[output].at(i) == aig._storage->tt[output].at(j)){
					aig._storage->tt[output].erase(aig._storage->tt[output].begin() + j);
					j--;
				}
			}
		}
		std::cout << "deleting duplicates\n";
		for(int idx = 0; idx < aig._storage->tt[output].size(); idx++){
			std::cout << "tt.at(" << idx << ") ";
			for(int jdx = 0; jdx < aig._storage->tt[output].at(idx).size(); jdx++){
				 std::cout << aig._storage->tt[output].at(idx).at(jdx);
			}
			std::cout << "\n";
		}
	}//finalize_truth()

	void blif_write(mockturtle::aig_network aig, int partition, int output_wanted, int output_num){

		time_t rawtime;
  		struct tm * timeinfo;

 		time (&rawtime);
  		timeinfo = localtime (&rawtime);

		std::string filename = aig._storage->net_name;
		filename.append("_part_" + std::to_string(partition) + ".blif");

		std::ofstream output;
  		output.open (filename);
  		output << "# Blif file written by lstools on " << asctime(timeinfo);
  		output << ".model " << aig._storage->net_name << "\n";
  		output << ".inputs ";

  		std::string inputs;
  		for(int i = 0; i < aig._storage->partitionInputs[partition].size(); i++){
  			int curr_input = aig._storage->partitionInputs[partition].at(i);
  			int corrected_input = curr_input;
  			if(aig.is_po(curr_input)){
		  		corrected_input = (aig.num_pis() + (aig.size() - curr_input));
		  		corrected_input--;
		  	}
  			output << corrected_input + 1 << " ";
  			inputs.append(std::to_string(corrected_input + 1) + " ");
  		}
  		output << "\n";
  		output << ".outputs ";

  		for(int i = 0; i < aig._storage->partitionOutputs[partition].size(); i++){

  			int curr_output = aig._storage->partitionOutputs[partition].at(i);

  			if(aig.is_po(curr_output)){
	  			std::string corrected_output = std::to_string((aig.num_pis() + (aig.size() - curr_output)));
	  			output << corrected_output << " ";
	  		}
	  		else{
	  			if(curr_output < (aig.num_pis() + aig.num_pos())){
	  				curr_output++;
	  			}
	  			output << curr_output << " ";
	  		}
  		}
  		output << "\n";


  		for(int i = 0; i < aig._storage->partitionOutputs[partition].size(); i++){
  			int curr_output = aig._storage->partitionOutputs[partition].at(i);

  			int corrected_output_num = curr_output;
	  		if(aig.is_po(curr_output)){
	  			corrected_output_num = (aig.num_pis() + (aig.size() - curr_output));
	  		}
	  		else if(is_in_vector(aig._storage->partitionInputs[partition], curr_output)){
	  			corrected_output_num++;
	  		}

  			output << ".names ";

  			for(int j = 0; j < aig._storage->partitionInputs[partition].size(); j++){
  				if(!is_in_vector(aig._storage->index[curr_output], aig._storage->partitionInputs[partition].at(j))){
  					aig._storage->index[curr_output].push_back(aig._storage->partitionInputs[partition].at(j));
  				}
  			}
  			for(int j = 0; j < aig._storage->index[curr_output].size(); j++){
  				int curr_index = aig._storage->index[curr_output].at(j); 
  				int corrected_index = curr_index;
  				if(aig.is_po(curr_index)){
		  			corrected_index = (aig.num_pis() + (aig.size() - curr_index));
		  			corrected_index--;
		  		}
  				output << corrected_index + 1 << " "; 
  			} 

  			output << corrected_output_num  << "\n";

  			for(int j = 0; j < aig._storage->tt[curr_output].size(); j++){

  				for(int k = 0; k < aig._storage->tt[curr_output].at(j).size(); k++){
  					output << aig._storage->tt[curr_output].at(j).at(k);
  				}

  				int remaining_inputs = aig._storage->index[curr_output].size() - aig._storage->tt[curr_output].at(j).size();
  				for(int k = 0; k < remaining_inputs; k++){
  					output << "-";
  				}

  				output << " " << output_wanted << "\n";

  			}
  		}
  		output << ".end\n";

 		output.close();

	}

	void blif_test(mockturtle::aig_network aig, int output_wanted){

		time_t rawtime;
  		struct tm * timeinfo;

 		time (&rawtime);
  		timeinfo = localtime (&rawtime);

		std::string filename = aig._storage->net_name;
		filename.append("_test.blif");

		std::ofstream output;
  		output.open (filename);
  		output << "# Blif file written by lstools on " << asctime(timeinfo);
  		output << ".model " << aig._storage->net_name << "\n";

  		output << ".inputs ";
  		for(int i = 1; i <= aig.num_pis(); i++){
  			output << i << " ";
  		}
  		output << "\n";

  		output << ".outputs ";
  		for(int i = 1; i <= aig.num_pos(); i++){
  			output << aig.num_pis() + i << " ";
  		}
  		output << "\n";

  		for(int idx = 0; idx < aig._storage->num_partitions; idx++){
  			int partition = idx;

	  		for(int i = 0; i < aig._storage->partitionOutputs[partition].size(); i++){
	  			int curr_output = aig._storage->partitionOutputs[partition].at(i);

	  			int corrected_output_num = curr_output;
		  		if(aig.is_po(curr_output)){
		  			corrected_output_num = (aig.num_pis() + (aig.size() - curr_output));
		  		}
		  		else if(is_in_vector(aig._storage->partitionInputs[partition], curr_output)){
		  			corrected_output_num++;
		  		}

	  			output << ".names ";

	  			for(int j = 0; j < aig._storage->partitionInputs[partition].size(); j++){
	  				if(!is_in_vector(aig._storage->index[curr_output], aig._storage->partitionInputs[partition].at(j))){
	  					aig._storage->index[curr_output].push_back(aig._storage->partitionInputs[partition].at(j));
	  				}
	  			}
	  			for(int j = 0; j < aig._storage->index[curr_output].size(); j++){
	  				int curr_index = aig._storage->index[curr_output].at(j); 
	  				int corrected_index = curr_index;
	  				if(aig.is_po(curr_index)){
			  			corrected_index = (aig.num_pis() + (aig.size() - curr_index));
			  			corrected_index--;
			  		}
	  				output << corrected_index + 1 << " "; 
	  			} 

	  			output << corrected_output_num  << "\n";

	  			for(int j = 0; j < aig._storage->tt[curr_output].size(); j++){

	  				for(int k = 0; k < aig._storage->tt[curr_output].at(j).size(); k++){
	  					output << aig._storage->tt[curr_output].at(j).at(k);
	  				}

	  				int remaining_inputs = aig._storage->index[curr_output].size() - aig._storage->tt[curr_output].at(j).size();
	  				for(int k = 0; k < remaining_inputs; k++){
	  					output << "-";
	  				}

	  				output << " " << output_wanted << "\n";

	  			}
	  		}
  		}
  		
  		output << ".end\n";

 		output.close();

	}

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

			  			if(connections[nodeNdx].at(i) == childIdx && !aig.is_po(node))
			  				connections[nodeNdx].erase(connections[nodeNdx].begin() + i);
			  			
			  		}
			  		
			  	}

			  	//Get truth table logic for each node
			  	auto func = aig.node_function(node);
				aig.foreach_fanin( node, [&]( auto const& conn, auto i ) {
					if ( aig.is_complemented( conn ) ) {
				       	kitty::flip_inplace( func, i );
				   	}
				});
				std::vector<int> out_int = get_logic_from_hex(kitty::to_hex(func));

				for(int i = 0; i < out_int.size(); i++){
					
					aig._storage->onset[nodeNdx].push_back(out_int.at(i));
				}

				std::vector<std::vector<int>> offset_data = other_possible_outputs(aig._storage->onset[nodeNdx]);
				aig._storage->offset[nodeNdx] = offset_data;

			});

			filename.erase(filename.end() - 4, filename.end());
			aig._storage->net_name = filename;
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
	  		output << hyperEdges.size() + 1 << " " << aig.size() /*- 1*/ << "\n";
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

	ALICE_COMMAND(node_tt, "TT logic", "Output truth table data for each node"){
		auto aig = store<mockturtle::aig_network>().current();
		aig.foreach_node( [&]( auto node ) {

			int nodeIdx = aig.node_to_index(node);
			std::cout << "truth tables for [" << nodeIdx << "]\n";
			std::cout << "index = {";
			for(int j = 0; j < aig.fanin_size(node); j++){
				std::cout << aig._storage->nodes[node].children[j].index << " ";
			}
			std::cout << "}\n";

			std::cout << "onset:\n";
			for(int i = 0; i < aig._storage->onset[nodeIdx].size(); i++){
				
				std::cout << aig._storage->onset[nodeIdx].at(i);
				
				
			}
			std::cout << " 1\n";

			std::cout << "\noffset:\n";
			for(int i = 0; i < aig._storage->offset[nodeIdx].size(); i++){
				for(int j = 0; j < aig._storage->offset[nodeIdx].at(i).size(); j++){
					std::cout << aig._storage->offset[nodeIdx].at(i).at(j);
				}
				std::cout << " 0\n";
			}
			std::cout << "\n";
		});

	}//node_tt()

	ALICE_COMMAND(test_string_insert, "test", "test string insert to be used for truth table"){

		std::vector<int> index;

		std::vector<std::vector<bool>> tt;
		int numInputs = 4;
		std::vector<bool> defaultSlate;
		for(int i = 0; i < numInputs; i++){
			defaultSlate.push_back(1);
		}
		tt.push_back(defaultSlate);

		std::vector<std::vector<bool>> testAdd = {{0,0},{0,1},{1,0}};

		int child1 = 5;
		int child2 = 8;
		index.push_back(child1);
		index.push_back(child2);

		std::cout << "index = {";
		for(int j = 0; j < index.size(); j++){
			std::cout << index.at(j) << " ";
		}
		std::cout << "}\n";

		//Wanted Out is 0
		int size = tt.size() * 3;
		for(int i = 0; i < size; i += 3){


			std::cout << "Set index: " << get_index(index, child1) << " to " << testAdd.at(0).at(0) << "\n";
			std::cout << "Set index: " << get_index(index, child2) << " to " << testAdd.at(0).at(1) << "\n";
			tt.at(i).at( get_index(index, child1)) = testAdd.at(0).at(0); 
			tt.at(i).at( get_index(index, child2)) = testAdd.at(0).at(1); 

			
			int idx = i + 1;
			for(int j = 1; j < testAdd.size(); j++){

				

				tt.insert(tt.begin() + idx, tt.at(i));

				std::cout << "Set index: " << get_index(index, child1) << " to " << testAdd.at(j).at(0) << "\n";
				std::cout << "Set index: " << get_index(index, child2) << " to " << testAdd.at(j).at(1) << "\n";

				tt.at(idx).at(get_index(index, child1)) = testAdd.at(j).at(0); 
				tt.at(idx).at(get_index(index, child2)) = testAdd.at(j).at(1); 

				idx++;
			}

		}

		std::cout << "Final after insertion:\n";
		for(int i = 0; i < tt.size(); i++){
			std::cout << "tt.at(" << i << ") ";
			for(int j = 0; j < tt.at(i).size(); j++){

			 std::cout << tt.at(i).at(j);
			}
			std::cout << "\n";
		}
		

		int child3 = 1;
		int child4 = 2;
		std::replace(index.begin(), index.end(),child1, child3);
		index.push_back(child4);

		std::vector<std::vector<int>> testAdd2 = {{1,0}};
		std::cout << "index = {";
		for(int j = 0; j < index.size(); j++){
			std::cout << index.at(j) << " ";
		}
		std::cout << "}\n";

		size = tt.size();
		for(int i = 0; i < size; i++){


			std::cout << "Set index: " << get_index(index, child3) << " to " << testAdd2.at(0).at(0) << "\n";
			std::cout << "Set index: " << get_index(index, child4) << " to " << testAdd2.at(0).at(1) << "\n";
			tt.at(i).at(get_index(index, child3)) = testAdd2.at(0).at(0); 
			tt.at(i).at(get_index(index, child4)) = testAdd2.at(0).at(1); 

			
			int idx = i + 1;
			for(int j = 1; j < testAdd2.size(); j++){

				

				tt.insert(tt.begin() + idx, tt.at(i));

				std::cout << "Set index: " << get_index(index, child3) << " to " << testAdd2.at(j).at(0) << "\n";
				std::cout << "Set index: " << get_index(index, child4) << " to " << testAdd2.at(j).at(1) << "\n";

				tt.at(idx).at(get_index(index, child3)) = testAdd2.at(j).at(0); 
				tt.at(idx).at(get_index(index, child4)) = testAdd2.at(j).at(1); 

				idx++;
			}

		}

		std::cout << "Final after insertion:\n";
		for(int i = 0; i < tt.size(); i++){
			std::cout << "tt.at(" << i << ") ";
			for(int j = 0; j < tt.at(i).size(); j++){

			 std::cout << tt.at(i).at(j);
			}
			std::cout << "\n";
		}

		int child5 = 3;
		int child6 = 4;
		std::replace(index.begin(), index.end(), child2, child5);
		index.push_back(child6);

		std::vector<std::vector<int>> testAdd3 = {{0,1},{1,0},{1,1}};
		std::cout << "index = {";
		for(int j = 0; j < index.size(); j++){
			std::cout << index.at(j) << " ";
		}
		std::cout << "}\n";

		//Wanted out is 0
		size = tt.size() * 3;
		for(int i = 0; i < size; i += 3){


			std::cout << "Set index: " << get_index(index, child5) << " to " << testAdd3.at(0).at(0) << "\n";
			std::cout << "Set index: " << get_index(index, child6) << " to " << testAdd3.at(0).at(1) << "\n";
			tt.at(i).at(get_index(index, child5)) = testAdd3.at(0).at(0); 
			tt.at(i).at(get_index(index, child6)) = testAdd3.at(0).at(1); 

			
			int idx = i + 1;
			for(int j = 1; j < testAdd3.size(); j++){

				tt.insert(tt.begin() + idx, tt.at(i));

				std::cout << "Set index: " << get_index(index, child5) << " to " << testAdd3.at(j).at(0) << "\n";
				std::cout << "Set index: " << get_index(index, child6) << " to " << testAdd3.at(j).at(1) << "\n";

				tt.at(idx).at(get_index(index, child5)) = testAdd3.at(j).at(0);
				tt.at(idx).at(get_index(index, child6)) = testAdd3.at(j).at(1);

				idx++;
			}

		}

		std::cout << "Final after insertion:\n";
		for(int i = 0; i < tt.size(); i++){
			std::cout << "tt.at(" << i << ") ";
			for(int j = 0; j < tt.at(i).size(); j++){

			 std::cout << tt.at(i).at(j);
			}
			std::cout << "\n";
		}
	}//test_string_insert()

	ALICE_COMMAND(test_truth, "test", "Output the truth table using cut enumeration"){

		if(!store<mockturtle::aig_network>().empty()){
			auto aig = store<mockturtle::aig_network>().current();
			
			mockturtle::cut_enumeration_params ps;
			ps.cut_size = 6;
			ps.cut_limit = 8;
			ps.minimize_truth_table = true;

			auto cuts = mockturtle::cut_enumeration<mockturtle::aig_network, true>( aig, ps ); /* true enables truth table computation */
			aig.foreach_node( [&]( auto n ) {


			  const auto index = aig.node_to_index( n );
			 
			  for ( auto const& set : cuts.cuts( index ) )
			  {

			  	std::cout << "Cut " << *set
			                << " with truth table " << kitty::to_hex( cuts.truth_table( *set ) )
			                << "\n";
			  	
			   /* for ( auto const& cut : (auto &) set )
			    {
			      std::cout << "Cut " << *cut
			                << " with truth table " << kitty::to_hex( cuts.truth_table( *cut ) )
			                << "\n";
			    }*/
			    
			  }
			} );

		}
		else{
			std::cout << "No AIG network stored\n";
		}
	}//test_truth()

	ALICE_COMMAND(output_truth_table, "Output", "Output the truth table for each partition in blif format"){

		int output_wanted = "";
		std::cout << "Enter output wanted i.e. 1 for the onset and 0 for the offset: ";
		std::cin >> output_wanted;

		if(!store<mockturtle::aig_network>().empty()){

			auto aig = store<mockturtle::aig_network>().current();
			//Check to see if the AIG network's nodes have been mapped to their corresponding partitions
			if(aig._storage->partitionMap.size() != 0){

				for(int i = 0; i < aig._storage->num_partitions; i++){

					aig.foreach_node( [&]( auto node ) {

						int nodeIdx = aig.node_to_index(node);
						//Check to see if the current node is in the partition
						if(aig._storage->partitionMap[nodeIdx] == i){

							for(int j = 0; j < aig._storage->partitionConn[i][nodeIdx].size(); j++){
								
								//Add to the output list for the partition if (the node is a primary output OR (if output connections go to nodes
								//that are not located in the partition AND it is not already included in the output list)) AND (the node is 
								//not an input to the partition)
								if(((aig.is_po(node) || (!is_in_map(aig._storage->partitionConn[i], aig._storage->partitionConn[i][nodeIdx].at(j))))
								 && !is_in_vector(aig._storage->partitionOutputs[i],nodeIdx)) 
									&& (!aig.is_pi(node) && !is_in_vector(aig._storage->partitionInputs[i],nodeIdx))){
									aig._storage->partitionOutputs[i].push_back(nodeIdx);
								}
								int child1Idx = aig._storage->nodes[node].children[0].index;
								int child2Idx = aig._storage->nodes[node].children[1].index;
								//Adjust child indeces if they are primary inputs
								if(child1Idx < (aig.num_pis() + aig.num_pos()))
	  								child1Idx--;
	  							if(child2Idx < (aig.num_pis() + aig.num_pos()))
	  								child2Idx--;
								bool child1 = is_in_map(aig._storage->partitionConn[i], child1Idx);
								bool child2 = is_in_map(aig._storage->partitionConn[i], child2Idx);

								//If the node has children, check if either of its children are in the partition but the other one is not
								//add the child that is not in the partition to the list of inputs
								if((aig._storage->nodes[node].children.size() != 0) && ((child1 && !child2) || (!child1 && child2))){
									if(!child1 && !is_in_vector(aig._storage->partitionInputs[i],child1Idx))
										aig._storage->partitionInputs[i].push_back(child1Idx);
									else if(!child2 && !is_in_vector(aig._storage->partitionInputs[i],child2Idx))
										aig._storage->partitionInputs[i].push_back(child2Idx);
								}

								//If neither of the children of a node are in the partition or the node is a primary input and is not yet in
								//the list of inputs, add to the list of inputs
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

				//Create the truth table for each output of the partition
				for(int i = 0; i < aig._storage->num_partitions; i++){

					std::cout << "partition = " << i << "\n";
					for(int j = 0; j < aig._storage->partitionOutputs[i].size(); j++){

						int output = aig._storage->partitionOutputs[i].at(j);
						aig._storage->tt[output].clear();
						aig._storage->index[output].clear();

						//Create a truth table that has as many indeces as nodes in the 
						//original AIG network and set the defaults for each to 1
						std::vector<int> defaultSlate;
						for(int k = 0; k < aig.size(); k++){
							defaultSlate.push_back(1);
						}
						aig._storage->tt[output].push_back(defaultSlate);

						aig._storage->index[output].push_back(output);
						aig._storage->wantedOut[output].push_back(output_wanted);
						//Make the logic value for the root node (output) the output wanted
						aig._storage->tt[output].at(0).at(0) = output_wanted;

						genTruthBFS(aig, output, aig._storage->wantedOut[output], i, output);

						std::cout << "index = {";
						for(int idx = 0; idx < aig._storage->index[output].size(); idx++){
							std::cout << aig._storage->index[output].at(idx) << " ";
						}
						std::cout << "}\n";
						for(int idx = 0; idx < aig._storage->tt[output].size(); idx++){
							std::cout << "tt.at(" << idx << ") ";
							for(int jdx = 0; jdx < aig._storage->tt[output].at(idx).size(); jdx++){

							 std::cout << aig._storage->tt[output].at(idx).at(jdx);
							}
							std::cout << "\n";
						}

						finalize_truth(aig, output, i);

						std::cout << "index = {";
						for(int idx = 0; idx < aig._storage->index[output].size(); idx++){
							std::cout << aig._storage->index[output].at(idx) << " ";
						}
						std::cout << "}\n";
						std::cout << "Final truth table\n";
						for(int idx = 0; idx < aig._storage->tt[output].size(); idx++){
							std::cout << "tt.at(" << idx << ") ";
							for(int jdx = 0; jdx < aig._storage->tt[output].at(idx).size(); jdx++){

							 std::cout << aig._storage->tt[output].at(idx).at(jdx);
							}
							std::cout << "\n";
						}
						blif_write(aig, i, output_wanted, output);
					 }
					
				}
				blif_test(aig, output_wanted);
				/*

				for(int i = 0; i < aig._storage->num_partitions; i++){

					std::cout << "Partition " << 0 << " truth table logic\n";
					for(int j = 0; j < aig._storage->partitionOutputs[0].size(); j++){
						for(int k = 0; k < aig._storage->truth[0][j].size(); k++){
							std::cout << aig._storage->truth[0][j].at(k);
						}
						std::cout << " 1\n";
					}
				}
				*/

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
