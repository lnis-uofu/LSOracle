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
  ALICE_ADD_STORE( mockturtle::xmg_network, "xmg", "x", "xmg", "XMGs" )

    /* Implements the short string to describe a store element in store -a */
    ALICE_DESCRIBE_STORE( mockturtle::xmg_network, aig ){

        const auto name = "aig_placeholder";
        const auto pi_num = aig.num_pis();
        const auto po_num = aig.num_pos();
        return fmt::format( "{} i/o = {}/{}", name, pi_num, po_num );
    }//end aig_network describe store

    ALICE_LOG_STORE_STATISTICS( mockturtle::xmg_network, aig){

        return {
                {"nodes", aig.size()},
                {"inputs", aig.num_pis() },
                {"latches", aig.num_registers()},
                {"outputs", aig.num_pos() },
                {"AIG nodes", aig.num_gates()}};
    }//end aig_network log store statistics

    /* Implements the functionality of ps -b */
    ALICE_PRINT_STORE_STATISTICS( mockturtle::xmg_network, os, aig ){
        os << "nodes: " << aig.size() << std::endl;
        os << "inputs: " << aig.num_pis()  << std::endl;
        os << "latches: " << aig.num_registers() << std::endl;
        os << "outputs: " << aig.num_pos() << std::endl;
        os << "AIG nodes: " << aig.num_gates() << std::endl;

    }//end aig_network print store statistics
}
