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
#include "algorithms/partitioning/partition_view.hpp"
#include "algorithms/partitioning/partition_manager.hpp"

#include "union_find.hpp"

#include <stdio.h>
#include <fstream>

#include <sys/stat.h>
#include <stdlib.h>

namespace oracle
{
using aig_names = mockturtle::names_view<mockturtle::aig_network>;
using aig_ntk = std::shared_ptr<aig_names>;

using mig_names = mockturtle::names_view<mockturtle::mig_network>;
using mig_ntk = std::shared_ptr<mig_names>;

using xag_names = mockturtle::names_view<mockturtle::xag_network>;
using xag_ntk = std::shared_ptr<xag_names>;

using xmg_names = mockturtle::names_view<mockturtle::xmg_network>;
using xmg_ntk = std::shared_ptr<xmg_names>;

template<typename Ntk>
int computeLevel(Ntk const &ntk, int index)
{
    //if node not visited
    if (ntk._storage->nodes[index].data[1].h1 == 0) {

        //set node as visited
        ntk._storage->nodes[index].data[1].h1 = 1;

        //if is input
        if (ntk.is_ci(index)) {
            return 0;
        }

        auto inIdx2 = ntk._storage->nodes[index].children[1].data;
        if (inIdx2 & 1)
            inIdx2 = inIdx2 - 1;

        //calculate input node index
        auto inNode1 = inIdx2 >> 1;
        int levelNode1 = computeLevel(ntk, inNode1);

        auto inIdx = ntk._storage->nodes[index].children[0].data;
        if (inIdx & 1)
            inIdx = inIdx - 1;

        //calculate input node index
        auto inNode0 = inIdx >> 1;
        int levelNode0 = computeLevel(ntk, inNode0);

        int level = 1 + std::max(levelNode0, levelNode1);
        return level;
    }
    return 0;
}

template int computeLevel<mockturtle::names_view<mockturtle::aig_network>>
(mockturtle::names_view<mockturtle::aig_network> const &, int);

template<typename Ntk>
int computeLevel(Ntk const &ntk, typename Ntk::node curr_node,
                 std::set<typename Ntk::node> partition_inputs)
{
    //if node not visited
    if (ntk._storage->nodes[curr_node].data[1].h1 == 0
            && !ntk.is_constant(curr_node))  {
        //set node as visited
        ntk._storage->nodes[curr_node].data[1].h1 = 1;
        //if is input
        if (partition_inputs.find(curr_node) != partition_inputs.end()) {
            return 0;
        }

        auto inIdx2 = ntk._storage->nodes[curr_node].children[1].data;

        if (inIdx2 & 1)
            inIdx2 = inIdx2 - 1;

        //calculate input node index
        auto inNode1 = inIdx2 >> 1;
        int levelNode1 = computeLevel(ntk, inNode1, partition_inputs);

        auto inIdx = ntk._storage->nodes[curr_node].children[0].data;
        if (inIdx & 1)
            inIdx = inIdx - 1;

        //calculate input node index
        auto inNode0 = inIdx >> 1;
        int levelNode0 = computeLevel(ntk, inNode0, partition_inputs);

        int level = 1 + std::max(levelNode0, levelNode1);
        return level;
    }
    return 0;
}

void dfs(mockturtle::aig_network aig, uint64_t index, UnionFind uf)
{
    if (aig._storage->nodes[index].data[1].h1 == 0) {
        //set node as visited
        aig._storage->nodes[index].data[1].h1 = 1;

        //traverse the left nodes to the PIs
        if (!aig.is_pi(aig._storage->nodes[index].children[0].index)
                && index > aig.num_pis()) {
            auto inData0 = aig._storage->nodes[index].children[0].data;
            //treat inverted and inputs
            if (inData0 & 1)
                inData0 = aig._storage->nodes[index].children[0].data - 1;

            //calculate input node index
            auto inIndex = inData0 >> 1;

            //ignores latches and PIs
            if (inIndex > aig.num_pis()) {
                uf.merge(index, inIndex);
                dfs(aig, inIndex, uf);
            }
        }

        //traverse the right nodes to the PIs
        if (!aig.is_pi(aig._storage->nodes[index].children[1].index)
                && index > aig.num_pis()) {
            auto inData1 = aig._storage->nodes[index].children[1].data;
            if (inData1 & 1)
                inData1 = aig._storage->nodes[index].children[1].data - 1;

            //calculate input node index
            auto inIndex = inData1 >> 1;

            //im ignoring latches
            if (inIndex > aig.num_pis()) {
                uf.merge(index, inIndex);
                dfs(aig, inIndex, uf);
            }
        }
    }
}

void compute_cone(mockturtle::aig_network aig, uint64_t index,
                  std::unordered_map<int, int> &nodes, int outindex,
                  std::unordered_map<int, int> &ins)
{
    if (aig._storage->nodes[index].data[1].h1 == 0) {

        //increment number of nodes in this cone
        std::unordered_map<int, int>::iterator it = nodes.find(outindex);

        if (it != nodes.end() && index > aig.num_pis()) {
            //increment the number of nodes
            it->second++;
        }

        //set node as visited
        aig._storage->nodes[index].data[1].h1 = 1;

        //traverse one side to the PIs
        if (!aig.is_pi(aig._storage->nodes[index].children[0].index)
                && index > aig.num_pis()) {
            if (aig._storage->nodes[index].children[0].data & 1)
                aig._storage->nodes[index].children[0].data =
                    aig._storage->nodes[index].children[0].data - 1;

            //calculate input node index
            auto inIndex = aig._storage->nodes[index].children[0].data >> 1;

            //im ignoring latches
            if (inIndex > aig.num_pis()) {
                //call recursion
                compute_cone(aig, inIndex, nodes, outindex, ins);
            }
        }

        //traverse the other side to the PIs
        if (!aig.is_pi(aig._storage->nodes[index].children[1].index)
                && index > aig.num_pis()) {
            if (aig._storage->nodes[index].children[1].data & 1)
                aig._storage->nodes[index].children[1].data =
                    aig._storage->nodes[index].children[1].data - 1;

            //calculate input node index
            auto inIndex = aig._storage->nodes[index].children[1].data >> 1;

            //im ignoring latches
            if (inIndex > aig.num_pis()) {
                //call recursion
                compute_cone(aig, inIndex, nodes, outindex, ins);
            }
        }

        //if my child is PI and was not visited yet, I increase the input counter
        if (aig.is_ci(aig._storage->nodes[index].children[0].index)
                && aig._storage->nodes[aig._storage->nodes[index].children[0].index].data[1].h1
                == 0) {
            aig._storage->nodes[aig._storage->nodes[index].children[0].index].data[1].h1 =
                1;

            std::unordered_map<int, int>::iterator it = ins.find(outindex);
            if (it != ins.end()) {
                //increment the number of inputs
                it->second++;
            }
        }

        //if my other child is PI and was not visited yet, I also increase the input counter
        if (aig.is_ci(aig._storage->nodes[index].children[1].index)
                && aig._storage->nodes[aig._storage->nodes[index].children[1].index].data[1].h1
                == 0) {
            aig._storage->nodes[aig._storage->nodes[index].children[1].index].data[1].h1 =
                1;

            std::unordered_map<int, int>::iterator it = ins.find(outindex);
            if (it != ins.end()) {
                //increment the number of inputs
                it->second++;
            }
        }
    }
}

/***************************************************
  Truth Table
***************************************************/

// Helper function to flip the bit
char flip(char c)
{
    return (c == '0') ? '1' : '0';
}

std::string to_binary(int dec)
{

    std::string bin;
    while (dec != 0) {
        bin = (dec % 2 == 0 ? "0" : "1") + bin;
        dec /= 2;
    }
    return bin;
}

std::string graytoBinary(std::string gray)
{
    std::string binary = "";

    binary += gray[0];

    for (int i = 1; i < gray.length(); i++) {
        if (gray[i] == '0')
            binary += binary[i - 1];
        else
            binary += flip(binary[i - 1]);
    }

    return binary;
}

// Function to convert binary to decimal
int binaryToDecimal(int n)
{

    int num = n;
    int dec_value = 0;

    // Initializing base value to 1, i.e 2^0
    int base = 1;

    int temp = num;
    while (temp) {

        int last_digit = temp % 10;
        temp = temp / 10;

        dec_value += last_digit * base;

        base = base * 2;
    }

    return dec_value;
}

/***************************************************/

/***************************************************
  Network conversion
***************************************************/
mig_ntk aig_to_mig(aig_names aig, int skip_edge_min)
{

    using NtkSource = aig_names;
    using NtkDest = mig_names;
    mockturtle::mig_network ntk;
    NtkDest mig(ntk);

    mockturtle::node_map<mockturtle::mig_network::signal, mockturtle::aig_network>
    node2new(aig);

    node2new[aig.get_node(aig.get_constant(false))] = mig.get_constant(false);
    if (aig.get_node(aig.get_constant(true)) != aig.get_node(aig.get_constant(
                false))) {

        node2new[aig.get_node(aig.get_constant(true))] = mig.get_constant(true);
    }

    aig.foreach_pi([&](auto n) {
        if (aig.is_ro(n)) {
            mig._storage->data.latches.emplace_back(0);
        }
        node2new[n] = mig.create_pi();

        if constexpr(mockturtle::has_has_name_v<NtkSource>
                     &&mockturtle::has_get_name_v<NtkSource> &&mockturtle::has_set_name_v<NtkDest>) {
            if (aig.has_name(aig.make_signal(n)))
                mig.set_name(node2new[n], aig.get_name(aig.make_signal(n)));
        }
    });

    aig.foreach_node([&](auto n) {
        if (aig.is_constant(n) || aig.is_pi(n) || aig.is_ci(n) || aig.is_ro(n))
            return;

        std::vector<mockturtle::mig_network::signal> children;
        aig.foreach_fanin(n, [&](auto const & f) {
            children.push_back(aig.is_complemented(f) ? mig.create_not(
                                   node2new[f]) : node2new[f]);
        });

        //removed create_maj_part() call; it just aliased create_maj
        node2new[n] = mig.create_maj(mig.get_constant(false), children.at(0),
                                     children.at(1));

        if constexpr(mockturtle::has_has_name_v<NtkSource>
                     &&mockturtle::has_get_name_v<NtkSource> &&mockturtle::has_set_name_v<NtkDest>) {
            if (aig.has_name(aig.make_signal(n)))
                mig.set_name(node2new[n], aig.get_name(aig.make_signal(n)));
        }

    });

    /* map primary outputs */
    aig.foreach_po([&](auto const & f, auto index) {
        mig.create_po(aig.is_complemented(f) ? mig.create_not(node2new[f]) :
                      node2new[f]);

        if constexpr(mockturtle::has_has_output_name_v<NtkSource>
                     &&mockturtle::has_get_output_name_v<NtkSource>
                     &&mockturtle::has_set_output_name_v<NtkDest>) {
            if (aig.has_output_name(index)) {
                mig.set_output_name(index, aig.get_output_name(index));
            }
        }
    });

    return std::make_shared<mig_names>(mig);
}

mig_ntk part_to_mig(oracle::partition_view<mig_names> part, int skip_edge_min)
{
    mockturtle::mig_network mig;

    std::unordered_map<mockturtle::mig_network::node, mockturtle::mig_network::signal>
    node2new;

    node2new[part.get_node(part.get_constant(false))] = mig.get_constant(false);
    if (part.get_node(part.get_constant(true)) != part.get_node(part.get_constant(
                false))) {

        node2new[part.get_node(part.get_constant(true))] = mig.get_constant(true);
    }

    part.foreach_pi([&](auto n) {
        node2new[n] = mig.create_pi();
    });

    part.foreach_node([&](auto n) {
        if (part.is_constant(n) || part.is_pi(n) || part.is_ci(n) || part.is_ro(n))
            return;

        std::vector<mockturtle::mig_network::signal> children;
        part.foreach_fanin(n, [&](auto const & f) {
            children.push_back(part.is_complemented(f) ? mig.create_not(
                                   node2new[part.get_node(f)]) : node2new[part.get_node(f)]);
        });

        node2new[n] = mig.create_maj(children.at(0), children.at(1),
                                     children.at(2)); //see above re: create_maj_part()
    });

    /* map primary outputs */
    part.foreach_po([&](auto const & f) {
        mig.create_po(part.is_complemented(f) ? mig.create_not(node2new[part.get_node(
                          f)]) : node2new[part.get_node(f)]);
    });

    return std::make_shared<mig_names>(mig);
}

aig_ntk mig_to_aig(mig_names mig)
{
    using NtkSource = mig_names;
    using NtkDest = aig_names;
    mockturtle::aig_network ntk;
    NtkDest aig(ntk);

    mockturtle::node_map<mockturtle::aig_network::signal, mockturtle::mig_network>
    node2new(mig);

    node2new[mig.get_node(mig.get_constant(false))] = aig.get_constant(false);
    if (mig.get_node(mig.get_constant(true)) != mig.get_node(mig.get_constant(
                false))) {

        node2new[mig.get_node(mig.get_constant(true))] = aig.get_constant(true);
    }

    mig.foreach_pi([&](auto n) {
        node2new[n] = aig.create_pi();

        if constexpr(mockturtle::has_has_name_v<NtkSource>
                     &&mockturtle::has_get_name_v<NtkSource> &&mockturtle::has_set_name_v<NtkDest>) {
            if (mig.has_name(mig.make_signal(n)))
                aig.set_name(node2new[n], mig.get_name(mig.make_signal(n)));
        }
    });

    std::set<mockturtle::mig_network::node> nodes_to_change;
    mig.foreach_node([&](auto n) {

        if (mig.is_constant(n) || mig.is_pi(n) || mig.is_ci(n) || mig.is_ro(n))
            return;

        std::vector<mockturtle::aig_network::signal> children;

        if (mig._storage->nodes[n].children[0].data != 0) {
            mockturtle::mig_network::signal child1 = mig._storage->nodes[n].children[1];
            mockturtle::mig_network::signal child2 = mig._storage->nodes[n].children[2];
            children.push_back(mig.is_complemented(child1) ? node2new[child1] :
                               aig.create_not(node2new[child1]));
            children.push_back(mig.is_complemented(child2) ? node2new[child2] :
                               aig.create_not(node2new[child2]));
            nodes_to_change.insert(n);
        } else {
            for (int i = 1; i < mig._storage->nodes[n].children.size(); i++) {
                auto node = mig.get_node(mig._storage->nodes[n].children[i]);
                mockturtle::mig_network::signal child = mig._storage->nodes[n].children[i];
                if (nodes_to_change.find(node) != nodes_to_change.end()) {

                    children.push_back(mig.is_complemented(child) ? node2new[child] :
                                       aig.create_not(node2new[child]));
                } else {

                    children.push_back(mig.is_complemented(child) ? aig.create_not(
                                           node2new[child]) : node2new[child]);
                }
            }
        }
        node2new[n] = aig.create_and(children.at(0), children.at(1));

        if constexpr(mockturtle::has_has_name_v<NtkSource>
                     &&mockturtle::has_get_name_v<NtkSource> &&mockturtle::has_set_name_v<NtkDest>) {
            if (mig.has_name(mig.make_signal(n)))
                aig.set_name(node2new[n], mig.get_name(mig.make_signal(n)));
        }

    });

    /* map primary outputs */
    mig.foreach_po([&](auto const & f, auto index) {
        aig.create_po(mig.is_complemented(f) ? aig.create_not(node2new[f]) :
                      node2new[f]);

        if constexpr(mockturtle::has_has_output_name_v<NtkSource>
                     &&mockturtle::has_get_output_name_v<NtkSource>
                     &&mockturtle::has_set_output_name_v<NtkDest>) {
            if (mig.has_output_name(index)) {
                aig.set_output_name(index, mig.get_output_name(index));
            }
        }
    });

    return std::make_shared<aig_names>(aig);
}

/***************************************************/

/***************************************************
  Network helpers
***************************************************/
template <typename Ntk>
bool is_po(Ntk const &ntk, typename Ntk::node const &n)
{
    int nodeIdx = ntk.node_to_index(n);
    bool result = false;
    for (int i = 0; i < ntk._storage->outputs.size(); i++) {
        if (ntk._storage->outputs.at(i).index == nodeIdx)
            result = true;
    }
    return result;
}

template bool is_po<aig_names>(aig_names const &, aig_names::node const &);
template bool is_po<partition_manager<aig_names>>(partition_manager<aig_names>
        const &, partition_manager<aig_names>::node const &);
template bool is_po<mig_names>(mig_names const &, mig_names::node const &);

using topo_aig =
    mockturtle::topo_view<mockturtle::aig_network>;
template bool is_po<topo_aig>(topo_aig const &, topo_aig::node const &);

using topo_names_aig =
    mockturtle::topo_view<mockturtle::names_view<mockturtle::aig_network>>;
template bool is_po<topo_names_aig>(topo_names_aig const &,
                                    topo_names_aig::node const &);

using xag_names = mockturtle::names_view<mockturtle::xag_network>;
template bool is_po<xag_names>(xag_names const &, xag_names::node const &);

using topo_mig =
    mockturtle::topo_view<mockturtle::names_view<mockturtle::mig_network>>;
template bool is_po<topo_mig>(topo_mig const &, topo_mig::node const &);

using xmg_names = mockturtle::names_view<mockturtle::xmg_network>;
template bool is_po<xmg_names>(xmg_names const &, xmg_names::node const &);

using topo_names_xmg =
    mockturtle::topo_view<mockturtle::names_view<mockturtle::xmg_network>>;
template bool is_po<topo_names_xmg>(topo_names_xmg const &,
                                    topo_names_xmg::node const &);

bool is_in_vector(std::vector<int> vec, int nodeIdx)
{

    if (std::find(vec.begin(), vec.end(), nodeIdx) != vec.end())
        return true;
    else
        return false;
}

template<typename Ntk>
int get_output_index(Ntk const &ntk, int nodeIdx)
{

    assert(ntk.is_po(nodeIdx));

    for (int i = 0; i < ntk._storage->outputs.size(); i++) {
        if (ntk._storage->outputs.at(i).index == nodeIdx) {
            return i;
        }
    }
    return 0;
}

template<typename Ntk>
std::vector<int> get_output_indeces(Ntk const &ntk, int nodeIdx)
{

    assert(ntk.is_po(nodeIdx));
    std::vector<int> indeces;
    for (int i = 0; i < ntk._storage->outputs.size(); i++) {
        if (ntk._storage->outputs.at(i).index == nodeIdx) {
            indeces.push_back(i);
        }
    }
    return indeces;
}//get_output_indeces()

/***************************************************/

int get_index(std::vector<int> index, int nodeIdx)
{

    std::vector<int>::iterator it = find(index.begin(), index.end(), nodeIdx);
    return std::distance(index.begin(), it);
}

/* Checks to see if a file has a specified extension
 *
 * params:
 * filename: The name of the file to check the extension of
 * ext: The extension that you are checking that filename has
 *
 */
bool checkExt(std::string filename, std::string ext)
{

    if (filename.substr(filename.find_last_of(".") + 1) == ext) {
        return true;
    } else {
        return false;
    }
}//end checkExt


enum reconfig_function { AND, OR, XOR, XNOR, XOR3, MAJ, INPUT, UNKNOWN };

template <typename network>
reconfig_function node_function(const network &ntk,
				const typename network::node &node)
{
    if (ntk.is_pi(node) || ntk.is_ro(node) || ntk.is_constant(node)) {
	return reconfig_function::INPUT;
    } else if (ntk.is_and(node)) {
	return reconfig_function::AND;
	    } else if (ntk.is_or(node)) {
	return reconfig_function::OR;
    } else if (ntk.is_xor(node)) {
	return reconfig_function::XOR;
    } else if (ntk.is_maj(node)) {
	typename network::signal first_signal = ntk._storage->nodes[node].children[0];
	typename network::node first_fanin = ntk.get_node(first_signal);

	if (ntk.is_constant(first_fanin)) {
	    if (first_signal.complement) {
		return reconfig_function::OR;
	    } else {
		return reconfig_function::AND;
		    }
	} else {
	    return reconfig_function::MAJ;
	}
    } else if (ntk.is_xor3(node)) {
	typename network::signal first_signal = ntk._storage->nodes[node].children[0];
	typename network::node first_fanin = ntk.get_node(first_signal);
	if (ntk.is_constant(first_fanin)) {
	    if (first_signal.complement) {
		return reconfig_function::XNOR;
	    } else {
		return reconfig_function::XOR;
	    }
	} else {
	    return reconfig_function::XOR3;
	}
    } else {
	return reconfig_function::UNKNOWN;
    }
}

template <typename network>
void update_counts(function_counts &counts,
		   const network &ntk,
		   const typename network::node &node)
{
    reconfig_function func = node_function(ntk, node);
    switch(func) {
    case AND: counts.and_num++; break;
    case OR: counts.or_num++; break;
    case XOR: counts.xor_num++; break;
    case XNOR: counts.xnor_num++; break;
    case XOR3: counts.xor3_num++; break;
    case MAJ: counts.maj_num++; break;
    case INPUT: counts.input_num++; break;
    case UNKNOWN:
    default: counts.unknown_num++;
    }
}
template void update_counts<aig_names>(function_counts &, const aig_names &, const typename aig_names::node &);
template void update_counts<mig_names>(function_counts &, const mig_names &, const typename mig_names::node &);
template void update_counts<xmg_names>(function_counts &, const xmg_names &, const typename xmg_names::node &);
template void update_counts<xag_names>(function_counts &, const xag_names &, const typename xag_names::node &);

template <typename network> function_counts node_functions(const network &ntk)
{
    function_counts counts;
    ntk.foreach_node([&](auto node) {
	update_counts(counts, ntk, node);
    });
    return counts;
}
template function_counts node_functions<mig_names>(const mig_names&);
template function_counts node_functions<xmg_names>(const xmg_names&);
template function_counts node_functions<aig_names>(const aig_names&);
template function_counts node_functions<xag_names>(const xag_names&);
}
bool is_digits(const std::string &str)
{
    return std::all_of(str.begin(), str.end(), ::isdigit); // C++11
}

std::vector<int> read_integer_file(string filename)
{

    std::vector<int> output;
    std::ifstream ifs;

    ifs.open(filename);
    if (ifs.is_open()) {
        while (ifs.good()) {
            std::string part;
            getline(ifs, part);
            std::cout<<"part:"<<part<<std::endl;
            if (!part.empty() && is_digits(part))
                output.push_back(std::stoi(part));
        }
        ifs.close();
        return output;
    } else {
        throw exception();
    }
}
