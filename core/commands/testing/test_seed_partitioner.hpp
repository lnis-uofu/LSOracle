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
#include <alice/alice.hpp>

#include <mockturtle/mockturtle.hpp>

#include <stdio.h>
#include <fstream>

#include <sys/stat.h>
#include <stdlib.h>


namespace alice
{

class seed_partitioning_command : public alice::command{

  public:
    explicit seed_partitioning_command( const environment::ptr& env )
            : command( env, "Runs AIG optimization on entire circuit before partitioning and determining best MIG partitions" ){

            opts.add_option( "--num_pis,-p", num_pis, "Number of PIs constraint" )->required();
            opts.add_option( "--num_int,-i", num_int, "Number of internal nodes constraint" )->required();
            add_flag("--mig,-m", "Use seed partitioning on stored MIG network (AIG is default)");
    }

  protected:
    void execute(){
      if(is_set("mig")){
        if(!store<mockturtle::mig_network>().empty()){
          auto ntk = store<mockturtle::mig_network>().current();
          oracle::seed_partitioner<mockturtle::mig_network> seed_parts(ntk, num_pis, num_int);
          oracle::partition_manager<mockturtle::mig_network> part_man = seed_parts.create_part_man(ntk);
          store<oracle::partition_manager<mockturtle::mig_network>>().extend() = part_man;
        }
        else{
          env->err() << "MIG network not stored\n";
        }
      }
      else{
        if(!store<mockturtle::aig_network>().empty()){
          auto ntk = store<mockturtle::aig_network>().current();
          oracle::seed_partitioner<mockturtle::aig_network> seed_parts(ntk, num_pis, num_int);
          env->out() << "network partitioned\n";
          oracle::partition_manager<mockturtle::aig_network> part_man = seed_parts.create_part_man(ntk);
          store<oracle::partition_manager<mockturtle::aig_network>>().extend() = part_man;
        }
        else{
          env->err() << "AIG network not stored\n";
        }
      }
    }

  private:
    int num_pis = 0;
    int num_int = 0;
  };

  ALICE_ADD_COMMAND(seed_partitioning, "Testing");


}
