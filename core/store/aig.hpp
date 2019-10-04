#include <memory>

#include <alice/alice.hpp>
#include <lorina/aiger.hpp>
#include <lorina/blif.hpp>
#include <lorina/verilog.hpp>
#include <mockturtle/io/aiger_reader.hpp>
#include <mockturtle/io/blif_reader.hpp>
#include <mockturtle/io/verilog_reader.hpp>
#include <mockturtle/io/write_bench.hpp>
#include <mockturtle/io/write_blif.hpp>
#include <mockturtle/io/write_verilog.hpp>
#include <mockturtle/networks/aig.hpp>
#include <mockturtle/views/depth_view.hpp>
#include <mockturtle/views/mapping_view.hpp>
#include <mockturtle/views/names_view.hpp>

#include <fmt/format.h>

namespace alice
{
  using aig_names = mockturtle::names_view<mockturtle::aig_network>;
  using aig_ntk = std::shared_ptr<aig_names>;
  using part_man_aig = oracle::partition_manager<aig_names>;
  using part_man_aig_ntk = std::shared_ptr<part_man_aig>;

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

  ALICE_PRINT_STORE_STATISTICS( aig_ntk, os, aig ){
    mockturtle::depth_view depth{*aig};
  	os << "nodes: " << aig->size() << std::endl;
  	os << "inputs: " << aig->num_pis() - aig->num_latches() << std::endl;
  	os << "latches: " << aig->num_latches() << std::endl;
  	os << "outputs: " << aig->num_pos() - aig->num_latches() << std::endl;
  	os << "AIG nodes: " << aig->num_gates() << std::endl;
    os << "AIG level: " << depth.depth() << std::endl;

  }

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
}