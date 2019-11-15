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
  ALICE_COMMAND(get_cones, "Stats", "Displays size and depth of all logic cones in the stored AIG network") {

    if(!store<aig_ntk>().empty()){
    	auto aig = *store<aig_ntk>().current();

    	//map with number of nodes in each logical cone
    	std::unordered_map<int, int> po_nodes;
    	std::unordered_map<int, int> ri_nodes;

    	//number of inputs for each cone
    	std::unordered_map<int, int> po_ins;
    	std::unordered_map<int, int> ri_ins;

    	//first processing logical cones for POs
    	for(int outIndex=0; outIndex<aig.num_pos()- aig.num_latches(); outIndex++) {

    		aig.foreach_node([&](auto node) {
    			//set all nodes as not visited
    			aig._storage->nodes[node].data[1].h1 = 0;
    		});

    		//start counter for a given output index
    		po_nodes.insert(std::make_pair(outIndex, 0));

    		//starting the counter of inputs
    		po_ins.insert(std::make_pair(outIndex, 0));

    		//calculate the index of the node driving the output
    		auto inIdx = aig._storage->outputs[outIndex].data;
    		if (aig._storage->outputs[outIndex].data & 1)
    			inIdx = aig._storage->outputs[outIndex].data - 1;

    		inIdx = inIdx >> 1;

    		//call DFS
    		oracle::compute_cone(aig, inIdx, po_nodes, outIndex, po_ins);

    		aig.foreach_node([&](auto node) {
    			//set all nodes as not visited
    			aig._storage->nodes[node].data[1].h1 = 0;
    		});

    		int level = oracle::computeLevel(aig, inIdx);
    		int nodes = 0;
    		int inputs = 0;

    		// for each output prints index, nodes, depth and number of inputs, respectively
    		std::unordered_map<int, int>::iterator it;
    		it = po_nodes.find(outIndex);

    		if (it != po_nodes.end())
    			nodes = it->second;

    		std::unordered_map<int, int>::iterator init;
    		init = po_ins.find(outIndex);

    		if (it != po_nodes.end())
    			inputs = init->second;

    		std::cout << "Output " << outIndex << " " << nodes << " " << level << " " << inputs << std::endl;
    		
    	}

    	//processing logical cones for registers - pseudo POs
    	aig.foreach_ri([&](auto fi) {
    		aig.foreach_node([&](auto node) {
    			//set all nodes as not visited
    			aig._storage->nodes[node].data[1].h1 = 0;
    		});

              //PO index to keep track of each cone on the map structure
    		auto outIndex = aig.ri_index(fi);

    		//start counter for a given ri index
    		ri_nodes.insert(std::make_pair(outIndex, 0));
    		ri_ins.insert(std::make_pair(outIndex, 0));

    		//calculate the index of the node driving the output
    		auto inIndex = fi.data;
    		if(fi.data & 1)
    			inIndex = fi.data - 1;

    		//calculate latch input node index
    		inIndex = inIndex >> 1;

    		//call DFS
        oracle::compute_cone(aig, inIndex, ri_nodes, outIndex, ri_ins);

        aig.foreach_node([&](auto node) {
            //set all nodes as not visited
            aig._storage->nodes[node].data[1].h1 = 0;
        });

        int level = oracle::computeLevel(aig, inIndex);
        int nodes = 0;
    		int inputs = 0;

    		// for each output prints index, nodes, depth and number of inputs, respectively
    		std::unordered_map<int, int>::iterator it;
    		it = ri_nodes.find(outIndex);

    		if(it != po_nodes.end())
    			nodes = it->second;

    		std::unordered_map<int, int>::iterator init;
    		init = ri_ins.find(outIndex);

    		if(it != po_nodes.end())
    			inputs = init->second;

    		std::cout << "Register " << outIndex << " " << nodes << " " << level << " " << inputs << std::endl;
    	});
    }
    else{
      std::cout << "There is not an AIG network stored.\n";
    }
  }
}