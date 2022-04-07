/* LSOracle: A learning based Oracle for Logic Synthesis

 * MIT License
 * Copyright 2019 Laboratory for Nano Integrated Systems (LNIS)
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */
#include <kitty/kitty.hpp>
#include <mockturtle/mockturtle.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <fmt/format.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <regex>

namespace oracle
{
class parser
{
public:
    mockturtle::aig_network run_aig(mockturtle::aig_network &aig, std::string script)
    {
        /* Open script file from FlowTune */
        std::ifstream script_file; 
        std::string line; 
        std::string final = script + ".script";
        script_file.open(final);
        bool dbg = true; 

     //   if ( dbg ) { 
     //     printf("Opened file for debugging! File name %s\n", script );
     //   }

        /* Let's just use the first recommendation */
        int line_count = 0; 
        /* Ensure the file was properly opened */
        if ( script_file.is_open() ) 
        {
          while ( script_file )
          {
            if ( line_count >= 1 )
              break; 
            
            std::getline( script_file, line );
       //     printf("line: %s\n", line.c_str()); 
            std::string delimiter = ";";
            line_count++;
            size_t pos = 0;
            std::string token;

            bool is_rw = false; 
            bool is_rf = false; 
            bool is_rs = false; 

            bool is_d = false;
            bool is_k = false; 
            bool is_s = false; 
            bool is_z = false; 
            int cut_size = 0;

            while ((pos = line.find(delimiter)) != std::string::npos) {
                token = line.substr(0, pos);
                std::string delimiter_word = " ";
         //       printf("token1: %s\n", token.c_str()); 
                
                size_t pos_word = 0;
                std::string token_word;
                std::regex digit("[0-9]");
                while ((pos_word = token.find(delimiter_word)) != std::string::npos) {
                    token_word = token.substr(0, pos_word);
                    
                    if ( token_word != "read" && token_word != "-a" && token_word != "ps")
                    {
                      printf("word:\n");
                      printf("%s\n", token_word.c_str());
                      if ( token_word == "rw" ) is_rw = true; 
                      if ( token_word == "refactor" ) is_rf = true;
                      if ( token_word == "resub" ) is_rs = true;  
                      if ( token_word == "-d" ) is_d = true;  
                      if ( token_word == "-k" ) is_k = true; 
                      if ( token_word == "-s" ) is_s = true;
                      if ( token_word == "-z" ) is_z = true;
                      //printf("c1_0! token word %s! var %d\n", token_word.c_str(), is_rw);
                      if ( std::regex_match(token_word, digit) )
                      {
                        cut_size = std::atoi( token_word.c_str() );
                      }
                      // already read all the info, time to apply the transformation 
                      if ( token_word == " "  ||  token_word == "" )
                      {
                        //printf("c1! token word %s! var %d\n", token_word.c_str(), is_rw);
                        //getchar();
                        if ( is_rw )
                        {
           //               printf("applying rw\n");
                          mockturtle::xag_npn_resynthesis<mockturtle::aig_network> resyn;
                          mockturtle::cut_rewriting_params ps;

                          if ( is_z ) ps.allow_zero_gain = true;
                          if ( cut_size == 0 ) cut_size = 4; 
                          ps.cut_enumeration_ps.cut_size = cut_size;
                          mockturtle::cut_rewriting_with_compatibility_graph( aig, resyn, ps );
                          aig = mockturtle::cleanup_dangling(aig);

                          is_z = false; 
                          is_rw = false; 
                          cut_size = 0;
                          std::cout << "Post-rw size = " << aig.num_gates() << std::endl;
                        }
                        if ( is_rf )
                        {
             //             printf("applying rf\n");
                          mockturtle::refactoring_params ps;
                          if ( is_z ) ps.allow_zero_gain = true;
                          if ( is_d ) ps.use_dont_cares = true;
                          mockturtle::xag_npn_resynthesis<mockturtle::aig_network> resyn;
                          if ( cut_size == 0 ) cut_size = 4; 
                          ps.max_pis = cut_size;
                          mockturtle::refactoring(aig, resyn, ps);
                          aig = mockturtle::cleanup_dangling(aig);
                          is_rf = false; 
                          is_z = false; 
                          is_d = false; 
                          cut_size = 0;
               //           std::cout << "Post-rf size = " << aig.num_gates() << std::endl;
                        }
                        if ( is_rs )
                        {
                 //         printf("applying rs\n");
                          mockturtle::resubstitution_params ps;
                          mockturtle::resubstitution_stats st;

                          if ( cut_size == 0 ) cut_size = 4; 
                          ps.progress = false; 
                          ps.max_pis = cut_size;

                          if ( is_s ) 
                          {
                            mockturtle::sim_resubstitution( aig, ps, &st );
                          } else {
                            mockturtle::aig_resubstitution( aig, ps, &st );
                          }
                          aig = mockturtle::cleanup_dangling( aig );
                          cut_size = 0;
                          is_s = false; 
                          is_rs = false; 
                   //       std::cout << "Post-rs size = " << aig.num_gates() << std::endl;
                        }
                      }
                    }                   
                    token.erase(0, pos_word + delimiter_word.length());
                }
                line.erase(0, pos + delimiter.length());
            }
          }
        }
        //std::cout << "Final network size = " << aig.num_gates() << std::endl;
        return aig; 
        // mockturtle::depth_view mig_depth{mig};

        // mockturtle::mig_algebraic_depth_rewriting_params pm;
        // //pm.selective;

        // // std::cout << "1st round depth optimization " << std::endl;

        // mockturtle::mig_algebraic_depth_rewriting(mig_depth, pm);

        // mig = mockturtle::cleanup_dangling(mig);

        // // std::cout << "1st round area recovering " << std::endl;

        // // AREA RECOVERING
        // mockturtle::mig_npn_resynthesis resyn;
        // mockturtle::cut_rewriting_params ps;

        // ps.cut_enumeration_ps.cut_size = 4;

        // mockturtle::cut_rewriting(mig, resyn, ps);
        // mig = mockturtle::cleanup_dangling(mig);

        // // std::cout << "2nd round area recovering " << std::endl;

        // // AREA RECOVERING
        // mockturtle::cut_rewriting(mig, resyn, ps);
        // mig = mockturtle::cleanup_dangling(mig);

        // // std::cout << "2nd round depth optimization" << std::endl;

        // //DEPTH REWRITING
        // mockturtle::depth_view mig_depth1{mig};

        // mockturtle::mig_algebraic_depth_rewriting(mig_depth1, pm);
        // mig = mockturtle::cleanup_dangling(mig);

        // // std::cout << "3rd round area recovering" << std::endl;

        // // AREA RECOVERING
        // mockturtle::cut_rewriting(mig, resyn, ps);
        // mig = mockturtle::cleanup_dangling(mig);

        // // std::cout << "4th round area recovering" << std::endl;

        // // AREA RECOVERING
        // mockturtle::cut_rewriting(mig, resyn, ps);
        // mig = mockturtle::cleanup_dangling(mig);

        // // std::cout << "3rd round depth optimization" << std::endl;

        // //DEPTH REWRITING
        // mockturtle::depth_view mig_depth2{mig};

        // mockturtle::mig_algebraic_depth_rewriting(mig_depth2, pm);
        // mig = mockturtle::cleanup_dangling(mig);

        // // std::cout << "5th round area recovering" << std::endl;

        // // AREA RECOVERING
        // mockturtle::cut_rewriting(mig, resyn, ps);
        // mig = mockturtle::cleanup_dangling(mig);

        // // std::cout << "6th round area recovering" << std::endl;

        // // AREA RECOVERING
        // mockturtle::cut_rewriting(mig, resyn, ps);
        // mig = mockturtle::cleanup_dangling(mig);

        // // std::cout << "Final depth optimization" << std::endl;

        // //DEPTH REWRITING
        // mockturtle::depth_view mig_depth3{mig};

        // // std::cout << "Network Optimized" << std::endl;

        // mockturtle::mig_algebraic_depth_rewriting(mig_depth3, pm);
        // mig = mockturtle::cleanup_dangling(mig);

        // // std::cout << "Majority nodes " << mig.num_gates() << " MIG depth " << mig_depth3.depth() << std::endl;

        // return mig;
    }
    mockturtle::aig_network run_mig(mockturtle::aig_network &aig, std::string script)
    {
        
        return aig;
    }
};
}
