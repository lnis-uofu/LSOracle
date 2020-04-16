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

namespace oracle{

  using aig_names = mockturtle::names_view<mockturtle::aig_network>;
  using aig_ntk = std::shared_ptr<aig_names>;

  using mig_names = mockturtle::names_view<mockturtle::mig_network>;
  using mig_ntk = std::shared_ptr<mig_names>;
  
  using lut_names = mockturtle::names_view<mockturtle::klut_network>;
  using lut_ntk = std::shared_ptr <lut_names>;

  lut_names abc_optimization_test(aig_names ntk_aig) {

    mockturtle::direct_resynthesis<mockturtle::mig_network> resyn_mig;
    mockturtle::direct_resynthesis<mockturtle::aig_network> resyn_aig;

    lut_names ntk_lut;
    auto ntk_mig = *aig_to_mig(ntk_aig, 1); //convert aig to mig; final network in original version of oracle was MIG

    //mixed synthesis
    std::cout << "Performing High Effort Classification and Optimization\n";

    mockturtle::abc_view abc;
    abc.to_abc(ntk_aig);
    abc.dc2();
    mockturtle::lut_mapping_params ps;
    ps.cut_enumeration_ps.cut_size = 6;
    ps.cut_enumeration_ps.cut_limit = 6;
    mockturtle::abc_lut_mapping(abc, ps);
    const auto klut_aig = mockturtle::collapse_abc_lut<mockturtle::klut_network>(abc);
    mockturtle::names_view<mockturtle::klut_network> aignames_view{klut_aig};
    mockturtle::depth_view aigklut_depth{aignames_view};
     
      

    //auto ntk_mig = oracle::aig_to_mig(ntk_aig, 1);
    oracle::mig_script migopt;
    ntk_mig = migopt.run(ntk_mig);
    mockturtle::mapping_view<mockturtle::mig_network, true> mapped{ntk_mig};
    mockturtle::lut_mapping<mockturtle::mapping_view<mockturtle::mig_network, true>, true>( mapped, ps );
    std::cout << "number of cells = " << mapped.num_cells() << "\n";
    const auto klut_mig = mockturtle::collapse_mapped_network<mockturtle::klut_network>( mapped );
    mockturtle::names_view<mockturtle::klut_network> names_view{*klut_mig};
    mockturtle::depth_view migklut_depth{names_view};
    std::cout << "MIG LUT = " << mapped.num_cells() << " lev = " << migklut_depth.depth() << "\n";

    if (migklut_depth.depth() <= aigklut_depth.depth()){
      std::cout << "returning MIG\n";
      return names_view;
    }
    
    std::cout << "returning AIG\n";
    return aignames_view;
  }

} //namespace oracle