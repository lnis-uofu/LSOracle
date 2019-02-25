/******************************************
 *  Source file: gts.cpp                  *
 *  Description: Functions for gts class  *
 *  Author:      Xifan Tang               *
 *  Affiliation: University of Utah, LNIS *
 *  Date:        Feb. 19, 2019            *
 *  Revision:    Feb. 19, 2019            *
 ******************************************
 */
#include <iostream>
#include <fstream>

#include "gts.hpp"

namespace gts;

/* Functions to read benchmark file list */
std::vector<string> read_benchmark_list(std::string list_name) {
  /* Create file handle */
  std::ifstream ifs;
  std::string line;
  std::string content;
  vector<string> file_names; 
  /* Open file */
  ifs.open(list_name, std::ifstream::in);
  /* Get lines and tokenize */ 
  while (std::getline(ifs, line, '#')) {
    /* Get rid of comments */
    std::getline(line, content, ',');  
    /* Check each token */   
    for (std::string::iterator it = content.begin();
         it != content.end();
         it++) {
      file_names.push_back(*it);
      /* We only care the first element now */
      break;
    }
  }
  /* close file */
  ifs.close();

  return file_names;
}

bool file_exist (const std::string& name) {
    return ( access( name.c_str(), F_OK ) != -1 );
}

/* A generic file reader for network files
 * The file type will be recognized by the postfix
 * .v, .blif, .aig, etc.
 * Depending on the identifed file type, different file reader will be launched 
 */
void read_network_file (std::string input_file) {
  /* Check existence */
  if (TRUE == std::regex_match(input_file, std::regex("\.blif$"))) {
    /* Call blif reader */
    mockturtle::klut_network blif;
    lorina::read_blif(input_file, mockturtle::blif_reader( blif ));
    /* Store the network in a container */
    alice::store<mockturtle::klut_network>().extend() = blif;
  } else if (TRUE == std::regex_match(input_file, std::regex("\.bench$"))) {
    /* Call bench reader */
    mockturtle::klut_network bench;
    lorina::read_bench(input_file, mockturtle::bench_reader( bench ));
    /* Store the network in a container */
    alice::store<mockturtle::klut_network>().extend() = bench;
  } else if (TRUE == std::regex_match(input_file, std::regex("\.aig$"))) {
    /* Call aig reader */
    mockturtle::aig_network aig;
    lorina::read_aiger(input_file, mockturtle::aiger_reader( aig ));
    /* Store the network in a container */
    alice::store<mockturtle::aig_network>().extend() = aig;
  } else if (TRUE == std::regex_match(input_file, std::regex("\.mig$"))) {
    /* Call mig reader */
    mockturtle::mig_network mig;
    lorina::read_aiger(input_file, mockturtle::aiger_reader( mig ));
    /* Store the network in a container */
    alice::store<mockturtle::mig_network>().extend() = mig;
  } else {
    /* unknown file type, error out */
    std::cout << "Error: Unable to recognize the file type of " << input_file << "!\n";
    std::cout << "Supported files types are [.blif|.bench|.aig]\n";
    exit(GTS_FAIL);
  }

  return store;
}

/* Main function */
void run_gts (x_gts_opts gts_opts) {
  /* Read the benchmark file list*/
  std::vector<string> filenames = read_benchmark_list(gts_opts.get_benchmark_list()); 
  /* For each benchmark file:
   * 1. Check existence
   * 2. Identify the file type 
   * 3. Read into associated store of logic networks
   * 4. Partition with a given input_size  
   */
  for (int ifile = 0; ifile < filenames.size(); ifile++) {
    /* Check existence */  
    if (FALSE == file_exist(filenames[ifile])) {
      /* error out */
      std::cout << "Error: File (" << filenames[ifile] << ") does not exist!\n";
      exit(GTS_FAIL);
    }
  }
  /* Identify file type and get the logic network */
  for (int ifile = 0; ifile < filenames.size(); ifile++) {
    read_network_file(filenames[ifile]);
    if (TRUE == alice::store<mockturtle::aig_network>().empty()) {
      /* error out */
      std::cout << "Error: No logic network read from (" << filenames[ifile] << ")!\n";
      exit(GTS_FAIL);
    }
    /* Run partitioner */
    oracle::partition_manager<mockturtle::mig_network> partitions_aig(ntk, num_parts);
    /* For each paritition:
     * 1. Run optimizations - according to label_[aig|mig]
     * 2. Create labels if logic-level option is enabled 
     * 3. Output into Verilog format 
     * 4. Generate scripts to run tech-mapping  
     * 5. Generate scripts to run P&R flow 
     * 6. Output kaunaurghn-map images 
     */
    for (int ipart = 0; ipart < num_parts; ipart++) {
      oracle::partition_view<mockturtle::mig_network> part_mig = partitions_aig.create_part(ntk, part_mig_opt.at(ipart));
      /* Run AIG optimization */
      auto aig_opt = mockturtle::node_resynthesis<mockturtle::aig_network>( part_aig, resyn_aig );
      mockturtle::aig_script aigopt;
      aig_opt = migopt.run(aig_opt);
      mockturtle::depth_view opt_aig_depth{aig_opt};
      /* Run MIG optimization */
      auto mig_opt = mockturtle::node_resynthesis<mockturtle::mig_network>( part_mig, resyn_mig );
      mockturtle::mig_script migopt;
      mig_opt = migopt.run(mig_opt);
      mockturtle::depth_view opt_mig_depth{mig_opt};
      /* Compare the depth and output labels */
      if ( opt_mig_depth.depth() > opt_aig_depth.depth() ) {
        /* Output a label */
      } else { 
        /* Output a label */
      }
      /* Get the logic cones */
    } 
  }  

  return;
}


