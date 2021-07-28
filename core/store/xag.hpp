#pragma once

#include <memory>

#include <alice/alice.hpp>
#include <lorina/aiger.hpp>
#include <lorina/blif.hpp>
#include <lorina/verilog.hpp>
#include <mockturtle/mockturtle.hpp>

#include <fmt/format.h>

namespace alice
{
  using xag_names = mockturtle::names_view<mockturtle::xag_network>;
  using xag_ntk = std::shared_ptr<xag_names>;

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
}