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
class disjoint_clouds_command : public alice::command
{

public:
    explicit disjoint_clouds_command(const environment::ptr &env)
        : command(env,
                  "Writes the status for each disjoint combinational cloud on the current AIG")
    {

        opts.add_option("--filename,filename", filename,
                        "File to write to")->required();
    }

protected:
    void execute()
    {


        if (!store<aig_ntk>().empty()) {
            auto &aig = *store<aig_ntk>().current();
            std::ofstream dset;
            dset.open(filename);

            //maps to keep the depth and numb of nodes for each comb cloud
            std::unordered_map<int, int> outputs;
            std::unordered_map<int, int> depth;
            std::unordered_map<int, int> number_nodes;

            //copy sets in a local vector
            std::vector<int> clouds;

            //number of aig inputs + 1 to start at the nodes index
            uint64_t inSize = aig.num_pis() + 1;

            //union find constructor - inSize sets the vector index where and nodes start
            UnionFind uf(aig.size() - 1, inSize);

            //set all nodes as not visited for the DFS
            aig.foreach_node([&](auto node) {
                aig._storage->nodes[node].data[1].h1 = 0;
            });

            //first looking to latches
            aig.foreach_ri([&](auto ri) {

                auto indata = ri.data;

                if (indata & 1)
                    indata = indata - 1;

                //calculate latch input node index
                auto inIndex = indata >> 1;

                if (inIndex > aig.num_pis()) {
                    oracle::dfs(aig, inIndex, uf);
                }
            });

            aig.foreach_node([&](auto node) {
                //set all nodes as not visited
                aig._storage->nodes[node].data[1].h1 = 0;
            });

            //then considering POs
            for (int i = 0; i < aig.num_pos() - aig.num_latches(); i++) {
                auto inIdx = aig._storage->outputs[i].data;
                if (aig._storage->outputs[i].data & 1)
                    inIdx = aig._storage->outputs[i].data - 1;
                //calculate the index of the node driving the output
                inIdx = inIdx >> 1;
                if (inIdx > aig.num_pis()) {
                    oracle::dfs(aig, inIdx, uf);
                }
            }

            uf.get_sets(clouds);

            for (int j = 0; j < clouds.size() ; ++j) {
                auto it = number_nodes.find(clouds[j]);
                //element not in the table yet
                if (it == number_nodes.end()) {
                    int count = std::count(std::begin(clouds), std::end(clouds), clouds[j]);
                    number_nodes.insert(std::make_pair(clouds[j], count));
                }
            }

            for (std::pair<int, int> element : number_nodes) {
                dset << element.second << std::endl;
            }

            env->out() << "Number of disjoint graphs: " << uf.getNumbOfSets() << std::endl;

            dset.close();
        } else {
            env->err() << "There is not an AIG network stored.\n";
        }
    }
private:
    std::string filename{};
};

ALICE_ADD_COMMAND(disjoint_clouds, "Output");
}
