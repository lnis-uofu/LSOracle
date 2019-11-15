#include <memory>

#include <alice/alice.hpp>
#include <lorina/bench.hpp>
#include <mockturtle/io/bench_reader.hpp>
#include <lorina/blif.hpp>
#include <mockturtle/io/blif_reader.hpp>
#include <mockturtle/networks/klut.hpp>
#include <mockturtle/views/depth_view.hpp>
#include <mockturtle/views/names_view.hpp>

#include <fmt/format.h>

namespace alice
{
  using klut_names = mockturtle::names_view<mockturtle::klut_network>;
  using klut_ntk = std::shared_ptr<klut_names>;

  ALICE_ADD_STORE( klut_ntk, "klut", "k", "klut", "kluts" )

  ALICE_DESCRIBE_STORE( klut_ntk, klut ){

    return fmt::format( "i/o = {}/{} gates = {}", klut->num_pis(), klut->num_pos(), klut->num_gates() );
  }

  ALICE_LOG_STORE_STATISTICS( klut_ntk, klut){
    mockturtle::depth_view depth{*klut};
    return {
    	{"nodes", klut->size()},
    	{"inputs", klut->num_pis() - klut->num_latches()},
    	{"latches", klut->num_latches()},
    	{"outputs", klut->num_pos() - klut->num_latches()},
    	{"klut nodes", klut->num_gates()},
      {"klut level", depth.depth()}};
  }

  ALICE_PRINT_STORE_STATISTICS( klut_ntk, os, klut ){
    mockturtle::depth_view depth{*klut};
  	os << "nodes: " << klut->size() << std::endl;
  	os << "inputs: " << klut->num_pis() - klut->num_latches() << std::endl;
  	os << "latches: " << klut->num_latches() << std::endl;
  	os << "outputs: " << klut->num_pos() - klut->num_latches() << std::endl;
  	os << "klut nodes: " << klut->num_gates() << std::endl;
    os << "klut level: " << depth.depth() << std::endl;

  }

}