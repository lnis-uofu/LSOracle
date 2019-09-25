//#include <darknet.h>
#include <fdeep/fdeep.hpp>
#include <lorina/lorina.hpp>
#include <alice/alice.hpp>
#include <kitty/kitty.hpp>
#include <mockturtle/mockturtle.hpp>
#include <oracle/oracle.hpp>
#include <libkahypar.h>

#include <mockturtle/views/mffc_view.hpp>
#include <mockturtle/views/fanout_view.hpp>

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
#include <sys/stat.h>
#include <sys/types.h>
#include <iomanip>
#include <ot/timer/timer.hpp>

namespace alice{

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

  using xag_names = mockturtle::names_view<mockturtle::xag_network>;
  using xag_ntk = std::shared_ptr<xag_names>;

  //object to hold STA configuration
  STA sta_cfg;



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

  ALICE_ADD_STORE( mockturtle::aig_network, "old_aig", "o_a", "old_aig", "OLD_AIGs" )

  /* Implements the short string to describe a store element in store -a */
  ALICE_DESCRIBE_STORE( mockturtle::aig_network, aig ){

    const auto name = "aig_placeholder";
    const auto pi_num = aig.num_pis();
    const auto po_num = aig.num_pos();
    return fmt::format( "{} i/o = {}/{}", name, pi_num, po_num );
  }//end aig_network describe store

  ALICE_LOG_STORE_STATISTICS( mockturtle::aig_network, aig){
    mockturtle::depth_view depth{aig};
    return {
      {"nodes", aig.size()},
      {"inputs", aig.num_pis() - aig.num_latches()},
      {"latches", aig.num_latches()},
      {"outputs", aig.num_pos() - aig.num_latches()},
      {"AIG nodes", aig.num_gates()},
      {"AIG level", depth.depth()}};
  }//end aig_network log store statistics

  /* Implements the functionality of ps -b */
  ALICE_PRINT_STORE_STATISTICS( mockturtle::aig_network, os, aig ){
    mockturtle::depth_view depth{aig};
    os << "nodes: " << aig.size() << std::endl;
    os << "inputs: " << aig.num_pis() - aig.num_latches() << std::endl;
    os << "latches: " << aig.num_latches() << std::endl;
    os << "outputs: " << aig.num_pos() - aig.num_latches() << std::endl;
    os << "AIG nodes: " << aig.num_gates() << std::endl;
    os << "AIG level: " << depth.depth() << std::endl;

  }//end aig_network print store statistics

  ALICE_ADD_STORE( mockturtle::NameMap<mockturtle::aig_network>, "old_aig_names", "o_a_n", "old_aig_names", "OLD_AIG_NAMEs" )

  /* Implements the short string to describe a store element in store -a */
  ALICE_DESCRIBE_STORE( mockturtle::NameMap<mockturtle::aig_network>, names ){

    return fmt::format( "AIG name store");
  }//end aig_network describe store

  ALICE_LOG_STORE_STATISTICS( mockturtle::NameMap<mockturtle::aig_network>, names){

    return {
      {"store"}};
  }//end aig_network log store statistics

  /* Implements the functionality of ps -b */
  ALICE_PRINT_STORE_STATISTICS( mockturtle::NameMap<mockturtle::aig_network>, os, names ){
    os << "store" << std::endl;

  }//end aig_network print store statistics

  ALICE_ADD_STORE( mockturtle::NameMap<mockturtle::mig_network>, "old_mig_names", "o_m_n", "old_mig_names", "OLD_MIG_NAMEs" )

  /* Implements the short string to describe a store element in store -a */
  ALICE_DESCRIBE_STORE( mockturtle::NameMap<mockturtle::mig_network>, names ){

    return fmt::format( "MIG name store");
  }//end aig_network describe store

  ALICE_LOG_STORE_STATISTICS( mockturtle::NameMap<mockturtle::mig_network>, names){

    return {
      {"store"}};
  }//end aig_network log store statistics

  /* Implements the functionality of ps -b */
  ALICE_PRINT_STORE_STATISTICS( mockturtle::NameMap<mockturtle::mig_network>, os, names ){
    os << "store" << std::endl;

  }//end aig_network print store statistics

  /* Adds XOR-AND graphs (Mockturtle type xag_network) as store element type to
   * alice.
   *
   * One can access XAGs in general store commands using the long --xag flag or
   * the short -x flag.
   */
  ALICE_ADD_STORE( mockturtle::xag_network, "old_xag", "o_x", "old_xag", "OLD_XAGs" )

  /* Implements the short string to describe a store element in store -a */
  ALICE_DESCRIBE_STORE( mockturtle::xag_network, xag ){

    const auto name = "xag_placeholder";
    const auto pi_num = xag.num_pis();
    const auto po_num = xag.num_pos();
    return fmt::format( "{} i/o = {}/{}", name, pi_num, po_num );
  }//end xag_network describe store

  ALICE_LOG_STORE_STATISTICS( mockturtle::xag_network, xag){

    return {
      {"nodes", xag.size()},
      {"inputs", xag.num_pis()},
      {"outputs", xag.num_pos()},
      {"AIG nodes", xag.num_gates()}};
  }//end xag_network log store statistics

  /* Implements the functionality of ps -b */
  ALICE_PRINT_STORE_STATISTICS( mockturtle::xag_network, os, xag ){
    os << "nodes: " << xag.size() << std::endl;
    os << "inputs: " << xag.num_pis() << std::endl;
    os << "outputs: " << xag.num_pos() << std::endl;
    os << "AIG nodes: " << xag.num_gates() << std::endl;

  }//end xag_network print store statistics

  ALICE_ADD_STORE( mockturtle::mig_network, "old_mig", "o_m", "old_mig", "OLD_MIGs" )

  /* Implements the short string to describe a store element in store -a */
  ALICE_DESCRIBE_STORE( mockturtle::mig_network, mig ){

    const auto name = "mig_placeholder";
    const auto pi_num = mig.num_pis();
    const auto po_num = mig.num_pos();

    return fmt::format( "{} i/o = {}/{}", name, pi_num, po_num );
  }//end aig_network describe store

  ALICE_LOG_STORE_STATISTICS( mockturtle::mig_network, mig){
    mockturtle::depth_view depth{mig};
    return {
        {"nodes", mig.size()},
        {"inputs", mig.num_pis() - mig.num_latches()},
        {"latches", mig.num_latches()},
        {"outputs", mig.num_pos() - mig.num_latches()},
        {"MAJ nodes", mig.num_gates()},
        {"MIG level", depth.depth()}};
  }//end aig_network log store statistics

  /* Implements the functionality of ps -b */
  ALICE_PRINT_STORE_STATISTICS( mockturtle::mig_network, os, mig ){
    mockturtle::depth_view depth{mig};
    os << "nodes: " << mig.size() << std::endl;
    os << "inputs: " << mig.num_pis() - mig.num_latches() << std::endl;
    os << "latches: " << mig.num_latches() << std::endl;
    os << "outputs: " << mig.num_pos() - mig.num_latches()<< std::endl;
    os << "MAJ nodes: " << mig.num_gates() << std::endl;
    os << "MIG level: " << depth.depth() << std::endl;
  }//end aig_network print store statistics

  ALICE_ADD_STORE( oracle::partition_manager<mockturtle::mig_network>, "old_part_man_mig", "o_pm_m", "old_part_man_mig", "OLD_PART_MAN_MIGs")

  /* Implements the short string to describe a store element in store -a */
  ALICE_DESCRIBE_STORE( oracle::partition_manager<mockturtle::mig_network>, part_man ){

    const auto name = "partition manager for MIG networks";
    const auto part_num = part_man.get_part_num();

    return fmt::format( "{} # partitions = {}", name, part_num );
  }//end partition manager<mig_network> describe store

  ALICE_LOG_STORE_STATISTICS( oracle::partition_manager<mockturtle::mig_network>, part_man){

    return {
            {"partition number", part_man.get_part_num()}};
  }//end partition manager<mig_network> log store statistics

  /* Implements the functionality of ps -b */
  ALICE_PRINT_STORE_STATISTICS( oracle::partition_manager<mockturtle::mig_network>, os, part_man ){
    os << "partition number: " << part_man.get_part_num() << std::endl;
  }//end partition manager<mig_network> print store statistics

  ALICE_ADD_STORE( oracle::partition_manager<mockturtle::aig_network>, "old_part_man_aig", "o_pm_a", "old_part_man_aig", "OLD_PART_MAN_AIGs")

  /* Implements the short string to describe a store element in store -a */
  ALICE_DESCRIBE_STORE( oracle::partition_manager<mockturtle::aig_network>, part_man ){

    const auto name = "partition manager for AIG networks";
    const auto part_num = part_man.get_part_num();

    return fmt::format( "{} # partitions = {}", name, part_num );
  }//end partition manager<aig_network> describe store

  ALICE_LOG_STORE_STATISTICS( oracle::partition_manager<mockturtle::aig_network>, part_man){

    return {
            {"partition number", part_man.get_part_num()}};
  }//end partition manager<aig_network> log store statistics

  /* Implements the functionality of ps -b */
  ALICE_PRINT_STORE_STATISTICS( oracle::partition_manager<mockturtle::aig_network>, os, part_man ){
    os << "partition number: " << part_man.get_part_num() << std::endl;
  }//end partition manager<aig_network> print store statistics

  ALICE_ADD_STORE( aig_ntk, "aig", "a", "aig", "AIGs" )

  ALICE_DESCRIBE_STORE( aig_ntk, aig ){

    return fmt::format( "i/o = {}/{} gates = {}", aig->num_pis(), aig->num_pos(), aig->num_gates() );
  }

  ALICE_LOG_STORE_STATISTICS( aig_ntk, aig){
    mockturtle::depth_view depth{*aig};
    return {
    	{"nodes", aig->size()},
    	{"inputs", aig->num_pis() - aig->num_latches()},
    	{"latches", aig->num_latches()},
    	{"outputs", aig->num_pos() - aig->num_latches()},
    	{"AIG nodes", aig->num_gates()},
      {"AIG level", depth.depth()}};
  }

  /*Reads an aig file and stores the AIG network in a store*/
  class old_read_aig_command : public alice::command{

    public:
      explicit old_read_aig_command( const environment::ptr& env )
          : command( env, "Uses the lorina library to read in an aig file" ){

        opts.add_option( "--filename,filename", filename, "AIG file to read in" )->required();
        add_flag("--mig,-m", "Store AIG file as MIG network (AIG network is default)");
        add_flag("--xag,-x", "Store AIG file as XAG network (AIG network is default)");
      }

    protected:
      void execute(){

        if(checkExt(filename, "aig")){
          if(is_set("mig")){
            mockturtle::mig_network ntk;
            mockturtle::NameMap<mockturtle::mig_network> names;
            lorina::read_aiger(filename, mockturtle::aiger_reader( ntk, &names ));
                
            store<mockturtle::mig_network>().extend() = ntk;

            store<mockturtle::NameMap<mockturtle::mig_network>>().extend() = names;

            // ntk.foreach_pi([&](auto pi){
            //   std::vector<std::string> pi_names = names.get_name(ntk.make_signal(pi));
            //   for(std::string curr_name : pi_names){
            //     std::cout << "PI " << pi << " name: " << curr_name << "\n";
            //   }
            // });
            // ntk.foreach_po([&](auto po){
            //   std::vector<std::string> po_names = names.get_name(po);
            //   for(std::string curr_name : po_names){
            //     std::cout << "PO " << po.index << " name: " << curr_name << "\n";
            //   }
            // });


            // if(ntk._storage->inputNames.size() == 0){
            //   for(int i = 0; i < ntk.num_pis(); i++){
            //     std::string input_name = "pi";
            //     input_name.append(std::to_string(i));
            //     ntk._storage->inputNames[i] = input_name;
            //   }
            // }
            // if(ntk._storage->outputNames.size() == 0){

            //   for(int i = 0; i < ntk.num_pos(); i++){
            //     std::string output_name = "po";
            //     output_name.append(std::to_string(i));
            //     ntk._storage->outputNames[i] = output_name;
            //   }
            // }

            filename.erase(filename.end() - 4, filename.end());
            ntk._storage->net_name = filename;
          }
          else if(is_set("xag")){
            mockturtle::xag_network ntk;
            lorina::read_aiger(filename, mockturtle::aiger_reader( ntk ));
                
            store<mockturtle::xag_network>().extend() = ntk;


            if(ntk._storage->inputNames.size() == 0){
              for(int i = 0; i < ntk.num_pis(); i++){
                std::string input_name = "pi";
                input_name.append(std::to_string(i));
                ntk._storage->inputNames[i] = input_name;
              }
            }
            if(ntk._storage->outputNames.size() == 0){

              for(int i = 0; i < ntk.num_pos(); i++){
                std::string output_name = "po";
                output_name.append(std::to_string(i));
                ntk._storage->outputNames[i] = output_name;
              }
            }
            ntk.foreach_node([&](auto node){
              std::cout << "Node = " << node << "\n";
              if(ntk.is_xor(node)){
                std::cout << "XOR\n";
              }
            });
            filename.erase(filename.end() - 4, filename.end());
            ntk._storage->net_name = filename;
          }
          else{
            mockturtle::aig_network ntk;
            mockturtle::NameMap<mockturtle::aig_network> names;
            lorina::read_aiger(filename, mockturtle::aiger_reader( ntk, &names ));
                
            store<mockturtle::aig_network>().extend() = ntk;

            store<mockturtle::NameMap<mockturtle::aig_network>>().extend() = names;

            // ntk.foreach_pi([&](auto pi){
            //   std::vector<std::string> pi_names = names.get_name(ntk.make_signal(pi));
            //   for(std::string curr_name : pi_names){
            //     std::cout << "PI " << pi << " name: " << curr_name << "\n";
            //   }
            // });
            // ntk.foreach_po([&](auto po){
            //   std::vector<std::string> po_names = names.get_name(po);
            //   for(std::string curr_name : po_names){
            //     std::cout << "PO " << po.index << " name: " << curr_name << "\n";
            //   }
            // });
            // if(ntk._storage->inputNames.size() == 0){

            //   for(int i = 0; i < ntk.num_pis(); i++){
            //     std::string input_name = "pi";
            //     input_name.append(std::to_string(i));
            //     ntk._storage->inputNames[i] = input_name;
            //   }
            // }
            // if(ntk._storage->outputNames.size() == 0){

            //   for(int i = 0; i < ntk.num_pos(); i++){
            //     std::string output_name = "po";
            //     output_name.append(std::to_string(i));
            //     ntk._storage->outputNames[i] = output_name;
            //   }
            // }

            filename.erase(filename.end() - 4, filename.end());
            ntk._storage->net_name = filename;
          }

        }
        else{
            std::cout << filename << " is not a valid aig file\n";
        }
        
      }
    private:
      std::string filename{};
    };

  ALICE_ADD_COMMAND(old_read_aig, "Input");

  class old_partitioning_command : public alice::command{

    public:
      explicit old_partitioning_command( const environment::ptr& env )
        : command( env, "Partitionins current network using k-means hypergraph partitioner" ) {

          opts.add_option( "--num,num", num_partitions, "Number of desired partitions" )->required();
          opts.add_option( "--config_direc,-c", config_direc, "Path to the configuration file for KaHyPar (../../core/test.ini is default)" );
          add_flag("--mig,-m", "Partitions stored MIG network (AIG network is default)");
        }

    protected:
      void execute(){
        mockturtle::mig_npn_resynthesis resyn_mig;
        mockturtle::xag_npn_resynthesis<mockturtle::aig_network> resyn_aig;
        if(is_set("mig")){
          if(!store<mockturtle::mig_network>().empty()){
            std::cout << "Partitioning stored MIG network\n";
            auto ntk = store<mockturtle::mig_network>().current();

            if(config_direc != ""){
              oracle::partition_manager<mockturtle::mig_network> partitions(ntk, num_partitions, config_direc);
              store<oracle::partition_manager<mockturtle::mig_network>>().extend() = partitions;
            }
            else{
              oracle::partition_manager<mockturtle::mig_network> partitions(ntk, num_partitions);
              store<oracle::partition_manager<mockturtle::mig_network>>().extend() = partitions;
            }           
          }
          else{
            std::cout << "MIG network not stored\n";
          }
        }
        else{
          if(!store<mockturtle::aig_network>().empty()){
            std::cout << "Partitioning stored AIG network\n";
            auto ntk = store<mockturtle::aig_network>().current();

            if(config_direc != ""){
              oracle::partition_manager<mockturtle::aig_network> partitions(ntk, num_partitions, config_direc);
              store<oracle::partition_manager<mockturtle::aig_network>>().extend() = partitions;
            }
            else{
              oracle::partition_manager<mockturtle::aig_network> partitions(ntk, num_partitions);
              store<oracle::partition_manager<mockturtle::aig_network>>().extend() = partitions;
            }
          }
          else{
            std::cout << "AIG network not stored\n";
          }
        }
      }
    private:
      int num_partitions{};
      std::string config_direc = "";
  };

  ALICE_ADD_COMMAND(old_partitioning, "Partitioning");

  mockturtle::mig_network old_aig_to_mig(mockturtle::aig_network aig, int skip_edge_min){
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

      if(skip_edge_min == 1){
        node2new[n] = mig.create_maj_part(mig.get_constant( false ), children.at(0), children.at(1));
      }
      else{
        node2new[n] = mig.create_maj(mig.get_constant( false ), children.at(0), children.at(1));
      }
      
          
    } );

    /* map primary outputs */
    aig.foreach_po( [&]( auto const& f ) {
      mig.create_po( aig.is_complemented( f ) ? mig.create_not( node2new[f] ) : node2new[f] );
    } );

    return mig;
  }

  mockturtle::mig_network old_part_to_mig(oracle::partition_view<mockturtle::mig_network> part, int skip_edge_min){
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

    return mig;
  }

  mockturtle::aig_network old_mig_to_aig(mockturtle::mig_network mig){
    mockturtle::aig_network aig;

    mockturtle::node_map<mockturtle::aig_network::signal, mockturtle::mig_network> node2new( mig );

    node2new[mig.get_node( mig.get_constant( false ) )] = aig.get_constant( false );
    if ( mig.get_node( mig.get_constant( true ) ) != mig.get_node( mig.get_constant( false ) ) ){

      node2new[mig.get_node( mig.get_constant( true ) )] = aig.get_constant( true );
    }

    mig.foreach_pi( [&]( auto n ) {
      node2new[n] = aig.create_pi();
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
          
    } );

    /* map primary outputs */
    mig.foreach_po( [&]( auto const& f ) {
      aig.create_po( mig.is_complemented( f ) ? aig.create_not( node2new[f] ) : node2new[f] );
    } );

    return aig;
  }

  class old_optimization_command : public alice::command{

    public:
        explicit old_optimization_command( const environment::ptr& env )
                : command( env, "Brute force approach to finding best optimization methods" ){

                opts.add_option( "--nn_model,-n", nn_model, "Trained neural network model for classification" );
                opts.add_option( "--out,-o", out_file, "Verilog output" );
                add_flag("--high,-b", "Uses a high effort approach instead of classification");
                add_flag("--aig,-a", "Perform only AIG optimization on all partitions");
                add_flag("--mig,-m", "Perform only MIG optimization on all partitions");
                add_flag("--combine,-c", "Combine adjacent partitions that have been classified for the same optimization");
        }

    protected:
      void execute(){

        mockturtle::direct_resynthesis<mockturtle::mig_network> resyn_mig;
        mockturtle::direct_resynthesis<mockturtle::aig_network> resyn_aig;
        std::vector<int> aig_parts;
        std::vector<int> mig_parts;
        std::vector<int> comb_aig_parts;
        std::vector<int> comb_mig_parts;
        if(!store<mockturtle::aig_network>().empty()){

          auto ntk_aig = store<mockturtle::aig_network>().current();

          auto ntk_aig_names = store<mockturtle::NameMap<mockturtle::aig_network>>().current();
          std::vector<std::string> input_names;
          std::vector<std::string> output_names;
          ntk_aig.foreach_pi([&](auto pi){
            std::vector<std::string> pi_names = ntk_aig_names.get_name(ntk_aig.make_signal(pi));
            for(std::string curr_name : pi_names){
              input_names.push_back(curr_name);
            }
          });
          ntk_aig.foreach_po([&](auto po, auto i){
            std::vector<std::string> po_names = ntk_aig_names.get_name(po);
            for(std::string curr_name : po_names){
              output_names.push_back(curr_name);
            }
          });

          std::string file_base = ntk_aig._storage->net_name;
          // std::cout << "ntk_aig size = " << ntk_aig.size() << "\n";
          std::string net_name = ntk_aig._storage->net_name;

          if(!store<oracle::partition_manager<mockturtle::aig_network>>().empty()){
            
            auto start = std::chrono::high_resolution_clock::now();

            auto partitions_aig = store<oracle::partition_manager<mockturtle::aig_network>>().current();
            int num_parts = partitions_aig.get_part_num();
            if(is_set("aig")){
              for(int i = 0; i < num_parts; i++){
                aig_parts.push_back(i);
              }
            }
            else if(is_set("mig")){
              for(int i = 0; i < num_parts; i++){
                mig_parts.push_back(i);
              }
            }
            else if(is_set("high")){

              for(int i = 0; i < num_parts; i++){
                oracle::partition_view<mockturtle::aig_network> part_aig = partitions_aig.create_part(ntk_aig, i);

                auto opt_aig = mockturtle::node_resynthesis<mockturtle::aig_network>( part_aig, resyn_aig );
                mockturtle::depth_view part_aig_depth{opt_aig};
                mockturtle::aig_script aigopt;
                opt_aig = aigopt.run(opt_aig);
                mockturtle::depth_view part_aig_opt_depth{opt_aig};
                int aig_opt_size = opt_aig.num_gates();
                int aig_opt_depth = part_aig_opt_depth.depth();

                auto opt_mig = mockturtle::node_resynthesis<mockturtle::mig_network>( part_aig, resyn_mig );
                mockturtle::depth_view part_mig_depth{opt_mig};
                mockturtle::mig_script migopt;
                opt_mig = migopt.run(opt_mig);
                mockturtle::depth_view part_mig_opt_depth{opt_mig};
                int mig_opt_size = opt_mig.num_gates();
                int mig_opt_depth = part_mig_opt_depth.depth();

                if((aig_opt_size * aig_opt_depth) <= (mig_opt_size * mig_opt_depth)){
                  aig_parts.push_back(i);
                }
                else{
                  mig_parts.push_back(i);
                }
              }

            }
            else{
              if(!nn_model.empty()){
                partitions_aig.run_classification(ntk_aig, nn_model);

                aig_parts = partitions_aig.get_aig_parts();
                mig_parts = partitions_aig.get_mig_parts();
              }
              else{
                std::cout << "Must include CNN model json file\n";
              }

            }

            std::cout << "Scheduled optimization\n";
            std::cout << aig_parts.size() << " AIGs and " << mig_parts.size() << " MIGs\n";

            if(is_set("combine")){
              std::vector<int> visited;
              std::unordered_map<int, int> comb_part;
              for(int i = 0; i < num_parts; i++){
                if(std::find(visited.begin(), visited.end(), i) == visited.end()){
                  std::vector<int> parts_to_combine;
                  
                  std::set<int>::iterator conn_it;
                  std::set<int> conn_parts;
                  conn_parts = partitions_aig.get_connected_parts(ntk_aig, i);
                  if(std::find(aig_parts.begin(), aig_parts.end(), i) != aig_parts.end()){
                    for(conn_it = conn_parts.begin(); conn_it != conn_parts.end(); ++conn_it){
                      if(std::find(aig_parts.begin(), aig_parts.end(), *conn_it) != aig_parts.end()){
                        parts_to_combine.push_back(*conn_it);
                      }
                    }
                  }
                  else{
                    for(conn_it = conn_parts.begin(); conn_it != conn_parts.end(); ++conn_it){
                      if(std::find(mig_parts.begin(), mig_parts.end(), *conn_it) != mig_parts.end()){
                        parts_to_combine.push_back(*conn_it);
                      }
                    }
                  }

                  if(parts_to_combine.size() == 0){
                    if(std::find(aig_parts.begin(), aig_parts.end(), i) != aig_parts.end()){
                      if(std::find(comb_aig_parts.begin(), comb_aig_parts.end(), i) == comb_aig_parts.end()){
                        comb_aig_parts.push_back(i);
                      }
                    }
                    else{
                      if(std::find(comb_mig_parts.begin(), comb_mig_parts.end(), i) == comb_mig_parts.end()){
                        comb_mig_parts.push_back(i);
                      }
                    }
                  }

                  for(int idx = 0; idx < parts_to_combine.size(); idx++){
                    int curr_part = parts_to_combine.at(idx);
                    int part_1 = 0;
                    int part_2 = 0;
                    if(std::find(visited.begin(), visited.end(), curr_part) == visited.end()){
                      part_1 = i;
                      part_2 = curr_part;
                    }
                    else{
                      if(std::find(visited.begin(), visited.end(), i) == visited.end()){
                        part_1 = curr_part;
                        part_2 = i;
                      }
                    }

                    if(std::find(visited.begin(), visited.end(), part_2) == visited.end()){
                      std::unordered_map<int,int>::const_iterator got = comb_part.find (part_1);
                      if(got != comb_part.end()){
                        part_1 = got->second;
                      }
                      std::set<int> connected_parts1 = partitions_aig.get_connected_parts(ntk_aig, part_1);
                      std::set<int> connected_parts2 = partitions_aig.get_connected_parts(ntk_aig, part_2);
                      std::set<int>::iterator conn_it;
                      
                      std::vector<std::set<mockturtle::aig_network::node>> combined_io = partitions_aig.combine_partitions(ntk_aig, part_1, part_2);
                      
                      auto new_inputs = combined_io.at(0);
                      auto new_outputs = combined_io.at(1);
                      comb_part[part_2] = part_1;

                      partitions_aig.set_part_inputs(part_1, new_inputs);
                      partitions_aig.set_part_outputs(part_1, new_outputs);

                      if(std::find(aig_parts.begin(), aig_parts.end(), part_1) != aig_parts.end()){
                        if(std::find(comb_aig_parts.begin(), comb_aig_parts.end(), part_1) == comb_aig_parts.end()){
                          comb_aig_parts.push_back(part_1);
                        }
                      }
                      else{
                        if(std::find(comb_mig_parts.begin(), comb_mig_parts.end(), part_1) == comb_mig_parts.end()){
                          comb_mig_parts.push_back(part_1);
                        }
                      }

                      visited.push_back(part_2); 

                      connected_parts1 = partitions_aig.get_connected_parts(ntk_aig, part_1);
                      for(conn_it = connected_parts1.begin(); conn_it != connected_parts1.end(); ++conn_it){
                        if(std::find(aig_parts.begin(), aig_parts.end(), i) != aig_parts.end()){
                          if(std::find(parts_to_combine.begin(), parts_to_combine.end(), *conn_it) == parts_to_combine.end() && 
                            std::find(aig_parts.begin(), aig_parts.end(), *conn_it) != aig_parts.end() &&
                            std::find(visited.begin(), visited.end(), *conn_it) == visited.end()){

                            parts_to_combine.push_back(*conn_it);
                          }
                        }
                        else{
                          if(std::find(parts_to_combine.begin(), parts_to_combine.end(), *conn_it) == parts_to_combine.end() && 
                            std::find(mig_parts.begin(), mig_parts.end(), *conn_it) != mig_parts.end() &&
                            std::find(visited.begin(), visited.end(), *conn_it) == visited.end()){

                            parts_to_combine.push_back(*conn_it);
                          }
                        }
                        
                      } 
                    }
                    visited.push_back(i);

                  }
                }
              }
              aig_parts = comb_aig_parts;
              mig_parts = comb_mig_parts;
              std::cout << "Scheduled optimization after partition merging\n";
              std::cout << aig_parts.size() << " AIGs and " << mig_parts.size() << " MIGs\n";
            }
            
            mockturtle::mig_network ntk_mig = old_aig_to_mig(ntk_aig, 1);
            oracle::partition_manager<mockturtle::mig_network> partitions_mig(ntk_mig, partitions_aig.get_all_part_connections(), 
                    partitions_aig.get_all_partition_inputs(), partitions_aig.get_all_partition_outputs(), partitions_aig.get_part_num());

            // std::cout << "AIG Optimization\n";
            for(int i = 0; i < aig_parts.size(); i++){
              
              oracle::partition_view<mockturtle::mig_network> part = partitions_mig.create_part(ntk_mig, aig_parts.at(i));
              mockturtle::depth_view part_depth{part};

              auto opt_part = old_part_to_mig(part, 1);
              auto opt = old_mig_to_aig(opt_part);

              mockturtle::depth_view opt_part_depth{opt};

              mockturtle::aig_script aigopt;
              opt = aigopt.run(opt);

              auto opt_mig = old_aig_to_mig(opt, 0);
              mockturtle::depth_view part_opt_depth{opt_mig};

              partitions_mig.synchronize_part(part, opt_mig, ntk_mig);
            }
            // std::cout << "MIG Optimization\n";
            for(int i = 0; i < mig_parts.size(); i++){
              
              oracle::partition_view<mockturtle::mig_network> part = partitions_mig.create_part(ntk_mig, mig_parts.at(i));
              mockturtle::depth_view part_depth{part};

              auto opt = old_part_to_mig(part, 0);

              mockturtle::depth_view opt_part_depth{opt};
              
              mockturtle::mig_script migopt;
              opt = migopt.run(opt);
              
              mockturtle::depth_view part_opt_depth{opt};

              partitions_mig.synchronize_part(part, opt, ntk_mig);
            }
            
            partitions_mig.connect_outputs(ntk_mig);
            
            mockturtle::depth_view ntk_before_depth2{ntk_mig};
            
            ntk_mig = mockturtle::cleanup_dangling( ntk_mig );
            mockturtle::depth_view ntk_depth2{ntk_mig};
            std::cout << "Final ntk size = " << ntk_mig.num_gates() << " and depth = " << ntk_depth2.depth() << "\n";
            std::cout << "Area Delay Product = " << ntk_mig.num_gates() * ntk_depth2.depth() << "\n";
            auto stop = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
            std::cout << "Full Optimization: " << duration.count() << "ms\n";
            std::cout << "Finished optimization\n";
            store<mockturtle::mig_network>().extend() = ntk_mig;

            mockturtle::NameMap<mockturtle::mig_network> names;
            ntk_mig.foreach_pi([&](auto pi, auto i){
              names.insert(ntk_mig.make_signal(pi), input_names.at(i));
            });
            ntk_mig.foreach_po([&](auto po, auto i){
              names.insert(po, output_names.at(i));
            });

            store<mockturtle::NameMap<mockturtle::mig_network>>().extend() = names;

            if(out_file != ""){
              mockturtle::write_verilog(ntk_mig, out_file);
              std::cout << "Resulting Verilog written to " << out_file << "\n";
            }
        
          }
          else{
              std::cout << "AIG not partitioned yet\n";
          }
        }
        else{
          std::cout << "No AIG stored\n";
        }
      }
    private:
        std::string nn_model{};
        std::string out_file{};
    };

  ALICE_ADD_COMMAND(old_optimization, "Optimization");

  class old_write_verilog_command : public alice::command{

    public:
      explicit old_write_verilog_command( const environment::ptr& env )
          : command( env, "Writes the Boolean network into structural verilog" ){

        opts.add_option( "--filename,filename", filename, "Verilog file to write out to" )->required();
        add_flag("--mig,-m", "Read from the MIG network");
        add_flag("--names,-n", "Preserve I/O names");
      }

    protected:
        void execute(){
        if(checkExt(filename, "v")){
          if(is_set("mig")){
            if(!store<mockturtle::mig_network>().empty()){
              auto& mig = store<mockturtle::mig_network>().current();
              auto& mig_names = store<mockturtle::NameMap<mockturtle::mig_network>>().current();
              mockturtle::write_verilog_params vp;
              std::vector<std::pair<std::string, uint32_t>> input_names;
              std::vector<std::pair<std::string, uint32_t>> output_names;
              if(is_set("names")){
                mig.foreach_pi([&](auto pi){
                  std::vector pi_names = mig_names.get_name(mig.make_signal(pi));
                  for(std::string curr_name : pi_names){
                    // std::cout << "PI " << pi << " name: " << curr_name << "\n";
                    input_names.push_back(std::make_pair(curr_name, 1));
                  }
                });
                mig.foreach_po([&](auto po, auto i){
                  std::vector po_names = mig_names.get_name(po);
                  for(std::string curr_name : po_names){
                    // std::cout << "PO " << po.index << " name: " << curr_name << "\n";
                    output_names.push_back(std::make_pair(curr_name, 1));
                  }
                });
                vp.input_names = input_names;
                vp.output_names = output_names;
              }
              mockturtle::write_verilog(mig, filename, vp);
            }
            else{
              std::cout << "There is not an MIG network stored.\n";
            }
          }
          else{
            if(!store<mockturtle::aig_network>().empty()){
              auto& aig = store<mockturtle::aig_network>().current();
              auto& aig_names = store<mockturtle::NameMap<mockturtle::aig_network>>().current();
              mockturtle::write_verilog_params vp;
              std::vector<std::pair<std::string, uint32_t>> input_names;
              std::vector<std::pair<std::string, uint32_t>> output_names;
              if(is_set("names")){
                aig.foreach_pi([&](auto pi){
                  std::vector pi_names = aig_names.get_name(aig.make_signal(pi));
                  for(std::string curr_name : pi_names){
                    // std::cout << "PI " << pi << " name: " << curr_name << "\n";
                    input_names.push_back(std::make_pair(curr_name, 1));
                  }
                });
                aig.foreach_po([&](auto po, auto i){
                  std::vector po_names = aig_names.get_name(po);
                  for(std::string curr_name : po_names){
                    // std::cout << "PO " << po.index << " name: " << curr_name << "\n";
                    output_names.push_back(std::make_pair(curr_name, 1));
                  }
                });
                vp.input_names = input_names;
                vp.output_names = output_names;
              }
              mockturtle::write_verilog(aig, filename, vp);
            }
            else{
              std::cout << "There is not an AIG network stored.\n";
            }
          }
        }
        else{
            std::cout << filename << " is not a valid verilog file\n";
        }
      }
    private:
      std::string filename{};
  };

  ALICE_ADD_COMMAND(old_write_verilog, "Output");

  ALICE_PRINT_STORE_STATISTICS( aig_ntk, os, aig ){
    mockturtle::depth_view depth{*aig};
  	os << "nodes: " << aig->size() << std::endl;
  	os << "inputs: " << aig->num_pis() - aig->num_latches() << std::endl;
  	os << "latches: " << aig->num_latches() << std::endl;
  	os << "outputs: " << aig->num_pos() - aig->num_latches() << std::endl;
  	os << "AIG nodes: " << aig->num_gates() << std::endl;
    os << "AIG level: " << depth.depth() << std::endl;

  }

  ALICE_ADD_STORE( xag_ntk, "xag", "x", "xag", "XAGs" )

  ALICE_DESCRIBE_STORE( xag_ntk, xag ){

    return fmt::format( "i/o = {}/{} gates = {}", xag->num_pis(), xag->num_pos(), xag->num_gates() );
  }

  ALICE_LOG_STORE_STATISTICS( xag_ntk, xag){
    mockturtle::depth_view depth{*xag};
    return {
      {"nodes", xag->size()},
      {"inputs", xag->num_pis()},
      {"outputs", xag->num_pos()},
      {"XAG nodes", xag->num_gates()},
      {"XAG level", depth.depth()}};
  }

  ALICE_PRINT_STORE_STATISTICS( xag_ntk, os, xag ){
    mockturtle::depth_view depth{*xag};
    os << "nodes: " << xag->size() << std::endl;
    os << "inputs: " << xag->num_pis() << std::endl;
    os << "outputs: " << xag->num_pos() << std::endl;
    os << "XAG nodes: " << xag->num_gates() << std::endl;
    os << "XAG level: " << depth.depth() << std::endl;

  }

  ALICE_ADD_STORE( mig_ntk, "mig", "m", "mig", "MIGs" )

  ALICE_DESCRIBE_STORE( mig_ntk, mig ){

    return fmt::format( "i/o = {}/{} gates = {}", mig->num_pis(), mig->num_pos(), mig->num_gates() );
  }

  ALICE_LOG_STORE_STATISTICS( mig_ntk, mig){
    mockturtle::depth_view depth{*mig};
    return {
      {"nodes", mig->size()},
      {"inputs", mig->num_pis() - mig->num_latches()},
      {"latches", mig->num_latches()},
      {"outputs", mig->num_pos() - mig->num_latches()},
      {"MAJ nodes", mig->num_gates()},
      {"MIG level", depth.depth()}};
  }

  ALICE_PRINT_STORE_STATISTICS( mig_ntk, os, mig ){
    mockturtle::depth_view depth{*mig};
    os << "nodes: " << mig->size() << std::endl;
    os << "inputs: " << mig->num_pis() - mig->num_latches() << std::endl;
    os << "latches: " << mig->num_latches() << std::endl;
    os << "outputs: " << mig->num_pos() - mig->num_latches() << std::endl;
    os << "MAJ nodes: " << mig->num_gates() << std::endl;
    os << "MIG level: " << depth.depth() << std::endl;

  }

  ALICE_ADD_STORE( part_man_mig_ntk,"part_man_mig", "pm_m", "part_man_mig", "PART_MAN_MIGs")

  /* Implements the short string to describe a store element in store -a */
  ALICE_DESCRIBE_STORE( part_man_mig_ntk, part_man ){

    const auto name = "partition manager for Named MIG networks";
    const auto part_num = part_man->get_part_num();

    return fmt::format( "{} # partitions = {}", name, part_num );
  }//end partition manager<mig_network> describe store

  ALICE_LOG_STORE_STATISTICS( part_man_mig_ntk, part_man){

    return {
            {"partition number", part_man->get_part_num()}};
  }//end partition manager<mig_network> log store statistics

  /* Implements the functionality of ps -b */
  ALICE_PRINT_STORE_STATISTICS( part_man_mig_ntk, os, part_man ){
    os << "partition number: " << part_man->get_part_num() << std::endl;
  }//end partition manager<mig_network> print store statistics

  ALICE_ADD_STORE( part_man_aig_ntk,"part_man_aig", "pm_a", "part_man_aig", "PART_MAN_AIGs")

  /* Implements the short string to describe a store element in store -a */
  ALICE_DESCRIBE_STORE( part_man_aig_ntk, part_man ){

    const auto name = "partition manager for Named MIG networks";
    const auto part_num = part_man->get_part_num();

    return fmt::format( "{} # partitions = {}", name, part_num );
  }//end partition manager<mig_network> describe store

  ALICE_LOG_STORE_STATISTICS( part_man_aig_ntk, part_man){

    return {
            {"partition number", part_man->get_part_num()}};
  }//end partition manager<mig_network> log store statistics

  /* Implements the functionality of ps -b */
  ALICE_PRINT_STORE_STATISTICS( part_man_aig_ntk, os, part_man ){
    os << "partition number: " << part_man->get_part_num() << std::endl;
  }//end partition manager<mig_network> print store statistics

  // class write_aig_command : public alice::command{

  //   public:
  //     explicit write_aig_command( const environment::ptr& env )
  //         : command( env, "Writes the AIG in the ASCII format" ){

  //       opts.add_option( "--filename,filename", filename, "AAG file to write stored AIG to" )->required();
  //     }

  //   protected:
  //     void execute(){
  //       if(checkExt(filename, "aag")){
  //         if(!store<aig_ntk>().empty()){
  //           auto aig = *store<aig_ntk>().current();

  //           std::ofstream aigfile;
  //           aigfile.open (filename);

  //           // header info - MILOA
  //           auto _num_inputs = aig.num_pis();
  //           auto _num_latches = aig.num_latches();
  //           auto _num_outputs = aig.num_pos();
  //           auto _num_ands = aig.num_gates();
  //           auto _num_vertices = aig.num_pis() + aig.num_gates();

  //           //write aig header
  //           aigfile << "aag " << _num_vertices << " " << _num_inputs - _num_latches << " " << _num_latches << " " << (_num_outputs - _num_latches) << " " << _num_ands << std::endl;

  //           aig.foreach_pi([&] (auto node){
  //             auto index = aig.pi_index(node);
  //             aigfile << (index+1)*2 << "\n";
  //           });

  //           //write aig latches
  //           auto lineIdx = ((aig.num_pis()-aig.num_latches())*2)+2;
  //           for(int i=0; i<aig.num_latches(); i++){
  //             auto regIdx = aig.num_pos() - aig.num_latches() + i;
  //             aigfile << lineIdx << " " << aig._storage->outputs[regIdx].data << "\n";
  //             lineIdx += 2;
  //           }

  //           //write aig outputs
  //           for(int i=0; i<aig.num_pos()- aig.num_latches(); i++){
  //             aigfile << aig._storage->outputs[i].data << "\n";
  //           }

  //           auto skipLatches = 0;
  //           aig.foreach_gate([&] (auto node){
  //             //skip latches in the nodes vector
  //             if(skipLatches>=aig.num_latches()) {
  //               auto index = aig.node_to_index(node);
  //               auto left = aig._storage->nodes[index].children[0].data;
  //               auto right = aig._storage->nodes[index].children[1].data;
  //               aigfile << index*2 << " " << left << " " << right << "\n";
  //             }
  //             skipLatches+=1;
  //           });

  //           for(int i = 0; i < aig._storage->inputs.size(); i++){
  //             aigfile << "i" << i << " " << aig._storage->inputNames[aig._storage->inputs.at(i) - 1] << "\n";
  //           }
  //           for(int i = 0; i < aig._storage->outputs.size(); i++){
  //             aigfile << "o" << i << " " << aig._storage->outputNames[i] << "\n";
  //           }

  //           aigfile.close();
  //         }
  //         else{
  //             std::cout << "No AIG stored\n";
  //         }
  //       }
  //       else{
  //         std::cout << "File not a vlid aag file\n";
  //       }
  //     }
  //   private:
  //     std::string filename{};
  //   };

  // ALICE_ADD_COMMAND(write_aig, "Output");

  class read_verilog_command : public alice::command{

    public:
      explicit read_verilog_command( const environment::ptr& env )
          : command( env, "Uses the lorina library to read in an Verilog file" ){

        opts.add_option( "--filename,filename", filename, "Verilog file to read in" )->required();
        add_flag("--mig,-m", "Store Verilog file as MIG network (AIG network is default)");
      }

    protected:
      void execute(){

        if(checkExt(filename, "v")){
          if(is_set("mig")){
            mockturtle::mig_network mig;
            mockturtle::names_view<mockturtle::mig_network> names_view{mig};
            lorina::read_verilog(filename, mockturtle::verilog_reader( names_view ));
            std::cout << "MIG network stored" << std::endl;
            store<mig_ntk>().extend() = std::make_shared<mig_names>( names_view );

            filename.erase(filename.end() - 4, filename.end());
            mig._storage->net_name = filename;
          }
          else{
            mockturtle::aig_network aig;
            mockturtle::names_view<mockturtle::aig_network> names_view{aig};
            lorina::read_verilog(filename, mockturtle::verilog_reader( names_view ));
            std::cout << "AIG network stored" << std::endl;
            store<aig_ntk>().extend() = std::make_shared<aig_names>( names_view );

            filename.erase(filename.end() - 2, filename.end());
            aig._storage->net_name = filename;

          }

        }
        else{
          std::cout << filename << " is not a valid Verilog file\n";
        }
        
      }
    private:
      std::string filename{};
    };

  ALICE_ADD_COMMAND(read_verilog, "Input");

  /*Reads an aig file and stores the AIG network in a store*/
  class read_aig_command : public alice::command{

    public:
      explicit read_aig_command( const environment::ptr& env )
          : command( env, "Uses the lorina library to read in an aig file" ){

        opts.add_option( "--filename,filename", filename, "AIG file to read in" )->required();
        add_flag("--mig,-m", "Store AIG file as MIG network (AIG network is default)");
        add_flag("--xag,-x", "Store AIG file as XAG network (AIG network is default)");
      }

    protected:
      void execute(){

        if(checkExt(filename, "aig")){
          if(is_set("mig")){
            mockturtle::mig_network ntk;
            mockturtle::names_view<mockturtle::mig_network> names_view{ntk};
            lorina::read_aiger(filename, mockturtle::aiger_reader( names_view ));
                
            store<mig_ntk>().extend() = std::make_shared<mig_names>( ntk );

            filename.erase(filename.end() - 4, filename.end());
            ntk._storage->net_name = filename;

            // std::cout << "MIG Inputs:\n";
            // names_view.foreach_pi([&](auto pi){
            //   std::cout << "PI: " << pi << " name: " << names_view.get_name(names_view.make_signal(pi)) << "\n";
            // });

            // std::cout << "MIG Outputs:\n";
            // names_view.foreach_po([&](auto po, auto i){
            //   std::cout << "PO: " << po.index << " name: " << names_view.get_output_name(i) << "\n";
            // });

          }
          else if(is_set("xag")){
            mockturtle::xag_network ntk;
            mockturtle::names_view<mockturtle::xag_network> names_view{ntk};
            lorina::read_aiger(filename, mockturtle::aiger_reader( names_view ));
                
            store<xag_ntk>().extend() = std::make_shared<xag_names>( ntk );

            filename.erase(filename.end() - 4, filename.end());
            ntk._storage->net_name = filename;

            // std::cout << "XAG Inputs:\n";
            // names_view.foreach_pi([&](auto pi){
            //   std::cout << "PI: " << pi << " name: " << names_view.get_name(names_view.make_signal(pi)) << "\n";
            // });

            // std::cout << "XAG Outputs:\n";
            // names_view.foreach_po([&](auto po, auto i){
            //   std::cout << "PO: " << po.index << " name: " << names_view.get_output_name(i) << "\n";
            // });

          }
          else{
            mockturtle::aig_network ntk;
            mockturtle::names_view<mockturtle::aig_network> names_view{ntk};
            lorina::read_aiger(filename, mockturtle::aiger_reader( names_view ));
                
            store<aig_ntk>().extend() = std::make_shared<aig_names>( ntk );

            filename.erase(filename.end() - 4, filename.end());
            ntk._storage->net_name = filename;

            // std::cout << "AIG Inputs:\n";
            // names_view.foreach_pi([&](auto pi){
            //   std::cout << "PI: " << pi << " name: " << names_view.get_name(names_view.make_signal(pi)) << "\n";
            // });

            // std::cout << "AIG Outputs:\n";
            // names_view.foreach_po([&](auto po, auto i){
            //   std::cout << "PO: " << po.index << " name: " << names_view.get_output_name(i) << "\n";
            // });

          }
          

        }
        else{
            std::cout << filename << " is not a valid aig file\n";
        }
        
      }
    private:
      std::string filename{};
    };

  ALICE_ADD_COMMAND(read_aig, "Input");

  /*Reads a blif file and stores the network in a store*/
  class read_blif_command : public alice::command{

    public:
      explicit read_blif_command( const environment::ptr& env )
          : command( env, "Uses the lorina library to read in an blif file" ){

        opts.add_option( "--filename,filename", filename, "BLIF file to read in" )->required();
        add_flag("--mig,-m", "Store BLIF file as MIG network (AIG network is default)");
        add_flag("--xag,-x", "Store BLIF file as XAG network (AIG network is default)");
      }

    protected:
      void execute(){

        if(checkExt(filename, "blif")){
          if(is_set("mig")){
            mockturtle::klut_network klut_ntk;
            mockturtle::names_view<mockturtle::klut_network> klut_name_view{klut_ntk};
            lorina::read_blif(filename, mockturtle::blif_reader( klut_name_view ));

            mockturtle::direct_resynthesis<mockturtle::mig_network> resyn;

            mockturtle::mig_network ntk;
            mockturtle::names_view<mockturtle::mig_network>named_dest ( ntk );

            mockturtle::node_resynthesis( named_dest, klut_name_view, resyn );

            // std::cout << "Blif Inputs:\n";
            // klut_name_view.foreach_pi([&](auto pi){
            //   std::cout << "PI: " << pi << " name: " << klut_name_view.get_name(klut_name_view.make_signal(pi)) << "\n";
            // });

            // std::cout << "Blif Outputs:\n";
            // klut_name_view.foreach_po([&](auto po, auto i){
            //   std::cout << "PO: " << po << " name: " << klut_name_view.get_output_name(i) << "\n";
            // });

            // std::cout << "MIG Inputs:\n";
            // named_dest.foreach_pi([&](auto pi){
            //   std::cout << "PI: " << pi << " name: " << named_dest.get_name(named_dest.make_signal(pi)) << "\n";
            // });

            // std::cout << "MIG Outputs:\n";
            // named_dest.foreach_po([&](auto po, auto i){
            //   std::cout << "PO: " << po.index << " name: " << named_dest.get_output_name(i) << "\n";
            // });

            store<mig_ntk>().extend() = std::make_shared<mig_names>( named_dest );
            

          }
          else if(is_set("xag")){
            mockturtle::klut_network klut_ntk;
            mockturtle::names_view<mockturtle::klut_network> klut_name_view{klut_ntk};
            lorina::read_blif(filename, mockturtle::blif_reader( klut_name_view ));

            mockturtle::direct_resynthesis<mockturtle::xag_network> resyn;

            mockturtle::xag_network ntk;
            mockturtle::names_view<mockturtle::xag_network>named_dest ( ntk );

            mockturtle::node_resynthesis( named_dest, klut_name_view, resyn );

            // std::cout << "Blif Inputs:\n";
            // klut_name_view.foreach_pi([&](auto pi){
            //   std::cout << "PI: " << pi << " name: " << klut_name_view.get_name(klut_name_view.make_signal(pi)) << "\n";
            // });

            // std::cout << "Blif Outputs:\n";
            // klut_name_view.foreach_po([&](auto po, auto i){
            //   std::cout << "PO: " << po << " name: " << klut_name_view.get_output_name(i) << "\n";
            // });

            // std::cout << "XAG Inputs:\n";
            // named_dest.foreach_pi([&](auto pi){
            //   std::cout << "PI: " << pi << " name: " << named_dest.get_name(named_dest.make_signal(pi)) << "\n";
            // });

            // std::cout << "XAG Outputs:\n";
            // named_dest.foreach_po([&](auto po, auto i){
            //   std::cout << "PO: " << po.index << " name: " << named_dest.get_output_name(i) << "\n";
            // });

            store<xag_ntk>().extend() = std::make_shared<xag_names>( named_dest );
            

          }
          else{
            mockturtle::klut_network klut_ntk;
            mockturtle::names_view<mockturtle::klut_network> klut_name_view{klut_ntk};
            lorina::read_blif(filename, mockturtle::blif_reader( klut_name_view ));

            mockturtle::direct_resynthesis<mockturtle::aig_network> resyn;

            mockturtle::aig_network ntk;
            mockturtle::names_view<mockturtle::aig_network>named_dest ( ntk );

            mockturtle::node_resynthesis( named_dest, klut_name_view, resyn );

            // std::cout << "Blif Inputs:\n";
            // klut_name_view.foreach_pi([&](auto pi){
            //   std::cout << "PI: " << pi << " name: " << klut_name_view.get_name(klut_name_view.make_signal(pi)) << "\n";
            // });

            // std::cout << "Blif Outputs:\n";
            // klut_name_view.foreach_po([&](auto po, auto i){
            //   std::cout << "PO: " << po << " name: " << klut_name_view.get_output_name(i) << "\n";
            // });

            // std::cout << "AIG Inputs:\n";
            // named_dest.foreach_pi([&](auto pi){
            //   std::cout << "PI: " << pi << " name: " << named_dest.get_name(named_dest.make_signal(pi)) << "\n";
            // });

            // std::cout << "AIG Outputs:\n";
            // named_dest.foreach_po([&](auto po, auto i){
            //   std::cout << "PO: " << po.index << " name: " << named_dest.get_output_name(i) << "\n";
            // });

            store<aig_ntk>().extend() = std::make_shared<aig_names>( named_dest );

          }

        }
        else{
            std::cout << filename << " is not a valid blif file\n";
        }
        
      }
    private:
      std::string filename{};
    };

  ALICE_ADD_COMMAND(read_blif, "Input");

  class show_ntk_command : public alice::command{

    public:
      explicit show_ntk_command( const environment::ptr& env )
          : command( env, "Display details about the stored network" ){

        add_flag("--mig,-m", "Store AIG file as MIG network (AIG network is default)");
      }

    protected:
      void execute(){
        if(is_set("mig")){
          if(!store<mig_ntk>().empty()){
            auto mig = *store<mig_ntk>().current();

            for (int j =1; j < mig._storage->nodes.size(); j++) {
              for (int i = 0; i < mig._storage->nodes.data()->children.size(); i++) {
                std::cout << "node index " << j << " node fan in " << mig._storage->nodes[j].children[i].index << " and data " << mig._storage->nodes[j].children[i].data << std::endl;
              }
            }
            for (unsigned k = mig.num_pis()+1; k<= mig._storage->inputs.size(); k++ ){
              auto node = mig.index_to_node(k);
              std::cout << " reg " << k << " fan out size " << mig.fanout_size(node) << std::endl;
            }
            for (unsigned l=0; l< mig._storage->outputs.size(); l++){
              std::cout << " outputs " << std::endl;
              std::cout << " node fan in data " << mig._storage->outputs[l].data << std::endl;
            }

            std::cout << "Inputs\n";
            mig.foreach_pi([&](auto pi){
              std::cout << "PI: " << pi << " name: " << mig.get_name(mig.make_signal(pi)) << "\n";
            });

            std::cout << "Outputs\n";
            mig.foreach_po([&](auto po, auto i){
              std::cout << "PO: " << po.index << " name: " << mig.get_output_name(i) << "\n";
            });

          }
          else{
            std::cout << "MIG network not stored\n";
          }
        }
        else{
          if(!store<aig_ntk>().empty()){

            auto aig = *store<aig_ntk>().current();

            for (int j =1; j < aig._storage->nodes.size(); j++) {
              for (int i = 0; i < aig._storage->nodes.data()->children.size(); i++) {
                std::cout << "node index " << j << " node fan in " << aig._storage->nodes[j].children[i].index << " and data " << aig._storage->nodes[j].children[i].data << std::endl;
              }
            }

            for (unsigned k = aig.num_pis()+1; k<= (aig._storage->inputs.size() - aig.num_latches()); k++ ){
              auto node = aig.index_to_node(k);
              std::cout << " reg " << k << " fan out size " << aig.fanout_size(node) << std::endl;
            }

            for( int outIndex=0; outIndex<aig.num_pos()- aig.num_latches(); outIndex++){
              std::cout << "Output " << outIndex << " data " << aig._storage->outputs[outIndex].data << std::endl;
            }

            aig.foreach_ri([&](auto fi) {
              std::cout << "Register " << aig.ri_index(fi) << " data " << fi.data << std::endl;
            });

            std::cout << "Inputs\n";
            aig.foreach_pi([&](auto pi){
              std::cout << "PI: " << pi << " name: " << aig.get_name(aig.make_signal(pi)) << "\n";
            });

            std::cout << "Outputs\n";
            aig.foreach_po([&](auto po, auto i){
              std::cout << "PO: " << po.index << " name: " << aig.get_output_name(i) << "\n";
            });
            
          }
          else{
            std::cout << "AIG network not stored\n";
          }
        }
      }
        
    private:
      
    };

  ALICE_ADD_COMMAND(show_ntk, "Output");

  class lut_map_command : public alice::command{

    public:
      explicit lut_map_command( const environment::ptr& env )
          : command( env, "Performs technology mapping of the network" ){

        opts.add_option( "--lut_size,-K", lut_size, "LUT size for mapping [DEFAULT = 6]" );
        opts.add_option( "--cut_size,-C", cut_size, "Max number of priority cuts [DEFAULT = 8]" );
        add_flag("--mig,-m", "Read from the stored MIG network");
        add_flag("--out,-o", "Write LUT mapping to bench file");
      }

    protected:
        void execute(){
            
          if(is_set("mig")){
            if(!store<mig_ntk>().empty()){
                auto& mig = *store<mig_ntk>().current();
                std::string filename = mig._storage->net_name + "_lut.bench";
                mockturtle::mapping_view<mockturtle::mig_network, true> mapped{mig};

                mockturtle::lut_mapping_params ps;
                ps.cut_enumeration_ps.cut_size = lut_size;
                ps.cut_enumeration_ps.cut_limit = cut_size;

                mockturtle::lut_mapping<mockturtle::mapping_view<mockturtle::mig_network, true>, true>( mapped, ps );

                const auto klut_opt = mockturtle::collapse_mapped_network<mockturtle::klut_network>( mapped );
                auto const& klut = *klut_opt;

                mockturtle::depth_view klut_depth{klut};
                std::cout << "LUT = " << mapped.num_cells() << " lev = " << klut_depth.depth() << "\n";
                std::cout << "#LUT Level Product = " << mapped.num_cells() * klut_depth.depth() << "\n";
                std::cout << "Finshed LUT mapping\n";
                if(is_set("out")){
                  std::cout << "filename = " << filename << "\n";
                  mockturtle::write_bench(klut, filename);
                }
            }
            else{
              std::cout << "There is not an MIG network stored.\n";
            }
          }
          else{
            if(!store<aig_ntk>().empty()){
              auto& aig = *store<aig_ntk>().current();
              std::string filename = aig._storage->net_name + "_lut.bench";
              mockturtle::mapping_view<mockturtle::aig_network, true> mapped{aig};

              mockturtle::lut_mapping_params ps;
              ps.cut_enumeration_ps.cut_size = lut_size;
              ps.cut_enumeration_ps.cut_limit = cut_size;

              mockturtle::lut_mapping<mockturtle::mapping_view<mockturtle::aig_network, true>, true>( mapped, ps );

              const auto klut_opt = mockturtle::collapse_mapped_network<mockturtle::klut_network>( mapped );
              auto const& klut = *klut_opt;

              mockturtle::depth_view klut_depth{klut};
              // std::cout << "klut size = " << klut.size() << "\n";
              // klut.foreach_node([&](auto node){
              //   std::cout << "Node = " << node << "\n";
              //   klut.foreach_fanin(node, [&](auto const &conn, auto i){
              //     std::cout << "child[" << i << "] = " << conn << "\n";
              //   });
              // });
              std::cout << "LUT = " << mapped.num_cells() << " lev = " << klut_depth.depth() << "\n";
              std::cout << "#LUT Level Product = " << mapped.num_cells() * klut_depth.depth() << "\n";
              std::cout << "Finshed LUT mapping\n";
              if(is_set("out")){
                std::cout << "filename = " << filename << "\n";
                mockturtle::write_bench(klut, filename);
              }
            }
            else{
              std::cout << "There is not an AIG network stored.\n";
            }
          }
        }    
    private:
      int lut_size = 6;
      int cut_size = 8;
    };

  ALICE_ADD_COMMAND(lut_map, "LUT");


  ALICE_COMMAND( cut_e, "Optimization", "Performs Cut Enumeration on a stored AIG network"){
  	auto aig = *store<aig_ntk>().current();
  	const mockturtle::network_cuts cuts = cut_enumeration( aig );
  	//store<mockturtle::network_cuts>().extend() = cuts;
  	aig.foreach_node( [&]( auto node ) {
  			std::cout << cuts.cuts( aig.node_to_index( node ) ) << "\n";
  	} );
  }

  ALICE_COMMAND( find_xor, "Test", "Convert AIG to XAG and find XOR gates"){
    auto ntk = *store<aig_ntk>().current();
    // mockturtle::write_dot(ntk, "aig_test.dot");
    mockturtle::direct_resynthesis<mockturtle::xag_network> resyn_xag;
    auto xag = mockturtle::node_resynthesis<mockturtle::xag_network>(ntk, resyn_xag);
    std::cout << "size = " << xag.num_gates() << "\n";
    xag.foreach_node( [&]( auto node ) {
      std::cout << "Node = " << node << "\n";
      if(xag.is_xor(node)){
        std::cout << "XOR\n";
      }
    });
    mockturtle::xag_npn_resynthesis<mockturtle::xag_network> resyn;
    mockturtle::cut_rewriting(xag, resyn);
    xag = mockturtle::cleanup_dangling(xag);
    std::cout << "size = " << xag.num_gates() << "\n";
    xag.foreach_node( [&]( auto node ) {
      std::cout << "Node = " << node << "\n";
      if(xag.is_xor(node)){
        std::cout << "XOR\n";
      }
    });
    mockturtle::direct_resynthesis<mockturtle::aig_network> resyn_aig;
    auto aig2 = mockturtle::node_resynthesis<mockturtle::aig_network>(xag, resyn_aig);
    // mockturtle::xag_npn_resynthesis<mockturtle::aig_network> resyn_to_aig;
    // mockturtle::cut_rewriting(aig2, resyn_to_aig);
    std::cout << "size = " << aig2.num_gates() << "\n";
    aig2.foreach_node( [&]( auto node ) {
      std::cout << "Node = " << node << "\n";
      if(aig2.is_xor(node)){
        std::cout << "XOR\n";
      }
    });
    // mockturtle::write_dot(xag, "xag_test.dot");
    // mockturtle::aig_network aig;
    // auto a = aig.create_pi();
    // auto b = aig.create_pi();
    // auto c = aig.create_pi();
    // auto d = aig.create_and(a, b);
    // auto e = aig.create_xor(c, d);
    // aig.create_po(e);

    // aig.foreach_node([&](auto node){
    //   std::cout << "Node = " << node << "\n";
    //   if(aig.is_xor(node)){
    //     std::cout << "XOR\n";
    //   }
    // });
  }

  ALICE_COMMAND( test_aig_conv, "Test", "Test aig to mig and then mig to aig"){
    mockturtle::direct_resynthesis<mockturtle::aig_network> resyn_aig;

    auto aig = *store<aig_ntk>().current();
    mockturtle::write_verilog(aig, aig._storage->net_name + ".v");
    mockturtle::depth_view aig_depth{aig};
    std::cout << "aig size = " << aig.size() << " and depth = " << aig_depth.depth() << "\n";
    mockturtle::write_verilog(aig, aig._storage->net_name + "_aig.v");

    auto mig = *aig_to_mig(aig, 0);
    mockturtle::depth_view mig_depth{mig};
    std::cout << "mig size = " << mig.size() << " and depth = " << mig_depth.depth() << "\n";
    mockturtle::write_verilog(mig, aig._storage->net_name + "_mig.v");

    auto new_aig = *mig_to_aig(mig);
    mockturtle::depth_view new_aig_depth{new_aig};
    std::cout << "new_aig size = " << new_aig.size() << " and depth = " << new_aig_depth.depth() << "\n";
    mockturtle::write_verilog(new_aig, aig._storage->net_name + "_converted.v");

  }

  class partition_detail_command : public alice::command{

  public:
    explicit partition_detail_command( const environment::ptr& env )
        : command( env, "Display all nodes in each Partition" ){

      add_flag("--mig,-m", "Use stored MIG network (Default is AIG)");
      add_flag("--names,-n", "Use stored named networks (Default is Named AIG)");
    }

  protected:
    void execute(){

      if(is_set("names")){
        if(is_set("mig")){
          if(!store<mig_ntk>().empty()){
            auto ntk = store<mig_ntk>().current();
            if(!store<part_man_mig_ntk>().empty()){
              auto partitions = store<part_man_mig_ntk>().current();
              int num_part = partitions->get_part_num();
              for(int i = 0; i < num_part; i++){
                oracle::partition_view<mig_names> part = partitions->create_part(*ntk, i);
                std::cout << "\n\nPartition " << i << "\n";
                std::cout << "Number of PI = " << part.num_pis() << "\n";
                std::cout << "Number of PO = " << part.num_pos() << "\n";
                std::cout << "Number of internal nodes = " << part.num_gates() << "\n";
                std::cout << "Partition volume = " << double(part.num_gates()) / double(part.num_pis()) << "\n";
                // std::cout << "Inputs = {";
                // part.foreach_pi([&](auto pi){
                //   std::cout << pi << " ";
                // });
                // std::cout << "}\n";
                // std::cout << "Outputs = {";
                // part.foreach_po([&](auto conn, auto i){
                //   std::cout << conn.index << " ";
                // });
                // std::cout << "}\n";
                // std::cout << "Nodes = {";
                // part.foreach_gate([&](auto node){
                //   std::cout << node << " ";
                // });
                // std::cout << "}\n";
                std::set<int> connected_parts = partitions->get_connected_parts(*ntk, i);
                std::set<int>::iterator it;
                std::cout << "connected partitions = {";
                for(it = connected_parts.begin(); it != connected_parts.end(); ++it){
                  std::cout << *it << " ";
                } 
                std::cout << "}\n";
              }
            }
            else{
              std::cout << "MIG not partitioned yet\n";
            }
          }
          else{
            std::cout << "There is no MIG network stored\n";
          }
        }
        else{
          if(!store<aig_ntk>().empty()){
            auto ntk = store<aig_ntk>().current();
            if(!store<part_man_aig_ntk>().empty()){
              auto partitions = store<part_man_aig_ntk>().current();
              int num_part = partitions->get_part_num();
              for(int i = 0; i < num_part; i++){
                oracle::partition_view<aig_names> part = partitions->create_part(*ntk, i);
                std::cout << "\n\nPartition " << i << "\n";
                std::cout << "Number of PI = " << part.num_pis() << "\n";
                std::cout << "Number of PO = " << part.num_pos() << "\n";
                std::cout << "Number of internal nodes = " << part.num_gates() << "\n";
                std::cout << "Partition volume = " << double(part.num_gates()) / double(part.num_pis()) << "\n";
                std::cout << "Inputs = {";
                part.foreach_pi([&](auto pi){
                  std::cout << pi << " ";
                });
                std::cout << "}\n";
                std::cout << "Outputs = {";
                part.foreach_po([&](auto conn, auto i){
                  std::cout << conn.index << " ";
                });
                std::cout << "}\n";
                std::cout << "Nodes = {";
                part.foreach_gate([&](auto node){
                  std::cout << node << " ";
                });
                std::cout << "}\n";
                std::set<int> connected_parts = partitions->get_connected_parts(*ntk, i);
                std::set<int>::iterator it;
                std::cout << "connected partitions = {";
                for(it = connected_parts.begin(); it != connected_parts.end(); ++it){
                  std::cout << *it << " ";
                } 
                std::cout << "}\n";
              }
            }
            else{
              std::cout << "AIG not partitioned yet\n";
            }
          }
          else{
            std::cout << "There is no AIG network stored\n";
          }
        }

      }
      // else{
      //   if(is_set("mig")){
      //     if(!store<mockturtle::mig_network>().empty()){
      //       auto ntk = store<mockturtle::mig_network>().current();
      //       if(!store<oracle::partition_manager<mockturtle::mig_network>>().empty()){
      //         auto partitions = store<oracle::partition_manager<mockturtle::mig_network>>().current();
      //         int num_part = partitions.get_part_num();
      //         for(int i = 0; i < num_part; i++){
      //           oracle::partition_view<mockturtle::mig_network> part = partitions.create_part(ntk, i);
      //           std::cout << "\n\nPartition " << i << "\n";
      //           std::cout << "Number of PI = " << part.num_pis() << "\n";
      //           std::cout << "Number of PO = " << part.num_pos() << "\n";
      //           std::cout << "Number of internal nodes = " << part.num_gates() << "\n";
      //           std::cout << "Partition volume = " << double(part.num_gates()) / double(part.num_pis()) << "\n";
      //           // std::cout << "Inputs = {";
      //           // part.foreach_pi([&](auto pi){
      //           //   std::cout << pi << " ";
      //           // });
      //           // std::cout << "}\n";
      //           // std::cout << "Outputs = {";
      //           // part.foreach_po([&](auto conn, auto i){
      //           //   std::cout << conn.index << " ";
      //           // });
      //           // std::cout << "}\n";
      //           // std::cout << "Nodes = {";
      //           // part.foreach_gate([&](auto node){
      //           //   std::cout << node << " ";
      //           // });
      //           // std::cout << "}\n";
      //           std::set<int> connected_parts = partitions.get_connected_parts(ntk, i);
      //           std::set<int>::iterator it;
      //           std::cout << "connected partitions = {";
      //           for(it = connected_parts.begin(); it != connected_parts.end(); ++it){
      //             std::cout << *it << " ";
      //           } 
      //           std::cout << "}\n";
      //         }
      //       }
      //       else{
      //         std::cout << "MIG not partitioned yet\n";
      //       }
      //     }
      //     else{
      //       std::cout << "There is no MIG network stored\n";
      //     }
      //   }
      //   else{
      //     if(!store<mockturtle::aig_network>().empty()){
      //       auto ntk = store<mockturtle::aig_network>().current();
      //       if(!store<oracle::partition_manager<mockturtle::aig_network>>().empty()){
      //         auto partitions = store<oracle::partition_manager<mockturtle::aig_network>>().current();
      //         int num_part = partitions.get_part_num();
      //         for(int i = 0; i < num_part; i++){
      //           oracle::partition_view<mockturtle::aig_network> part = partitions.create_part(ntk, i);
      //           std::cout << "\n\nPartition " << i << "\n";
      //           std::cout << "Number of PI = " << part.num_pis() << "\n";
      //           std::cout << "Number of PO = " << part.num_pos() << "\n";
      //           std::cout << "Number of internal nodes = " << part.num_gates() << "\n";
      //           std::cout << "Partition volume = " << double(part.num_gates()) / double(part.num_pis()) << "\n";
      //           std::cout << "Inputs = {";
      //           part.foreach_pi([&](auto pi){
      //             std::cout << pi << " ";
      //           });
      //           std::cout << "}\n";
      //           std::cout << "Outputs = {";
      //           part.foreach_po([&](auto conn, auto i){
      //             std::cout << conn.index << " ";
      //           });
      //           std::cout << "}\n";
      //           std::cout << "Nodes = {";
      //           part.foreach_gate([&](auto node){
      //             std::cout << node << " ";
      //           });
      //           std::cout << "}\n";
      //           std::set<int> connected_parts = partitions.get_connected_parts(ntk, i);
      //           std::set<int>::iterator it;
      //           std::cout << "connected partitions = {";
      //           for(it = connected_parts.begin(); it != connected_parts.end(); ++it){
      //             std::cout << *it << " ";
      //           } 
      //           std::cout << "}\n";
      //         }
      //       }
      //       else{
      //         std::cout << "AIG not partitioned yet\n";
      //       }
      //     }
      //     else{
      //       std::cout << "There is no AIG network stored\n";
      //     }
      //   }

      // }
      
    }

  private:
  };

  ALICE_ADD_COMMAND(partition_detail, "Partitioning");

  class write_hypergraph_command : public alice::command{

  public:
    explicit write_hypergraph_command( const environment::ptr& env )
        : command( env, "Output current stored network in a hypergraph representation to be used by hMetis" ){

      opts.add_option( "--filename,filename", filename, "hypergraph file to write to" )->required();
      add_flag("--mig,-m", "Use stored MIG network (Default is AIG)");
    }

  protected:
    void execute(){

      if(is_set("mig")){
        if(!store<mig_ntk>().empty()){

          if(checkExt(filename, "hpg")){
            std::ofstream output;
            output.open(filename);

            auto ntk = *store<mig_ntk>().current();  

            // std::map<int, std::vector<int>> connections;

            mockturtle::fanout_view fanout{ntk};
            std::vector<std::vector<int>> hyperEdges;
            ntk.foreach_node( [&]( auto node ) {
              std::vector<int> connections;
              int nodeNdx = ntk.node_to_index(node);

              std::set<mockturtle::node<mockturtle::mig_network>> nodes;
              fanout.foreach_fanout(node, [&](const auto& p){
                nodes.insert(p);
              });

              for(std::set<mockturtle::node<mockturtle::mig_network>>::iterator it = nodes.begin(); it != nodes.end(); ++it){
                
                connections.push_back(ntk.node_to_index(*it));
              }
              if(ntk.is_po(node)){
                connections.push_back(ntk._storage->nodes[node].children[0].index);
                connections.push_back(ntk._storage->nodes[node].children[1].index);
              }
              if(connections.size() > 0){
                  std::vector<int> connection_to_add = connections;
                  //Add root node to the hyper edge
                connection_to_add.insert(connection_to_add.begin(), nodeNdx);
                hyperEdges.push_back(connection_to_add);
              }
            }); 
              
            //Write to the hypergraph file
              //Subtract the size by one because the output node is duplicated with one of the gate nodes
              output << hyperEdges.size() + 1 << " " << ntk.size() /*- 1*/ << "\n";
              for(int i = 0; i < hyperEdges.size(); i++){
              for(int j = 0; j < hyperEdges.at(i).size(); j++){
                //Add 1 to the indeces because hMetis does not recognize hyperedges containing vertex 0
                output << hyperEdges.at(i).at(j) + 1 << " ";
              }
              output << "\n";
            }

            output.close();
          }
          else{
            std::cout << filename << " is not a valid hpg file\n";
          }

          
        }
        else{
          std::cout << "There is no MIG network stored\n";
        }
      }
      else{
        if(!store<aig_ntk>().empty()){

          if(checkExt(filename, "hpg")){
            std::ofstream output;
            output.open(filename);

            auto ntk = *store<aig_ntk>().current();  

            // std::map<int, std::vector<int>> connections;

            mockturtle::fanout_view fanout{ntk};
            std::vector<std::vector<int>> hyperEdges;
            ntk.foreach_node( [&]( auto node ) {
              std::vector<int> connections;
              int nodeNdx = ntk.node_to_index(node);

              std::set<mockturtle::node<mockturtle::aig_network>> nodes;
              fanout.foreach_fanout(node, [&](const auto& p){
                nodes.insert(p);
              });

              for(std::set<mockturtle::node<mockturtle::aig_network>>::iterator it = nodes.begin(); it != nodes.end(); ++it){
                
                connections.push_back(ntk.node_to_index(*it));
              }
              if(ntk.is_po(node)){
                connections.push_back(ntk._storage->nodes[node].children[0].index);
                connections.push_back(ntk._storage->nodes[node].children[1].index);
              }
              if(connections.size() > 0){
                  std::vector<int> connection_to_add = connections;
                  //Add root node to the hyper edge
                connection_to_add.insert(connection_to_add.begin(), nodeNdx);
                hyperEdges.push_back(connection_to_add);
              }
            }); 
              
            //Write to the hypergraph file
              //Subtract the size by one because the output node is duplicated with one of the gate nodes
              output << hyperEdges.size() + 1 << " " << ntk.size() /*- 1*/ << "\n";
              for(int i = 0; i < hyperEdges.size(); i++){
              for(int j = 0; j < hyperEdges.at(i).size(); j++){
                //Add 1 to the indeces because hMetis does not recognize hyperedges containing vertex 0
                output << hyperEdges.at(i).at(j) + 1 << " ";
              }
              output << "\n";
            }

            output.close();
          }
          else{
            std::cout << filename << " is not a valid hpg file\n";
          }

          
        }
        else{
          std::cout << "There is no AIG network stored\n";
        }
      }
    
      
    }

  private:
    std::string filename{};
  };

  ALICE_ADD_COMMAND(write_hypergraph, "Output");

  class optimization_command : public alice::command{

    public:
        explicit optimization_command( const environment::ptr& env )
                : command( env, "Perform Mixed Synthesis on Network after Partitioning" ){

                opts.add_option( "--nn_model,-i", nn_model, "Trained neural network model for classification" );
                opts.add_option( "--out,-o", out_file, "Verilog output" );
                add_flag("--names,-n", "Maintain original PI and PO names when writing result to Verilog output");
                add_flag("--high,-b", "Uses a high effort approach instead of classification");
                add_flag("--aig,-a", "Perform only AIG optimization on all partitions");
                add_flag("--mig,-m", "Perform only MIG optimization on all partitions");
                add_flag("--combine,-c", "Combine adjacent partitions that have been classified for the same optimization");
        }

    protected:
      void execute(){

        mockturtle::direct_resynthesis<mockturtle::mig_network> resyn_mig;
        mockturtle::direct_resynthesis<mockturtle::aig_network> resyn_aig;
        std::vector<int> aig_parts;
        std::vector<int> mig_parts;
        std::vector<int> comb_aig_parts;
        std::vector<int> comb_mig_parts;
        if(!store<aig_ntk>().empty()){

          auto ntk_aig = *store<aig_ntk>().current();

          std::string file_base = ntk_aig._storage->net_name;
          // std::cout << "ntk_aig size = " << ntk_aig.size() << "\n";
          std::string net_name = ntk_aig._storage->net_name;

          if(!store<part_man_aig_ntk>().empty()){
            
            auto start = std::chrono::high_resolution_clock::now();

            auto partitions_aig = *store<part_man_aig_ntk>().current();
            int num_parts = partitions_aig.get_part_num();
            if(is_set("aig")){
              for(int i = 0; i < num_parts; i++){
                aig_parts.push_back(i);
              }
            }
            else if(is_set("mig")){
              for(int i = 0; i < num_parts; i++){
                mig_parts.push_back(i);
              }
            }
            else if(is_set("high")){

              for(int i = 0; i < num_parts; i++){
                oracle::partition_view<aig_names> part_aig = partitions_aig.create_part(ntk_aig, i);

                mockturtle::aig_network aig;
                mockturtle::names_view<mockturtle::aig_network>opt_aig ( aig );

                opt_aig = mockturtle::node_resynthesis<mockturtle::aig_network>( part_aig, resyn_aig );
                // mockturtle::depth_view part_aig_depth{opt_aig};
                mockturtle::test_aig_script aigopt;
                opt_aig = aigopt.run(opt_aig);
                mockturtle::depth_view part_aig_opt_depth{opt_aig};
                int aig_opt_size = opt_aig.num_gates();
                int aig_opt_depth = part_aig_opt_depth.depth();

                mockturtle::mig_network mig;
                mockturtle::names_view<mockturtle::mig_network>opt_mig ( mig );

                opt_mig = mockturtle::node_resynthesis<mockturtle::mig_network>( part_aig, resyn_mig );
                // mockturtle::depth_view part_mig_depth{opt_mig};
                mockturtle::test_mig_script migopt;
                opt_mig = migopt.run(opt_mig);
                mockturtle::depth_view part_mig_opt_depth{opt_mig};
                int mig_opt_size = opt_mig.num_gates();
                int mig_opt_depth = part_mig_opt_depth.depth();

                if((aig_opt_size * aig_opt_depth) <= (mig_opt_size * mig_opt_depth)){
                  aig_parts.push_back(i);
                }
                else{
                  mig_parts.push_back(i);
                }
              }

            }
            else{
              if(!nn_model.empty()){
                partitions_aig.run_classification(ntk_aig, nn_model);

                aig_parts = partitions_aig.get_aig_parts();
                mig_parts = partitions_aig.get_mig_parts();
              }
              else{
                std::cout << "Must include Neural Network model json file\n";
              }

            }

            std::cout << "Scheduled optimization\n";
            std::cout << aig_parts.size() << " AIGs and " << mig_parts.size() << " MIGs\n";

            if(is_set("combine")){
              std::vector<int> visited;
              std::unordered_map<int, int> comb_part;
              for(int i = 0; i < num_parts; i++){
                if(std::find(visited.begin(), visited.end(), i) == visited.end()){
                  std::vector<int> parts_to_combine;
                  
                  std::set<int>::iterator conn_it;
                  std::set<int> conn_parts;
                  conn_parts = partitions_aig.get_connected_parts(ntk_aig, i);
                  if(std::find(aig_parts.begin(), aig_parts.end(), i) != aig_parts.end()){
                    for(conn_it = conn_parts.begin(); conn_it != conn_parts.end(); ++conn_it){
                      if(std::find(aig_parts.begin(), aig_parts.end(), *conn_it) != aig_parts.end()){
                        parts_to_combine.push_back(*conn_it);
                      }
                    }
                  }
                  else{
                    for(conn_it = conn_parts.begin(); conn_it != conn_parts.end(); ++conn_it){
                      if(std::find(mig_parts.begin(), mig_parts.end(), *conn_it) != mig_parts.end()){
                        parts_to_combine.push_back(*conn_it);
                      }
                    }
                  }

                  if(parts_to_combine.size() == 0){
                    if(std::find(aig_parts.begin(), aig_parts.end(), i) != aig_parts.end()){
                      if(std::find(comb_aig_parts.begin(), comb_aig_parts.end(), i) == comb_aig_parts.end()){
                        comb_aig_parts.push_back(i);
                      }
                    }
                    else{
                      if(std::find(comb_mig_parts.begin(), comb_mig_parts.end(), i) == comb_mig_parts.end()){
                        comb_mig_parts.push_back(i);
                      }
                    }
                  }

                  for(int idx = 0; idx < parts_to_combine.size(); idx++){
                    int curr_part = parts_to_combine.at(idx);
                    int part_1 = 0;
                    int part_2 = 0;
                    if(std::find(visited.begin(), visited.end(), curr_part) == visited.end()){
                      part_1 = i;
                      part_2 = curr_part;
                    }
                    else{
                      if(std::find(visited.begin(), visited.end(), i) == visited.end()){
                        part_1 = curr_part;
                        part_2 = i;
                      }
                    }

                    if(std::find(visited.begin(), visited.end(), part_2) == visited.end()){
                      std::unordered_map<int,int>::const_iterator got = comb_part.find (part_1);
                      if(got != comb_part.end()){
                        part_1 = got->second;
                      }
                      std::set<int> connected_parts1 = partitions_aig.get_connected_parts(ntk_aig, part_1);
                      std::set<int> connected_parts2 = partitions_aig.get_connected_parts(ntk_aig, part_2);
                      std::set<int>::iterator conn_it;
                      
                      std::vector<std::set<mockturtle::aig_network::node>> combined_io = partitions_aig.combine_partitions(ntk_aig, part_1, part_2);
                      
                      auto new_inputs = combined_io.at(0);
                      auto new_outputs = combined_io.at(1);
                      comb_part[part_2] = part_1;

                      partitions_aig.set_part_inputs(part_1, new_inputs);
                      partitions_aig.set_part_outputs(part_1, new_outputs);

                      if(std::find(aig_parts.begin(), aig_parts.end(), part_1) != aig_parts.end()){
                        if(std::find(comb_aig_parts.begin(), comb_aig_parts.end(), part_1) == comb_aig_parts.end()){
                          comb_aig_parts.push_back(part_1);
                        }
                      }
                      else{
                        if(std::find(comb_mig_parts.begin(), comb_mig_parts.end(), part_1) == comb_mig_parts.end()){
                          comb_mig_parts.push_back(part_1);
                        }
                      }

                      visited.push_back(part_2); 

                      connected_parts1 = partitions_aig.get_connected_parts(ntk_aig, part_1);
                      for(conn_it = connected_parts1.begin(); conn_it != connected_parts1.end(); ++conn_it){
                        if(std::find(aig_parts.begin(), aig_parts.end(), i) != aig_parts.end()){
                          if(std::find(parts_to_combine.begin(), parts_to_combine.end(), *conn_it) == parts_to_combine.end() && 
                            std::find(aig_parts.begin(), aig_parts.end(), *conn_it) != aig_parts.end() &&
                            std::find(visited.begin(), visited.end(), *conn_it) == visited.end()){

                            parts_to_combine.push_back(*conn_it);
                          }
                        }
                        else{
                          if(std::find(parts_to_combine.begin(), parts_to_combine.end(), *conn_it) == parts_to_combine.end() && 
                            std::find(mig_parts.begin(), mig_parts.end(), *conn_it) != mig_parts.end() &&
                            std::find(visited.begin(), visited.end(), *conn_it) == visited.end()){

                            parts_to_combine.push_back(*conn_it);
                          }
                        }
                        
                      } 
                    }
                    visited.push_back(i);

                  }
                }
              }
              aig_parts = comb_aig_parts;
              mig_parts = comb_mig_parts;
              std::cout << "Scheduled optimization after partition merging\n";
              std::cout << aig_parts.size() << " AIGs and " << mig_parts.size() << " MIGs\n";
            }
            
            auto ntk_mig = *aig_to_mig(ntk_aig, 1);
            oracle::partition_manager<mig_names> partitions_mig(ntk_mig, partitions_aig.get_all_part_connections(), 
                    partitions_aig.get_all_partition_inputs(), partitions_aig.get_all_partition_outputs(), partitions_aig.get_part_num());

            // std::cout << "MIG Inputs:\n";
            // ntk_mig.foreach_pi([&](auto pi){
            //   std::cout << "PI: " << pi << " name: " << ntk_mig.get_name(ntk_mig.make_signal(pi)) << "\n";
            // });

            // std::cout << "MIG Outputs:\n";
            // ntk_mig.foreach_po([&](auto po, auto i){
            //   std::cout << "PO: " << po.index << " name: " << ntk_mig.get_output_name(i) << "\n";
            // });

            // std::cout << "AIG Optimization\n";
            for(int i = 0; i < aig_parts.size(); i++){
              
              oracle::partition_view<mig_names> part = partitions_mig.create_part(ntk_mig, aig_parts.at(i));
              mockturtle::depth_view part_depth{part};

              auto opt_part = *part_to_mig(part, 1);
              auto opt = *mig_to_aig(opt_part);

              mockturtle::depth_view opt_part_depth{opt};

              mockturtle::test_aig_script aigopt;
              opt = aigopt.run(opt);

              auto opt_mig = *aig_to_mig(opt, 0);
              mockturtle::depth_view part_opt_depth{opt_mig};

              partitions_mig.synchronize_part(part, opt_mig, ntk_mig);
            }
            // std::cout << "MIG Optimization\n";
            for(int i = 0; i < mig_parts.size(); i++){
              
              oracle::partition_view<mig_names> part = partitions_mig.create_part(ntk_mig, mig_parts.at(i));
              mockturtle::depth_view part_depth{part};

              auto opt = *part_to_mig(part, 0);

              mockturtle::depth_view opt_part_depth{opt};
              
              mockturtle::test_mig_script migopt;
              opt = migopt.run(opt);
              
              mockturtle::depth_view part_opt_depth{opt};

              partitions_mig.synchronize_part(part, opt, ntk_mig);
            }
            
            partitions_mig.connect_outputs(ntk_mig);
            
            mockturtle::depth_view ntk_before_depth2{ntk_mig};
            
            ntk_mig = mockturtle::cleanup_dangling( ntk_mig );
            mockturtle::depth_view ntk_depth2{ntk_mig};
            std::cout << "Final ntk size = " << ntk_mig.num_gates() << " and depth = " << ntk_depth2.depth() << "\n";
            std::cout << "Area Delay Product = " << ntk_mig.num_gates() * ntk_depth2.depth() << "\n";
            auto stop = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
            std::cout << "Full Optimization: " << duration.count() << "ms\n";
            std::cout << "Finished optimization\n";
            store<mig_ntk>().extend() = std::make_shared<mig_names>( ntk_mig );

            // std::cout << "MIG Inputs:\n";
            // ntk_mig.foreach_pi([&](auto pi){
            //   std::cout << "PI: " << pi << " name: " << ntk_mig.get_name(ntk_mig.make_signal(pi)) << "\n";
            // });

            // std::cout << "MIG Outputs:\n";
            // ntk_mig.foreach_po([&](auto po, auto i){
            //   std::cout << "PO: " << po.index << " name: " << ntk_mig.get_output_name(i) << "\n";
            // });

            // mockturtle::NameMap<mockturtle::mig_network> names;
            // ntk_mig.foreach_pi([&](auto pi, auto i){
            //   names.insert(ntk_mig.make_signal(pi), input_names.at(i));
            // });
            // ntk_mig.foreach_po([&](auto po, auto i){
            //   names.insert(po, output_names.at(i));
            // });

            // store<mockturtle::NameMap<mockturtle::mig_network>>().extend() = names;

            if(out_file != ""){
              
              mockturtle::write_verilog(ntk_mig, out_file);
              std::cout << "Resulting Verilog written to " << out_file << "\n";
            }
        
          }
          else{
              std::cout << "AIG not partitioned yet\n";
          }
        }
        else{
          std::cout << "No AIG stored\n";
        }
      }
    private:
        std::string nn_model{};
        std::string out_file{};
    };

  ALICE_ADD_COMMAND(optimization, "Optimization");

  class find_part_command : public alice::command{

    public:
        explicit find_part_command( const environment::ptr& env )
                : command( env, "Classify partitions and perform corresponding optimization" ){

            opts.add_option( "--node,-n", nodeIdx, "Node to find partition of" )->required();
            add_flag("--mig,-m", "Look at stored MIG");
        }

    protected:
      void execute(){
        int partition = 0;

        if(is_set("mig")){
          if(!store<mig_ntk>().empty()){

            auto ntk = *store<mig_ntk>().current();

            if(!store<part_man_mig_ntk>().empty()){
              
              auto partitions = *store<part_man_mig_ntk>().current();

              mockturtle::mig_network::node node_to_find = ntk.node_to_index(nodeIdx);
              int num_partitions = partitions.get_part_num();
              for(int i = 0; i < num_partitions; i++){
                oracle::partition_view<mig_names> part = partitions.create_part(ntk, i);
                auto nodes = part.get_node_list();
                if(std::find(nodes.begin(), nodes.end(), node_to_find) != nodes.end()){
                  std::cout << "Found in partition " << i << " ";
                  if(part.is_pi(node_to_find)){
                    std::cout << "Input\n";
                  }
                  else if(part.is_po(node_to_find)){
                    std::cout << "Output\n";
                  }
                  else if(part.is_pi(node_to_find) && part.is_po(node_to_find)){
                    std::cout << "Input and Output\n";
                  }
                  else{
                    std::cout << "\n";
                  }
                }
              }
          
            }
            else{
                std::cout << "MIG not partitioned yet\n";
            }
          }
          else{
            std::cout << "No MIG stored\n";
          }
        }
        else{
          if(!store<aig_ntk>().empty()){

            auto ntk = *store<aig_ntk>().current();

            if(!store<part_man_aig_ntk>().empty()){

              auto partitions = *store<part_man_aig_ntk>().current();

              mockturtle::aig_network::node node_to_find = ntk.node_to_index(nodeIdx);
              int num_partitions = partitions.get_part_num();
              for(int i = 0; i < num_partitions; i++){
                oracle::partition_view<aig_names> part = partitions.create_part(ntk, i);
                auto nodes = part.get_node_list();
                if(std::find(nodes.begin(), nodes.end(), node_to_find) != nodes.end()){
                  std::cout << "Found in partition " << i << " ";
                  if(part.is_pi(node_to_find)){
                    std::cout << "Input\n";
                  }
                  else if(part.is_po(node_to_find)){
                    std::cout << "Output\n";
                  }
                  else if(part.is_pi(node_to_find) && part.is_po(node_to_find)){
                    std::cout << "Input and Output\n";
                  }
                  else{
                    std::cout << "\n";
                  }
                }
              }
          
            }
            else{
                std::cout << "AIG not partitioned yet\n";
            }
          }
          else{
            std::cout << "No AIG stored\n";
          }
        }
        
      }
    private:
        int nodeIdx = 0;
    };

  ALICE_ADD_COMMAND(find_part, "Testing");

  class xor_pattern_rec_command : public alice::command{

    public:
        explicit xor_pattern_rec_command( const environment::ptr& env )
                : command( env, "Find groups of nodes that make up XOR patterns" ){

            add_flag("--mig,-m", "Look at stored MIG");
        }

    protected:
      void execute(){

        if(is_set("mig")){
          if(!store<mig_ntk>().empty()){

            auto ntk = *store<mig_ntk>().current();
            oracle::pattern_view<mig_names> patt(ntk);
            patt.foreach_xor([&]( auto xor_patt, int i ){
              std::cout << "XOR number " << i << "\n";
              std::cout << "Nodes = ";
              for(int i = 0; i < xor_patt.size(); i++){
                std::cout << xor_patt.at(i) << " ";
              }
              std::cout << "}\n";
              auto inputs = patt.get_inputs(ntk, i);
              std::cout << "Inputs = ";
              for(int i = 0; i < inputs.size(); i++){
                std::cout << inputs.at(i) << " ";
              }
              std::cout << "\n";

              auto fanout = patt.get_fanout(ntk, i);
              std::cout << "fanout = ";
              for(int i = 0; i < fanout.size(); i++){
                std::cout << fanout.at(i) << " ";
              }
              std::cout << "}\n";

              oracle::slack_view<mockturtle::mig_network> slack(ntk);
              std::cout << "On critical path = " << slack.is_critical_path(patt.get_output(i)) << "\n";
            });

            patt.run_partitioning(ntk, 16, 30);
          }
          else{
            std::cout << "No MIG stored\n";
          }
        }
        else{
          if(!store<aig_ntk>().empty()){

            auto ntk = *store<aig_ntk>().current();
            oracle::pattern_view<aig_names> patt(ntk);
            patt.foreach_xor([&]( auto xor_patt, int i ){
              std::cout << "XOR number " << i << "\n";
              std::cout << "Nodes = {";
              for(int i = 0; i < xor_patt.size(); i++){
                std::cout << xor_patt.at(i) << " ";
              }
              std::cout << "}\n";
              auto inputs = patt.get_inputs(ntk, i);
              std::cout << "Inputs = {";
              for(int i = 0; i < inputs.size(); i++){
                std::cout << inputs.at(i) << " ";
              }
              std::cout << "}\n";

              auto fanout = patt.get_fanout(ntk, i);
              std::cout << "fanout = {";
              for(int i = 0; i < fanout.size(); i++){
                std::cout << fanout.at(i) << " ";
              }
              std::cout << "}\n";

              oracle::slack_view<mockturtle::aig_network> slack(ntk);
              std::cout << "On critical path = " << slack.is_critical_path(patt.get_output(i)) << "\n";
            });

            patt.run_partitioning(ntk, 16, 30);
          }
          else{
            std::cout << "No AIG stored\n";
          }
        }
        
      }
    private:
        
    };

  ALICE_ADD_COMMAND(xor_pattern_rec, "Testing");

  class get_fanout_command : public alice::command{

    public:
        explicit get_fanout_command( const environment::ptr& env )
                : command( env, "Get the fanout list of a given node" ){

            opts.add_option( "--node,-n", nodeIdx, "Node to find fanout of" )->required();
            add_flag("--mig,-m", "Look at stored MIG");
        }

    protected:
      void execute(){
        int partition = 0;

        if(is_set("mig")){
          if(!store<mig_ntk>().empty()){

            auto ntk = *store<mig_ntk>().current();
            mockturtle::fanout_view fanout{ntk};
            std::cout << "Fanout of " << nodeIdx << " = {";
            fanout.foreach_fanout(ntk.index_to_node(nodeIdx), [&](auto const& p){
              std::cout << p << " ";
            });
            std::cout << "}\n";
          }
          else{
            std::cout << "No MIG stored\n";
          }
        }
        else{
          if(!store<aig_ntk>().empty()){

            auto ntk = *store<aig_ntk>().current();
            mockturtle::fanout_view fanout{ntk};
            std::cout << "Fanout of " << nodeIdx << " = {";
            fanout.foreach_fanout(ntk.index_to_node(nodeIdx), [&](auto const& p){
              std::cout << p << " ";
            });
            std::cout << "}\n";
          }
          else{
            std::cout << "No AIG stored\n";
          }
        }
        
      }
    private:
        int nodeIdx = 0;
    };

  ALICE_ADD_COMMAND(get_fanout, "Testing");

  // class test_man_part_command : public alice::command{

  //   public:
  //       explicit test_man_part_command( const environment::ptr& env )
  //               : command( env, "Manually set nodes to be in partitions" ){

  //       }

  //   protected:
  //     void execute(){
        
  //       if(!store<aig_ntk>().empty()){

  //         auto ntk = *store<aig_ntk>().current();
  //         std::map<aig_names::node, int> partitions;
  //         // std::set<int> aig_part = {9, 10, 11, 12, 13, 14, 18, 27, 29, 20, 22, 19, 23, 28, 30, 21, 24, 36, 37, 31, 25, 34, 38, 32, 26, 39, 33, 40, 35};
  //         std::set<int> aig_part = {9, 10, 13, 14, 11, 12, 24, 21, 19, 22, 18, 29, 27, 23, 20, 30, 28, 39, 34, 31, 25, 32, 36, 26, 33, 35, 37, 41, 48, 38, 46};
  //         // std::set<int> aig_part = {9, 10, 13, 14, 11, 12, 24, 21, 19, 22, 18, 29, 27, 23, 20, 30, 28, 39, 34, 31, 25, 32, 36, 26, 33, 35, 37, 41, 48, 38, 46, 42, 62, 40, 49, 47, 54, 53, 58};
  //         // std::set<int> aig_part = {9, 10, 13, 14, 11, 12, 24, 21, 19, 22, 18, 29, 27, 23, 20, 30, 28, 39, 34, 31, 25, 32, 36, 26, 33, 35, 37, 41, 48, 38, 46, 42, 58};
  //         // std::set<int> aig_part = {9, 10, 13, 14, 11, 12, 24, 21, 19, 22, 18, 29, 27, 23, 20, 30, 28, 39, 34, 31, 25, 32, 36, 26, 33, 35, 37};

  //         ntk.foreach_node([&](auto node){
  //           if(aig_part.find(ntk.node_to_index(node)) != aig_part.end()){
  //             partitions[node] = 0;
  //           }
  //           else{
  //             partitions[node] = 1;
  //           }
  //         });

  //         part_man_aig_ntk part_man(ntk, partitions, 2);
  //         store<oracle::partition_manager<mockturtle::aig_network>>().extend() = part_man;
  //       }
  //       else{
  //         std::cout << "No AIG stored\n";
  //       }
        
        
  //     }
  //   private:
  //   };

  // ALICE_ADD_COMMAND(test_man_part, "Testing");


  ALICE_COMMAND( partition_sizes, "Partitioning", "Shows the size of each partition"){

    if(!store<aig_ntk>().empty()){
      auto aig = *store<aig_ntk>().current();
      for(int i = 0; i < aig._storage->num_partitions; i++){
        std::cout << "Partition=" << i << ": ";
        std::cout << aig._storage->partitionSize[i] << "\n";
      }
    }
    else{
      std::cout << "There is no AIG network stored\n";
    }
  }

  ALICE_COMMAND( read_lib, "STA", "Reads standard cell library"){
    std::string filename = "";
    std::cout << "Enter liberty path: ";
    std::cin >> filename;

    sta_cfg.set_lib_path(filename);
  }

  ALICE_COMMAND( read_netlist, "STA", "Reads mapped verilog"){
    std::string filename = "";
    std::cout << "Enter verilog path: ";
    std::cin >> filename;

    sta_cfg.set_netlist_path(filename);
  }

  ALICE_COMMAND( read_sdc, "STA", "Reads constraint file"){
    std::string filename = "";
    std::cout << "Enter sdc path: ";
    std::cin >> filename;

    sta_cfg.set_sdc_path(filename);
  }

  ALICE_COMMAND( run_slack, "STA", "Shows WNS and TNS"){

    sta_cfg.run_slack();
  }

  ALICE_COMMAND( report_critical_path, "STA", "Runs STA"){

    sta_cfg.run_report_timing();
  }

  class depth_command : public alice::command{

    public:
      explicit depth_command( const environment::ptr& env )
          : command( env, "Displays the depth of the stored network" ){

        add_flag("--mig,-m", "Display depth of stored MIG (AIG is default)");
      }

    protected:
      void execute(){
      
        if(is_set("mig")){
          if(!store<mig_ntk>().empty()){
            auto& mig = *store<mig_ntk>().current();
            mockturtle::depth_view mig_depth{mig};

            std::cout << "MIG level " << mig_depth.depth()  << std::endl;
          }
          else{
            std::cout << "There is not an MIG network stored.\n";
          }
        }
        else{
          if(!store<aig_ntk>().empty()){
            auto& aig = *store<aig_ntk>().current();
            mockturtle::depth_view aig_depth{aig};

            std::cout << "AIG level " << aig_depth.depth()  << std::endl;
          }
          else{
            std::cout << "There is not an AIG network stored.\n";
          }
        }
      }
    private:
  };

  ALICE_ADD_COMMAND(depth, "Network_Statistics");

  ALICE_COMMAND(get_cones, "Network_Statistics", "Displays size and depth of all logic cones in the stored AIG network") {

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
    }
    else{
      std::cout << "There is not an AIG network stored.\n";
    }
  }

  class disjoint_clouds_command : public alice::command{

    public:
      explicit disjoint_clouds_command( const environment::ptr& env )
          : command( env, "Writes the status for each disjoint combinational cloud on the current AIG" ){

        opts.add_option( "--filename,filename", filename, "File to write to" )->required();
      }

    protected:
      void execute(){
        
        
        if(!store<aig_ntk>().empty()){
          auto& aig = *store<aig_ntk>().current();
          std::ofstream dset;
          dset.open (filename);

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
          aig.foreach_ri( [&] (auto ri) {

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

          for (std::pair<int, int> element : number_nodes) {
            dset << element.second << std::endl;
          }

          std::cout << "Number of disjoint graphs: " << uf.getNumbOfSets() << std::endl;

          dset.close();
        }
        else{
          std::cout << "There is not an AIG network stored.\n";
        }
      }
    private:
      std::string filename{};
  };

  ALICE_ADD_COMMAND(disjoint_clouds, "Output");

  class write_verilog_command : public alice::command{

    public:
      explicit write_verilog_command( const environment::ptr& env )
          : command( env, "Writes the Boolean network into structural verilog" ){

        opts.add_option( "--filename,filename", filename, "Verilog file to write out to" )->required();
        add_flag("--mig,-m", "Read from the MIG network");
      }

    protected:
        void execute(){
        if(checkExt(filename, "v")){
          if(is_set("mig")){
            if(!store<mig_ntk>().empty()){
              auto& mig = *store<mig_ntk>().current();
              mockturtle::write_verilog(mig, filename);
            }
            else{
              std::cout << "There is not an MIG network stored.\n";
            }
          }
          else{
            if(!store<aig_ntk>().empty()){
              auto& aig = *store<aig_ntk>().current();
              mockturtle::write_verilog(aig, filename);
            }
            else{
              std::cout << "There is not an AIG network stored.\n";
            }
          }
        }
        else{
            std::cout << filename << " is not a valid verilog file\n";
        }
      }
    private:
      std::string filename{};
  };

  ALICE_ADD_COMMAND(write_verilog, "Output");

  class write_blif_command : public alice::command{

    public:
      explicit write_blif_command( const environment::ptr& env )
          : command( env, "Writes the Boolean network into a BLIF file" ){

        opts.add_option( "--filename,filename", filename, "BLIF file to write out to" )->required();
        add_flag("--mig,-m", "Write the stored MIG network in BLIF format");
        add_flag("--xag,-x", "Write the stored XAG network in BLIF format");
      }

    protected:
        void execute(){
        if(checkExt(filename, "blif")){
          if(is_set("mig")){
            if(!store<mig_ntk>().empty()){
              auto mig = store<mig_ntk>().current();
              mockturtle::write_blif( *mig, filename );
            }
            else{
              std::cout << "There is not an MIG network stored.\n";
            } 
          } 
          else if(is_set("xag")){
            if(!store<xag_ntk>().empty()){
              auto xag = store<xag_ntk>().current();

              mockturtle::write_blif( *xag, filename );
            }
            else{
              std::cout << "There is not an AIG network stored.\n";
            }
          }                 
          else{
            if(!store<aig_ntk>().empty()){
              auto aig = store<aig_ntk>().current();

              mockturtle::write_blif( *aig, filename );
            }
            else{
              std::cout << "There is not an AIG network stored.\n";
            }
          }
        }
        else{
            std::cout << filename << " is not a valid BLIF file\n";
        }
      }
    private:
      std::string filename{};
  };

  ALICE_ADD_COMMAND(write_blif, "Output");

  class write_bench_command : public alice::command{

    public:
      explicit write_bench_command( const environment::ptr& env )
          : command( env, "Writes the Boolean network into bench format" ){

        opts.add_option( "--filename,filename", filename, "Bench file to write out to" )->required();
        add_flag("--mig,-m", "Read from the MIG network");
      }

    protected:
      void execute(){
        if(checkExt(filename, "bench")){
          if(is_set("mig")){
            if(!store<mig_ntk>().empty()){
              auto& mig = *store<mig_ntk>().current();
              mockturtle::write_bench(mig, filename);
            }
            else{
              std::cout << "There is not an MIG network stored.\n";
            }
          }
          else{
            if(!store<aig_ntk>().empty()){
              auto& aig = *store<aig_ntk>().current();
              mockturtle::write_bench(aig, filename);
            }
            else{
              std::cout << "There is not an AIG network stored.\n";
            }
          }
        }
        else{
            std::cout << filename << " is not a valid bench file\n";
        }
      }
    private:
      std::string filename{};
  };

  ALICE_ADD_COMMAND(write_bench, "Output");

  class write_dot_command : public alice::command{

    public:
      explicit write_dot_command( const environment::ptr& env )
          : command( env, "Writes the Boolean network into structural verilog" ){

        opts.add_option( "--filename,filename", filename, "Verilog file to write out to" )->required();
        add_flag("--mig,-m", "Read from the MIG network and MIG partition manager for truth table generation");
      }

    protected:
      void execute(){
        if(checkExt(filename, "dot")){
          if(is_set("mig")){
            if(!store<mig_ntk>().empty()){
              auto& mig = *store<mig_ntk>().current();
              mockturtle::write_dot(mig, filename);
            }
            else{
              std::cout << "There is not an MIG network stored.\n";
            }
          }
          else{
            if(!store<aig_ntk>().empty()){
              auto& aig = *store<aig_ntk>().current();
              mockturtle::write_dot(aig, filename);
            }
            else{
              std::cout << "There is not an AIG network stored.\n";
            }
          }
            
        }
        else{
            std::cout << filename << " is not a valid dot file\n";
        }
      }

    private:
      std::string filename{};
    };

  ALICE_ADD_COMMAND(write_dot, "Output");

  ALICE_COMMAND(interleaving, "Modification", "NPN + depth MIG rewriting") {
    if(!store<mig_ntk>().empty()){
      auto& mig = *store<mig_ntk>().current();

      mockturtle::mig_npn_resynthesis resyn;
      mockturtle::cut_rewriting_params ps;

      ps.cut_enumeration_ps.cut_size = 4;

      mockturtle::cut_rewriting(mig, resyn, ps);
      mig = mockturtle::cleanup_dangling( mig );
    }
    else{
      std::cout << "There is not an MIG network stored.\n";
    }

  }

  ALICE_COMMAND(migscript, "Modification", "Exact NPN MIG rewriting") {
    if(!store<mig_ntk>().empty()){
    	auto& opt = *store<mig_ntk>().current();

      auto start = std::chrono::high_resolution_clock::now();
      mockturtle::depth_view mig_depth{opt};

    	//DEPTH REWRITING
      std::cout << "MIG logic depth " << mig_depth.depth() << " Majority nodes " << opt.num_gates() << std::endl;

      mockturtle::mig_script migopt;
      opt = migopt.run(opt);

      mockturtle::depth_view new_mig_depth{opt};
      std::cout << "MIG logic depth " << new_mig_depth.depth() << " Majority nodes " << opt.num_gates() << std::endl;

      std::cout << "Final ntk size = " << opt.num_gates() << " and depth = " << new_mig_depth.depth() << "\n";
      std::cout << "Area Delay Product = " << opt.num_gates() * new_mig_depth.depth() << "\n";
      auto stop = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
      std::cout << "Full Optimization: " << duration.count() << "ms\n";
      std::cout << "Finished optimization\n";

    }
    else{
      std::cout << "There is not an MIG network stored.\n";
    }

  }

  ALICE_COMMAND(aigscript, "Modification", "NPN XAG cut rewriting") {
    if(!store<aig_ntk>().empty()){
      auto& opt = *store<aig_ntk>().current();

      auto start = std::chrono::high_resolution_clock::now();
      mockturtle::depth_view aig_depth{opt};

      //DEPTH REWRITING
      std::cout << "AIG logic depth " << aig_depth.depth() << " nodes " << opt.num_gates() << std::endl;

      mockturtle::aig_script aigopt;
      opt = aigopt.run(opt);

      mockturtle::depth_view new_aig_depth{opt};
      std::cout << "AIG logic depth " << new_aig_depth.depth() << " nodes " << opt.num_gates() << std::endl;

      std::cout << "Final ntk size = " << opt.num_gates() << " and depth = " << new_aig_depth.depth() << "\n";
      std::cout << "Area Delay Product = " << opt.num_gates() * new_aig_depth.depth() << "\n";
      auto stop = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
      std::cout << "Full Optimization: " << duration.count() << "ms\n";
      std::cout << "Finished optimization\n";

    }
    else{
      std::cout << "There is not an AIG network stored.\n";
    }

  }

  ALICE_COMMAND(aigscript2, "Modification", "Based on ABC resyn2 script without AIG balancing") {
    if(!store<aig_ntk>().empty()){
      auto& opt = *store<aig_ntk>().current();

      auto start = std::chrono::high_resolution_clock::now();
      mockturtle::depth_view aig_depth{opt};

      //DEPTH REWRITING
      std::cout << "AIG logic depth " << aig_depth.depth() << " nodes " << opt.num_gates() << std::endl;

      mockturtle::aig_script2 aigopt;
      opt = aigopt.run(opt);

      mockturtle::depth_view new_aig_depth{opt};
      std::cout << "AIG logic depth " << new_aig_depth.depth() << " nodes " << opt.num_gates() << std::endl;

      std::cout << "Final ntk size = " << opt.num_gates() << " and depth = " << new_aig_depth.depth() << "\n";
      std::cout << "Area Delay Product = " << opt.num_gates() * new_aig_depth.depth() << "\n";
      auto stop = std::chrono::high_resolution_clock::now();
      auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
      std::cout << "Full Optimization: " << duration.count() << "ms\n";
      std::cout << "Finished optimization\n";

    }
    else{
      std::cout << "There is not an AIG network stored.\n";
    }

  }

  ALICE_COMMAND(depthr, "Modification", "Logic depth oriented MIG rewriting"){
    if(!store<mig_ntk>().empty()){
      auto& mig = *store<mig_ntk>().current();
    	std::cout << "Mig gates " << mig.num_gates() << std::endl;

    	//to compute at level
    	mockturtle::depth_view mig_depth{mig};

    	std::cout << "Mig level " << mig_depth.depth() << " mig gates " << mig.num_gates() << std::endl;

    	mockturtle::mig_algebraic_depth_rewriting_params pm;
    	pm.aggressive;
    	mockturtle::mig_algebraic_depth_rewriting(mig_depth, pm);

    	mig = mockturtle::cleanup_dangling( mig );
    	std::cout << "Mig level after algebraic rewriting " << mig_depth.depth() << " Mig gates " << mig.num_gates() << std::endl;
    }
    else{
      std::cout << "There is not an MIG network stored.\n";
    }
  }

  // ALICE_CONVERT( mockturtle::mig_network, element, mockturtle::aig_network ) {
  //   auto aig = mockturtle::aig_network();
  // 	std::unordered_set<uint32_t> toinvert;
  // 	std::unordered_set<uint32_t> duplicated;
  // 	std::map<uint32_t , mockturtle::aig_network::signal> update_output ;
  // 	std::vector<int> missing;

  //   //create aig inputs
  // 	for (int i = 0; i < element.num_pis() - element.num_latches(); ++i) {
  // 		aig.create_pi();
  // 	}

  //   //create registers output (PIs in the AIG)
  //   for (int i=0; i<element.num_latches(); i++){
  //   	aig.create_ro();
  //   }

  //   //convert majs into ands
  // 	element.foreach_gate([&] (auto gate) {
  // 		auto index = element.node_to_index(gate);

  // 		//ignore PIs and Latch inputs
  // 		if(index > element.num_pis()) {

  // 			if(element._storage->nodes[index].children[0].data == 1){

  // 				auto child2 = element._storage->nodes[index].children[1];
  // 				auto child3 = element._storage->nodes[index].children[2];

  // 				auto search = toinvert.find(2 * child2.index);
  // 				if (search == toinvert.end()) {
  // 					child2 = aig.create_not(child2);
  // 				}

  // 				auto search1 = toinvert.find(2 * child3.index);
  // 				if (search1 == toinvert.end()) {
  // 					child3 = aig.create_not(child3);
  // 				}

  // 				//add complemented nodes. If a complemented node (first child equals to 1) is fan-in of a node, it should be complemented while generating the aig
  // 				toinvert.insert(2 * index);

  // 				aig.create_maj(aig.child_to_signal(0), child2, child3);
  // 			}

  // 			else if (element._storage->nodes[index].children[0].data == 0){
  // 				auto child2 = element._storage->nodes[index].children[1];
  // 				auto child3 = element._storage->nodes[index].children[2];

  // 				auto search = toinvert.find(2 * child2.index);
  // 				if (search != toinvert.end()) {
  // 					child2 = aig.create_not(child2);
  // 					duplicated.insert(child2.index * 2);
  // 					toinvert.erase(2*index);
  // 				}

  // 				auto search1 = toinvert.find(2 * child3.index);
  // 				if (search1 != toinvert.end()) {
  // 					child3 = aig.create_not(child3);
  // 					duplicated.insert(child3.index * 2);
  // 				}

  // 				aig.create_maj(aig.child_to_signal(0), child2, child3);
  // 			}

  // 			else{
  // 				auto child1 = element._storage->nodes[index].children[0];
  // 				auto child2 = element._storage->nodes[index].children[1];
  // 				auto child3 = element._storage->nodes[index].children[2];

  // 				auto search = toinvert.find(2 * child1.index);
  // 				auto search0 = duplicated.find(2 * child1.index);
  // 				if (search != toinvert.end() && search0 == duplicated.end()) {
  // 					child1 = aig.create_not(child1);
  // 				}

  // 				auto search1 = duplicated.find(2 * child2.index);
  // 				auto search2 = toinvert.find(2 * child2.index);
  // 				if (search2 != toinvert.end() && search1 == duplicated.end()) {
  // 					child2 = aig.create_not(child2);
  // 				}

  // 				auto search3 = toinvert.find(2 * child3.index);
  // 				auto search4 = duplicated.find(2*child3.index);
  // 				if (search3 != toinvert.end() && search4 == duplicated.end()) {
  // 					child3 = aig.create_not(child3);
  // 				}

  // 				auto maj = aig.create_maj(child1, child2, child3);

  // 				std::cout << "Inserting in the table " << 2*index << " " << maj.index << std::endl;
  // 				update_output.insert(std::make_pair(2*index, maj));

  // 			}
  // 		}
  // 	});

  // 	aig.foreach_gate([&] (auto gate) {
  // 		auto index = aig.node_to_index(gate);

  // 		//ignore PIs and Latch inputs
  // 		if(index > aig.num_pis()) {

  // 			auto child1 = aig._storage->nodes[index].children[0];
  // 			auto child2 = aig._storage->nodes[index].children[1];

  // 			auto it = update_output.find(2*child1.index);

  // 			if(it!=update_output.end()){
  // 				if(index < (it->first)/2) {
  // 					//it->second.complement = it->second.complement ^ 1;
  // 					std::cout << "Child1! My index " << index <<std::endl;
  // 					std::cout << "Replacing " << 2*child1.index << " by " << it->second.index << " polarity " << it->second.complement << std::endl;
  // 					aig._storage->nodes[index].children[0] = it->second;
  // 					//aig.update();
  // 				}
  // 			}

  // 			auto it1 = update_output.find(2*child2.index);

  // 			if(it1!=update_output.end()){
  // 				if(index < (it1->first)/2) {
  // 					//it1->second.complement = it1->second.complement ^ 1;
  // 					std::cout << "Child2! My index " << index <<std::endl;
  // 					std::cout << "Replacing " << 2*child2.index << " by " << it1->second.index << " polarity " << it1->second.complement << std::endl;
  // 					aig._storage->nodes[index].children[1] = it1->second;
  // 					//aig.update();
  // 				}
  // 			}
  // 		}
  // 	});

  //   //create aig outputs
  // 	for(int i = 0; i < element.num_pos() - element.num_latches(); i++){
  // 		auto in = element._storage->outputs[i].data;
  // 		auto signal = element.child_to_signal(in);
  // 		auto signalOut = mockturtle::aig_network::signal(signal);

  // 		auto it = update_output.find(in);

  // 		if(it!=update_output.end()){
  // 			signalOut = it->second;
  // 		}

  // 		else {
  // 			auto search = toinvert.find(1 ^ in);
  // 			if (search != toinvert.end()) {
  // 				signalOut = aig.create_not(signalOut);
  // 			}

  // 			auto search1 = toinvert.find(in);
  // 			if (search1 != toinvert.end()) {
  // 				signalOut = aig.create_not(signalOut);
  // 			}
  // 		}
  // 		aig.create_po(signalOut);
  // 	}

  // 	//create registers input as AIG PO
  // 	for (unsigned int i = 0; i < element.num_latches() ; ++i) {
  // 		int8_t reset = 0;
  // 		std::string name = "";
  // 		auto regIdx = element.num_pos() - element.num_latches() + i;
  // 		auto signal = element.ri_at(regIdx);

  // 		bool comp = false;
  // 		auto temp = signal.data;

  // 		auto it = update_output.find(2*signal.index);

  // 		if(it!=update_output.end()){
  // 			auto ri = it->second;
  // 			aig.create_ri(ri, reset, name);

  // 		}

  // 		else{
  // 			if (signal.data & 1) {
  // 				auto temp = signal.data - 1;
  // 			}

  // 			auto nodeIn = signal.data / 2;
  // 			if (element._storage->nodes[nodeIn].children[0].data == 1) {
  // 				comp = true;
  // 			}

  // 			auto ri = mockturtle::aig_network::signal(signal);

  // 			if (comp) {
  // 				ri = aig.create_not(ri);
  // 			}

  // 			aig.create_ri(ri, reset, name);
  // 		}
  // 	}
  //   return aig;
  // }
} // namespace alice

/* Main method for the Alice shell (with prefix) */
ALICE_MAIN( lsoracle )
