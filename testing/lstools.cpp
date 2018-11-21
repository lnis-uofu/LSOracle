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
#include <../mockturtle/include/mockturtle/networks/storage.hpp>

namespace alice{

    //testing

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

	int computeLevel( mockturtle::aig_network aig, int index ) {
		//if node not visited
        if(aig._storage->nodes[index].data[1].h1==0) {

            //set node as visited
            aig._storage->nodes[index].data[1].h1=1;

            //if is input
            if (aig.is_ci(index)) {
                return 0;
            }

			auto inIdx2 = aig._storage->nodes[index].children[1].data;
			if (inIdx2 & 1)
				inIdx2 = inIdx2 - 1;

			//calculate input node index
			auto inNode1 = inIdx2 >> 1;
			int levelNode1 = computeLevel(aig, inNode1);

            auto inIdx = aig._storage->nodes[index].children[0].data;
            if (inIdx & 1)
                inIdx = inIdx - 1;

            //calculate input node index
            auto inNode0 = inIdx >> 1;
            int levelNode0 = computeLevel(aig, inNode0);

            int level = 1 + std::max(levelNode0, levelNode1);
            return level;
        }
	}

    int computeLevelMig( mockturtle::mig_network mig, int index ) {
        //if node not visited
        if(mig._storage->nodes[index].data[1].h1==0) {

            //set node as visited
            mig._storage->nodes[index].data[1].h1=1;

            //if is input
            if (mig.is_ci(index)) {
                return 0;
            }

			auto inIdx3 = mig._storage->nodes[index].children[2].data;
			if (inIdx3 & 1)
				inIdx3 = inIdx3 - 1;

			//calculate input node index
			auto inNode3 = inIdx3 >> 1;
			int levelNode3 = computeLevelMig(mig, inNode3);

            auto inIdx2 = mig._storage->nodes[index].children[1].data;
            if (inIdx2 & 1)
                inIdx2 = inIdx2 - 1;

            //calculate input node index
            auto inNode1 = inIdx2 >> 1;
            int levelNode1 = computeLevelMig(mig, inNode1);

            auto inIdx = mig._storage->nodes[index].children[0].data;
            if (inIdx & 1)
                inIdx = inIdx - 1;

            //calculate input node index
            auto inNode0 = inIdx >> 1;
            int levelNode0 = computeLevelMig(mig, inNode0);

            int maximo = std::max(levelNode0, levelNode1);
            int max = std::max(maximo, levelNode3);

            int level = 1 + max;
            return level;
        }
    }

    void dfs (mockturtle::aig_network aig, uint64_t index, UnionFind uf){
        if(aig._storage->nodes[index].data[1].h1==0){
			//set node as visited
            aig._storage->nodes[index].data[1].h1=1;

            //traverse the left nodes to the PIs
            if(!aig.is_pi(aig._storage->nodes[index].children[0].index) && index > aig.num_pis() ){
            	auto inData0 = aig._storage->nodes[index].children[0].data;
            	//treat inverted and inputs
				if( inData0 & 1)
					inData0 = aig._storage->nodes[index].children[0].data - 1;

                //calculate input node index
                auto inIndex = inData0 >> 1;

                //ignores latches and PIs
                if(inIndex > aig.num_pis()) {
					uf.merge(index, inIndex);
					dfs(aig, inIndex, uf);
				}
            }

			//traverse the right nodes to the PIs
			if(!aig.is_pi(aig._storage->nodes[index].children[1].index) && index > aig.num_pis() ){
				auto inData1 = aig._storage->nodes[index].children[1].data;
				if( inData1 & 1)
					inData1 = aig._storage->nodes[index].children[1].data - 1;

				//calculate input node index
				auto inIndex = inData1 >> 1;

				//im ignoring latches
				if(inIndex > aig.num_pis()) {
					uf.merge(index, inIndex);
					dfs(aig, inIndex, uf);
				}
			}
        }
    }

	void stats (mockturtle::aig_network aig, uint64_t index, UnionFind uf, std::unordered_map<int, int> &numb_nodes){
		if(aig._storage->nodes[index].data[1].h1==0) {
			//set node as visited
			aig._storage->nodes[index].data[1].h1 = 1;

			std::unordered_map<int, int>::iterator it = numb_nodes.find(uf.find(index));
			if (it != numb_nodes.end()) {
				//std::cout << "Incrementing " << uf.find(index) << " for node index " << index << std::endl;
				it->second++;
			} else {
				numb_nodes.insert(std::make_pair(uf.find(index), 1));
			}

			//traverse the left nodes to the PIs
			if (!aig.is_pi(aig._storage->nodes[index].children[0].index) ) {
				if (aig._storage->nodes[index].children[0].data & 1)
					aig._storage->nodes[index].children[0].data = aig._storage->nodes[index].children[0].data - 1;

				//calculate input node index
				auto inIndex = aig._storage->nodes[index].children[0].data >> 1;

				//im ignoring latches
				if (inIndex > aig.num_pis()) {
					stats(aig, inIndex, uf, numb_nodes);
				}
			}

			//traverse the right nodes to the PIs
			if (!aig.is_pi(aig._storage->nodes[index].children[1].index) ) {
				if (aig._storage->nodes[index].children[1].data & 1)
					aig._storage->nodes[index].children[1].data = aig._storage->nodes[index].children[1].data - 1;
				//calculate input node index
				auto inIndex = aig._storage->nodes[index].children[1].data >> 1;
				//im ignoring latches
				if (inIndex > aig.num_pis()) {
					stats(aig, inIndex, uf, numb_nodes);
				}
			}
		}
	}

	void compute_cone(mockturtle::aig_network aig, uint64_t index, std::unordered_map<int, int> &nodes, int outindex, std::unordered_map<int, int> &ins){
		if(aig._storage->nodes[index].data[1].h1==0){

			//increment number of nodes in this cone
			std::unordered_map<int, int>::iterator it = nodes.find(outindex);

			if(it!=nodes.end() && index > aig.num_pis()) {
				//increment the number of nodes
				it->second++;
			}

			//set node as visited
			aig._storage->nodes[index].data[1].h1=1;

			//traverse one side to the PIs
			if(!aig.is_pi(aig._storage->nodes[index].children[0].index) && index > aig.num_pis() ){
				if(aig._storage->nodes[index].children[0].data & 1)
					aig._storage->nodes[index].children[0].data = aig._storage->nodes[index].children[0].data - 1;

				//calculate input node index
				auto inIndex = aig._storage->nodes[index].children[0].data >> 1;

				//im ignoring latches
				if(inIndex > aig.num_pis()) {
					//call recursion
					compute_cone(aig, inIndex, nodes, outindex, ins);
				}
			}

			//traverse the other side to the PIs
			if(!aig.is_pi(aig._storage->nodes[index].children[1].index)&& index > aig.num_pis() ){
				if(aig._storage->nodes[index].children[1].data & 1)
					aig._storage->nodes[index].children[1].data = aig._storage->nodes[index].children[1].data - 1;

				//calculate input node index
				auto inIndex = aig._storage->nodes[index].children[1].data >> 1;

				//im ignoring latches
				if(inIndex > aig.num_pis()) {
					//call recursion
					compute_cone(aig, inIndex, nodes, outindex, ins);
				}
			}

			//if my child is PI and was not visited yet, I increase the input counter
			if(aig.is_ci(aig._storage->nodes[index].children[0].index) && aig._storage->nodes[aig._storage->nodes[index].children[0].index].data[1].h1 == 0 ){
				aig._storage->nodes[aig._storage->nodes[index].children[0].index].data[1].h1 = 1;

				std::unordered_map<int, int>::iterator it = ins.find(outindex);
				if(it!=ins.end()) {
					//increment the number of inputs
					it->second++;
				}
			}

			//if my other child is PI and was not visited yet, I also increase the input counter
			if(aig.is_ci(aig._storage->nodes[index].children[1].index) && aig._storage->nodes[aig._storage->nodes[index].children[1].index].data[1].h1 == 0){
				aig._storage->nodes[aig._storage->nodes[index].children[1].index].data[1].h1 = 1;

				std::unordered_map<int, int>::iterator it = ins.find(outindex);
				if(it!=ins.end()) {
					//increment the number of inputs
					it->second++;
				}
			}
		}
	}



	/* Adds And-inverter graphs (Mockturtle type aig_network) as store element type to
	 * alice.
	 *
	 * One can access AIGs in general store commands using the long --aig flag or
	 * the short -a flag.
	 */
	ALICE_ADD_STORE( mockturtle::aig_network, "aig", "a", "aig", "AIGs" )

	/* Implements the short string to describe a store element in store -a */
	ALICE_DESCRIBE_STORE( mockturtle::aig_network, aig ){

	  const auto name = "aig_placeholder";
	  const auto pi_num = aig.num_pis();
	  const auto po_num = aig.num_pos();
	  return fmt::format( "{} i/o = {}/{}", name, pi_num, po_num );
	}//end aig_network describe store

	ALICE_LOG_STORE_STATISTICS( mockturtle::aig_network, aig){

	  return {
	  	{"nodes", aig.size()},
	  	{"inputs", aig.num_pis()},
	  	{"latches", aig.num_latches()},
	  	{"outputs", aig.num_pos()},
	  	{"gates", aig.num_gates()}};
	}//end aig_network log store statistics

	/* Implements the functionality of ps -b */
	ALICE_PRINT_STORE_STATISTICS( mockturtle::aig_network, os, aig ){
		os << "nodes: " << aig.size() << std::endl;
	  	os << "inputs: " << aig.num_pis()  << std::endl;
		os << "latches: " << aig.num_latches() << std::endl;
		os << "outputs: " << aig.num_pos() << std::endl;
	  	os << "gates: " << aig.num_gates() << std::endl;

	}//end aig_network print store statistics

	ALICE_ADD_STORE( mockturtle::mig_network, "mig", "m", "mig", "MIGs" )

	/* Implements the short string to describe a store element in store -a */
	ALICE_DESCRIBE_STORE( mockturtle::mig_network, mig ){

		const auto name = "mig_placeholder";
		const auto pi_num = mig.num_pis();
		const auto po_num = mig.num_pos();

		return fmt::format( "{} i/o = {}/{}", name, pi_num, po_num );
	}//end aig_network describe store

	ALICE_LOG_STORE_STATISTICS( mockturtle::mig_network, mig){

		return {
				{"nodes", mig.size()},
				{"inputs", mig.num_pis()},
				{"latches", mig.num_latches()},
				{"outputs", mig.num_pos()},
				{"gates", mig.num_gates()}};
	}//end aig_network log store statistics

	/* Implements the functionality of ps -b */
	ALICE_PRINT_STORE_STATISTICS( mockturtle::mig_network, os, mig ){
		os << "nodes: " << mig.size() << std::endl;
		os << "inputs: " << mig.num_pis() << std::endl;
		os << "latches: " << mig.num_latches() << std::endl;
		os << "outputs: " << mig.num_pos() << std::endl;
		os << "gates: " << mig.num_gates() << std::endl;
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

	ALICE_ADD_STORE(abc::Gia_Man_t*, "gia", "g", "AIG", "AIGs")

    /* Implements the short string to describe a store element in store -a */
    ALICE_DESCRIBE_STORE( abc::Gia_Man_t*, gia )
    {
        const auto name = abc::Gia_ManName( gia );
        const auto pi_num = abc::Gia_ManPiNum( gia );
        const auto po_num = abc::Gia_ManPoNum( gia );
        return fmt::format( "{} i/o = {}/{}", name, pi_num, po_num );
    }

    /* Implements the functionality of ps -a */
    ALICE_PRINT_STORE_STATISTICS( abc::Gia_Man_t*, os, gia )
    {
        abc::Gps_Par_t Pars{};
        abc::Gia_ManPrintStats( gia, &Pars );
    }

    /* Implements the log returned by ps -a */
    ALICE_LOG_STORE_STATISTICS( abc::Gia_Man_t*, gia )
    {
        return {
                {"name", abc::Gia_ManName( gia )},
                {"inputs", abc::Gia_ManPiNum( gia )},
                {"outputs", abc::Gia_ManPoNum( gia )},
                {"nodes", abc::Gia_ManAndNum( gia )},
                {"latches"}, abc::Gia_ManRegNum(gia),
                {"levels", abc::Gia_ManLevelNum( gia )}};
    }

    ALICE_ADD_FILE_TYPE( aiger, "Aiger" )

/* Implements the functionality of read_aiger -a */
    ALICE_READ_FILE( abc::Gia_Man_t*, aiger, filename, cmd )
    {
        return abc::Gia_AigerRead( (char*)filename.c_str(), 0, 0, 0 );
    }

/* Implements the functionality of write_aiger -a */
    ALICE_WRITE_FILE( abc::Gia_Man_t*, aiger, gia, filename, cmd )
    {
        abc::Gia_AigerWrite( gia, (char*)filename.c_str(), 1, 0 );
    }

/* Implements the command syn3 */
    ALICE_COMMAND( syn3, "Optimization", "Performs AIG optimization" )
    {
        auto& gia = store<abc::Gia_Man_t*>().current();

        auto aig_new = abc::Gia_ManAigSyn3( gia, 0, 0 );
        abc::Gia_ManStop( gia );
        gia = aig_new;
    }

	/*Reads an blif file and stores the CBA network in a store*/
	ALICE_COMMAND( get_blif, "Input", "Uses the lorina library to read in a blif file" ){

		std::cout << "Not yet fully implemented\n";

		/*std::string filename = "";
		std::cout << "Enter blif filename: ";
		std::cin >> filename;
		read_blif(filename, blif());*/
		/*TODO check to make sure that the file entered is valid and add store*/	
	}//end get_blif

	ALICE_COMMAND( get_mig, "Input", "Uses the lorina library to read in an aig file into a mig network" ){

		std::string filename = "";
		std::cout << "Enter aig filename: ";
		std::cin >> filename;

		if(checkExt(filename, "aig")){
			mockturtle::mig_network mig;
			lorina::read_aiger(filename, mockturtle::aiger_reader( mig ));
			std::cout << "AIG file = " << filename << " stored" << std::endl;
			store<mockturtle::mig_network>().extend() = mig;

			std::cout << "MIG size = " << mig.size() << "\n";
		}
	}

    ALICE_COMMAND( read_aig, "Input", "Uses the lorina library to read in an aig file" ){

        std::string filename = "";
        std::cout << "Enter aig filename: ";
        std::cin >> filename;

        if(checkExt(filename, "aig")){
            mockturtle::aig_network aig;
            lorina::read_aiger(filename, mockturtle::aiger_reader( aig ));
            std::cout << "AIG file = " << filename << " stored" << std::endl;
            store<mockturtle::aig_network>().extend() = aig;
        }
    }

	/*Reads an aig file and stores the AIG network in a store*/
	/*
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
			});

			aig.add_connections_network(connections);
		}
		else{
			std::cout << filename << " is not a valid aig file\n";
		}
	}//end get_aig*/


	/*Reads a bench file and store the k-LUT network in a store*/
/*
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
	    add_flag("--mig,-m", "Write the stored AIG file to a bench file");
	    add_flag("--bench,-b", "Write the stored bench file to a bench file");
	  }

	protected:
	  void execute(){

	    if(is_set("mig")){
	    	mockturtle::mig_network mig = store<mockturtle::mig_network>().current();
		    mockturtle::write_bench(mig, filename);
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
	/*

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

	}*/

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

	ALICE_COMMAND(show_mig, "Output", "Writes the the mig dot file"){
		auto mig = store<mockturtle::mig_network>().current();

		for (int j =1; j < mig._storage->nodes.size(); j++) {
			for (int i = 0; i < mig._storage->nodes.data()->children.size(); i++) {
				std::cout << "node index " << j << " node fan in " << mig._storage->nodes[j].children[i].data << std::endl;
			}
		}
		for (unsigned k = mig.num_pis()+1; k<= mig._storage->inputs.size(); k++ ){
			auto node = mig.index_to_node(k);
			std::cout << " reg " << k << " fan out size " << mig.fanout_size(node) << std::endl;
            //mig._storage->nodes[node].data
        }
        for (unsigned l=0; l< mig._storage->outputs.size(); l++){
            std::cout << " outputs " << std::endl;
            //std::cout << " node fan in index " << mig._storage->outputs[l].index << std::endl;
            std::cout << " node fan in data " << mig._storage->outputs[l].data << std::endl;
        }
	}

    ALICE_COMMAND(test_sta, "Analysis", "Runs STA"){
//       ot::Timer timer;
//
//       timer.read_celllib("ot/osu018_stdcells.lib", std::nullopt)  // read the library (O(1) builder)
//			   .read_verilog("ot/simple.v")                  // read the verilog netlist (O(1) builder)
//			   .read_spef("ot/simple.spef")                  // read the parasitics (O(1) builder)
//			   .read_sdc("ot/simple.sdc")                    // read the design constraints (O(1) builder)
//			   .update_timing();                          // update timing (O(1) builder)
//
//		if(auto tns = timer.tns(); tns) std::cout << "TNS: " << *tns << '\n';  // (O(N) action)
//		if(auto wns = timer.wns(); wns) std::cout << "WNS: " << *wns << '\n';  // (O(N) action))
//
//		timer.dump_timer(std::cout);                    // dump the timer details (O(1) accessor)
    }

    ALICE_COMMAND(show_aig, "Output", "Writes the the aig dot file"){

//		std::string filename = " ";
//		std::cout << "Enter filename: ";
//		std::cin >> filename;
//
//		std::ofstream dump;
//		dump.open (filename);

        auto aig = store<mockturtle::aig_network>().current();

        for (int j =1; j < aig._storage->nodes.size(); j++) {
            for (int i = 0; i < aig._storage->nodes.data()->children.size(); i++) {
				std::cout << "node index " << j << " node fan in " << aig._storage->nodes[j].children[i].data << std::endl;
            }
        }

        for (unsigned k = aig.num_pis()+1; k<= (aig._storage->inputs.size() - aig.num_latches()); k++ ){
        	auto node = aig.index_to_node(k);
			std::cout << " reg " << k << " fan out size " << aig.fanout_size(node) << std::endl;
        }

		for( int outIndex=0; outIndex<aig.num_pos()- aig.num_latches(); outIndex++){
			std::cout << "Output " << outIndex << " data " << aig._storage->outputs[outIndex].data << std::endl;
			//std::cout << " node fan in index " << aig._storage->outputs[l].index << std::endl;
		}

		aig.foreach_ri([&](auto fi) {
			std::cout << "Register " << aig.ri_index(fi) << " data " << fi.data << std::endl;
		});
		//dump.close();
    }

    ALICE_COMMAND(print_eachout, "Output", "Testing foreach") {
		auto aig = store<mockturtle::aig_network>().current();
		static int counter = 0;
		aig.foreach_po([&](auto fi) {
			std::cout << "Out index " << aig.po_index(fi) << " Counter " << counter << std::endl;
		});
		counter++;
	}

	ALICE_COMMAND(depth, "Output", "Testing depth") {
		auto aig = store<mockturtle::aig_network>().current();

		mockturtle::depth_view aig_depth{aig};

		std::cout << "Aig level " << aig_depth.depth()  << std::endl;

	}

	ALICE_COMMAND(get_cones, "Output", "Get logical cones for each PO") {
//        std::string filename = " ";
//        std::cout << "Enter filename: ";
//        std::cin >> filename;
//
//        std::ofstream conesfile;
//        conesfile.open (filename);

		auto aig = store<mockturtle::aig_network>().current();

		//map with number of nodes in each logical cone
		std::unordered_map<int, int> po_nodes;
		std::unordered_map<int, int> ri_nodes;

		//number of inputs for each cone
		std::unordered_map<int, int> po_ins;
		std::unordered_map<int, int> ri_ins;

		//first processing logical cones for POs
		//aig.foreach_po([&](auto fi) {
		for(int outIndex=0; outIndex<aig.num_pos()- aig.num_latches(); outIndex++) {

			aig.foreach_node([&](auto node) {
				//set all nodes as not visited
				aig._storage->nodes[node].data[1].h1 = 0;
			});

			//PO index to keep track of each cone on the map structure
			//auto outIndex = aig.po_index(fi);

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
			compute_cone(aig, inIdx, po_nodes, outIndex, po_ins);

			aig.foreach_node([&](auto node) {
				//set all nodes as not visited
				aig._storage->nodes[node].data[1].h1 = 0;
			});

			int level = computeLevel(aig, inIdx);
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
			//});
		}

		//processing logical cones for registers - pseudo POs
		aig.foreach_ri([&](auto fi) {
			aig.foreach_node([&](auto node) {
				//set all nodes as not visited
				aig._storage->nodes[node].data[1].h1 = 0;
			});

            //std::cout << "Entrando no foreach RI " << std::endl;

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
            compute_cone(aig, inIndex, ri_nodes, outIndex, ri_ins);

            aig.foreach_node([&](auto node) {
                //set all nodes as not visited
                aig._storage->nodes[node].data[1].h1 = 0;
            });

            int level = computeLevel(aig, inIndex);
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
		//conesfile.close ();
    }

    ALICE_COMMAND(disjoint_clouds, "Output", "Writes the status for each disjoint combinational cloud on the current AIG"){
		std::string filename = " ";
		std::cout << "Enter filename: ";
		std::cin >> filename;

		std::ofstream dset;
		dset.open (filename);

		auto aig = store<mockturtle::aig_network>().current();

		//maps to keep the depth and numb of nodes for each comb cloud
		std::unordered_map<int, int> outputs;
		std::unordered_map<int, int> depth;
		std::unordered_map<int, int> number_nodes;


		//copy sets in a local vector
		std::vector<int> clouds;

	    //number of aig inputs + 1 to start at the nodes index
	    uint64_t inSize = aig.num_pis()+1;

	    //union find constructor - inSize sets the vector index where and nodes start
        UnionFind uf(aig.size()-1, inSize);

		//set all nodes as not visited for the DFS
		aig.foreach_node([&] (auto node) {
            aig._storage->nodes[node].data[1].h1 = 0;
        });

	    //first looking to latches
	    aig.foreach_ri( [&] (auto ri)
        {
	    	auto indata = ri.data;

           if(indata & 1)
               indata = indata - 1;

           //calculate latch input node index
           auto inIndex = indata >> 1;

           if(inIndex>aig.num_pis()) {
			   dfs(aig, inIndex, uf);
		   }
		});

		aig.foreach_node([&] (auto node) {
			//set all nodes as not visited
			aig._storage->nodes[node].data[1].h1 = 0;
		});

	    //then considering POs
	    for(int i = 0; i<aig.num_pos()-aig.num_latches(); i++){
			auto inIdx = aig._storage->outputs[i].data;
			if(aig._storage->outputs[i].data & 1)
				inIdx = aig._storage->outputs[i].data - 1;
			//calculate the index of the node driving the output
			inIdx = inIdx >> 1;
			if(inIdx>aig.num_pis()) {
				dfs(aig, inIdx, uf);
			}
	    }

	    uf.get_sets(clouds);

	    for (int j = 0; j < clouds.size() ; ++j) {
			auto it = number_nodes.find (clouds[j]);
			//element not in the table yet
			if(it==number_nodes.end()){
				int count = std::count(std::begin(clouds), std::end(clouds), clouds[j]);
				number_nodes.insert(std::make_pair(clouds[j],count));
			}
		}

		for (std::pair<int, int> element : number_nodes)
		{
			dset << element.second << std::endl;
		}

		std::cout << "Number of disjoint graphs: " << uf.getNumbOfSets() << std::endl;

		dset.close();
	}

    ALICE_COMMAND(write_verilog, "Output", "Writes the Boolean network into structural verilog") {
	    auto& mig = store<mockturtle::mig_network>().current();

        std::string filename = " ";
        std::cout << "Enter filename: ";
        std::cin >> filename;

        mockturtle::write_verilog(mig, filename);
    }

        ALICE_COMMAND(write_aig, "Output", "Writes the AIG in the ASCII format"){
		auto aig = store<mockturtle::aig_network>().current();

		std::string filename = " ";
		std::cout << "Enter aiger filename: ";
		std::cin >> filename;

		std::ofstream aigfile;
		aigfile.open (filename);

		// header info - MILOA
		auto _num_inputs = aig.num_pis();
		auto _num_latches = aig.num_latches();
		auto _num_outputs = aig.num_pos();
		auto _num_ands = aig.num_gates();
		auto _num_vertices = aig.num_pis() + aig.num_gates();

		//write aig header
		aigfile << "aag " << _num_vertices << " " << _num_inputs - _num_latches << " " << _num_latches << " " << (_num_outputs - _num_latches) << " " << _num_ands << std::endl;

		aig.foreach_pi([&] (auto node)
		{
			auto index = aig.pi_index(node);
			aigfile << (index+1)*2 << "\n";
		});

		//write aig latches
		auto lineIdx = ((aig.num_pis()-aig.num_latches())*2)+2;
		for(int i=0; i<aig.num_latches(); i++){
			auto regIdx = aig.num_pos() - aig.num_latches() + i;
			aigfile << lineIdx << " " << aig._storage->outputs[regIdx].data << "\n";
			lineIdx += 2;
		}

		//write aig outputs
		for(int i=0; i<aig.num_pos()- aig.num_latches(); i++){
			aigfile << aig._storage->outputs[i].data << "\n";
		}

		auto skipLatches = 0;
		aig.foreach_gate([&] (auto node)
		{
			//skip latches in the nodes vector
			if(skipLatches>=aig.num_latches()) {
				auto index = aig.node_to_index(node);
				auto left = aig._storage->nodes[index].children[0].data;
				auto right = aig._storage->nodes[index].children[1].data;
				aigfile << index*2 << " " << left << " " << right << "\n";
			}
			skipLatches+=1;
		});

		aigfile.close();
	}

	ALICE_COMMAND(tmap, "Transformation", "Performs LUT techmapping") {
		auto mig = store<mockturtle::mig_network>().current();;
		mockturtle::mapping_view<mockturtle::mig_network, true> mapped_mig{mig};

		mockturtle::lut_mapping_params ps;
		ps.cut_enumeration_ps.cut_size = 6;
		mockturtle::lut_mapping(mapped_mig, ps);
		//mockturtle::lut_mapping<mapped_view<mig_network, true>, true>( mapped_mig );
	}

    ALICE_COMMAND(interleaving, "Modification", "NPN + depth MIG rewriting") {
        auto& mig = store<mockturtle::mig_network>().current();

        mockturtle::mig_npn_resynthesis resyn;
        mockturtle::cut_rewriting_params ps;

        ps.cut_enumeration_ps.cut_size = 4;

        mockturtle::cut_rewriting(mig, resyn, ps);
        mig = mockturtle::cleanup_dangling( mig );

    }


	ALICE_COMMAND(migscript, "Modification", "Exact NPN MIG rewriting") {
		auto& opt = store<mockturtle::mig_network>().current();
        mockturtle::depth_view mig_depth{opt};

		//DEPTH REWRITING
        std::cout << "Mig level " << mig_depth.depth() << " mig gates " << opt.num_gates() << std::endl;

        mockturtle::mig_script migopt;
        opt = migopt.run(opt);

	}

	ALICE_COMMAND(depthr, "Modification", "Logic depth oriented MIG rewriting"){
	    auto& mig = store<mockturtle::mig_network>().current();
		std::cout << "Mig gates " << mig.num_gates() << std::endl;

		//to compute at level
		mockturtle::depth_view mig_depth{mig};

		std::cout << "Mig level " << mig_depth.depth() << " mig gates " << mig.num_gates() << std::endl;

		mockturtle::mig_algebraic_depth_rewriting_params pm;
		pm.aggressive;
		mockturtle::mig_algebraic_depth_rewriting(mig_depth, pm);

		mig = mockturtle::cleanup_dangling( mig );
		std::cout << "Mig level after algebraic rewriting " << mig_depth.depth() << " Mig gates " << mig.num_gates() << std::endl;

//		mockturtle::mig_npn_resynthesis resyn;
//		mockturtle::cut_rewriting_params ps;
//		ps.cut_enumeration_ps.cut_size = 4;
//		mockturtle::cut_rewriting(mig, resyn, ps);
//		//mig = mockturtle::cleanup_dangling(mig);
//
//        std::cout << " mig gates after rewriting " << mig.num_gates() << std::endl;

//		mockturtle::refactoring(mig, resyn);
//		//mockturtle::resubstitution (mig);
//		mig = cleanup_dangling( mig );
//
//		std::cout << " mig gates after refact 2" << mig.num_gates() << std::endl;
//
//		//mockturtle::resubstitution (mig);
//		//mig = cleanup_dangling( mig );
//
//		std::cout << " mig gates after 2 resub " << mig.num_gates() << std::endl;
	}

	ALICE_COMMAND(test_ci, "Output", "Testing mig is ci"){
		auto mig = store<mockturtle::mig_network>().current();

		mig.foreach_ci([&] (auto node) {
			if(mig.is_ci(node)){
				std::cout << "Found CI " << std::endl;
			}
		});
	}

    ALICE_CONVERT( mockturtle::mig_network, element, mockturtle::aig_network )
    {
	    auto aig = mockturtle::aig_network();
		std::unordered_set<uint32_t> toinvert;
		std::unordered_set<uint32_t> duplicated;
		std::map<uint32_t , mockturtle::aig_network::signal> update_output ;
		std::vector<int> missing;

        //create aig inputs
		for (int i = 0; i < element.num_pis() - element.num_latches(); ++i) {
			aig.create_pi();
		}

        //create registers output (PIs in the AIG)
        for (int i=0; i<element.num_latches(); i++){
        	aig.create_ro();
        }

        //convert majs into ands
		element.foreach_gate([&] (auto gate) {
			auto index = element.node_to_index(gate);

			//ignore PIs and Latch inputs
			if(index > element.num_pis()) {

				if(element._storage->nodes[index].children[0].data == 1){

					auto child2 = element._storage->nodes[index].children[1];
					auto child3 = element._storage->nodes[index].children[2];

					auto search = toinvert.find(2 * child2.index);
					if (search == toinvert.end()) {
						child2 = aig.create_not(child2);
					}

					auto search1 = toinvert.find(2 * child3.index);
					if (search1 == toinvert.end()) {
						child3 = aig.create_not(child3);
					}

					//add complemented nodes. If a complemented node (first child equals to 1) is fan-in of a node, it should be complemented while generating the aig
					toinvert.insert(2 * index);

					aig.create_maj(aig.child_to_signal(0), child2, child3);
				}

				else if (element._storage->nodes[index].children[0].data == 0){
					auto child2 = element._storage->nodes[index].children[1];
					auto child3 = element._storage->nodes[index].children[2];

					auto search = toinvert.find(2 * child2.index);
					if (search != toinvert.end()) {
						child2 = aig.create_not(child2);
						duplicated.insert(child2.index * 2);
						toinvert.erase(2*index);
					}

					auto search1 = toinvert.find(2 * child3.index);
					if (search1 != toinvert.end()) {
						child3 = aig.create_not(child3);
						duplicated.insert(child3.index * 2);
					}

					aig.create_maj(aig.child_to_signal(0), child2, child3);
				}

				else{
					auto child1 = element._storage->nodes[index].children[0];
					auto child2 = element._storage->nodes[index].children[1];
					auto child3 = element._storage->nodes[index].children[2];

					auto search = toinvert.find(2 * child1.index);
					auto search0 = duplicated.find(2 * child1.index);
					if (search != toinvert.end() && search0 == duplicated.end()) {
						child1 = aig.create_not(child1);
					}

					auto search1 = duplicated.find(2 * child2.index);
					auto search2 = toinvert.find(2 * child2.index);
					if (search2 != toinvert.end() && search1 == duplicated.end()) {
						child2 = aig.create_not(child2);
					}

					auto search3 = toinvert.find(2 * child3.index);
					auto search4 = duplicated.find(2*child3.index);
					if (search3 != toinvert.end() && search4 == duplicated.end()) {
						child3 = aig.create_not(child3);
					}

					auto maj = aig.create_maj(child1, child2, child3);

					std::cout << "Inserting in the table " << 2*index << " " << maj.index << std::endl;
					update_output.insert(std::make_pair(2*index, maj));

				}
			}
		});

		aig.foreach_gate([&] (auto gate) {
			auto index = aig.node_to_index(gate);

			//ignore PIs and Latch inputs
			if(index > aig.num_pis()) {

				auto child1 = aig._storage->nodes[index].children[0];
				auto child2 = aig._storage->nodes[index].children[1];

				auto it = update_output.find(2*child1.index);

				if(it!=update_output.end()){
					if(index < (it->first)/2) {
						//it->second.complement = it->second.complement ^ 1;
						std::cout << "Child1! My index " << index <<std::endl;
						std::cout << "Replacing " << 2*child1.index << " by " << it->second.index << " polarity " << it->second.complement << std::endl;
						aig._storage->nodes[index].children[0] = it->second;
						//aig.update();
					}
				}

				auto it1 = update_output.find(2*child2.index);

				if(it1!=update_output.end()){
					if(index < (it1->first)/2) {
						//it1->second.complement = it1->second.complement ^ 1;
						std::cout << "Child2! My index " << index <<std::endl;
						std::cout << "Replacing " << 2*child2.index << " by " << it1->second.index << " polarity " << it1->second.complement << std::endl;
						aig._storage->nodes[index].children[1] = it1->second;
						//aig.update();
					}
				}
			}
		});

        //create aig outputs
		for(int i = 0; i < element.num_pos() - element.num_latches(); i++){
			auto in = element._storage->outputs[i].data;
			auto signal = element.child_to_signal(in);
			auto signalOut = mockturtle::aig_network::signal(signal);

			auto it = update_output.find(in);

			if(it!=update_output.end()){
				//std::cout << "Need to replace node " << in << " by " << it->second.index << std::endl;
				signalOut = it->second;
			}

			else {
				auto search = toinvert.find(1 ^ in);
				if (search != toinvert.end()) {
					signalOut = aig.create_not(signalOut);
				}

				auto search1 = toinvert.find(in);
				if (search1 != toinvert.end()) {
					signalOut = aig.create_not(signalOut);
				}
			}
			aig.create_po(signalOut);
		}

		//create registers input as AIG PO
		for (unsigned int i = 0; i < element.num_latches() ; ++i) {
			int8_t reset = 0;
			std::string name = "";
			auto regIdx = element.num_pos() - element.num_latches() + i;
			auto signal = element.ri_at(regIdx);

			bool comp = false;
			auto temp = signal.data;

			auto it = update_output.find(2*signal.index);

			if(it!=update_output.end()){
				//std::cout << "Need to replace node " << in << " by " << it->second.index << std::endl;
				auto ri = it->second;
				aig.create_ri(ri, reset, name);

			}

			else{
				if (signal.data & 1) {
					auto temp = signal.data - 1;
				}

				auto nodeIn = signal.data / 2;
				if (element._storage->nodes[nodeIn].children[0].data == 1) {
					comp = true;
				}

				auto ri = mockturtle::aig_network::signal(signal);

				if (comp) {
					ri = aig.create_not(ri);
				}

				aig.create_ri(ri, reset, name);
			}
		}
		return aig;
    }

   /* ALICE_COMMAND(rewrite_mig, "Optimization", "Runs MIG rewrite"){
        auto mig = store<mockturtle::mig_network>().current();
        mockturtle::mig_algebraic_depth_rewriting_params ps;
        ps.strategy = mockturtle::mig_algebraic_depth_rewriting_params::selective;
        mockturtle::mig_algebraic_depth_rewriting(mig, ps);
    }*/

	ALICE_COMMAND(test_aig, "Output", "Writes the the mig dot file"){
		auto aig = store<mockturtle::aig_network>().current();

		aig.foreach_node( [&] (auto node)
						  {
							  for (int i=0; i < aig._storage->nodes.data()->children.size() ; i++ ){
								  std::cout << "node index " << node << " node fan in " << aig._storage->nodes.data()->children[i].data  << std::endl;
							  }
						  });
	}
/*
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
	*/

} // namespace alice

/* Main method for the Alice shell (with prefix) */
ALICE_MAIN( lstools )