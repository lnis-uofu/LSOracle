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
  using xmg_names = mockturtle::names_view<mockturtle::xmg_network>;
  using xmg_ntk = std::shared_ptr<xmg_names>;
  using part_man_xmg = oracle::partition_manager<xmg_names>;
  using part_man_xmg_ntk = std::shared_ptr<part_man_xmg>;


  ALICE_ADD_STORE( xmg_ntk, "xmg", "a", "xmg", "XMGs" )

  ALICE_DESCRIBE_STORE( xmg_ntk, xmg ){

    return fmt::format( "i/o = {}/{} gates = {}", xmg->num_pis(), xmg->num_pos(), xmg->num_gates() );
  }

  ALICE_LOG_STORE_STATISTICS( xmg_ntk, xmg){

    return {
      {"nodes", xmg->size()},
      {"inputs", xmg->num_pis() },
      {"outputs", xmg->num_pos() },
      {"XMG nodes", xmg->num_gates()}};
  }//end xmg_network log store statistics

  /* Implements the functionality of ps -b */
  ALICE_PRINT_STORE_STATISTICS( xmg_ntk, os, xmg ){
    os << "nodes: " << xmg->size() << std::endl;
    os << "inputs: " << xmg->num_pis()  << std::endl;
    os << "outputs: " << xmg->num_pos() << std::endl;
    os << "XMG nodes: " << xmg->num_gates() << std::endl;
  }//end xmg_network print store statistics
}
