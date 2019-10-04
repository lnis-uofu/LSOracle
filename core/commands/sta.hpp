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
  //object to hold STA configuration
  STA sta_cfg;
  
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
}