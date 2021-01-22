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
  using mig_names = mockturtle::names_view<mockturtle::mig_network>;
  using mig_ntk = std::shared_ptr<mig_names>;
  using part_man_mig = oracle::partition_manager<mig_names>;
  using part_man_mig_ntk = std::shared_ptr<part_man_mig>;

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
}