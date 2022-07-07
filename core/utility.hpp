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
#pragma once
#include <alice/alice.hpp>

#include <mockturtle/mockturtle.hpp>
#include "algorithms/partitioning/partition_view.hpp"

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

template<typename Ntk> int computeLevel(Ntk const &ntk, int index);

template<typename Ntk>
int computeLevel(Ntk const, typename Ntk::node, std::set<typename Ntk::node>);

void dfs(mockturtle::aig_network aig, uint64_t index, UnionFind uf);


void compute_cone(mockturtle::aig_network aig, uint64_t index,
                  std::unordered_map<int, int> &nodes, int outindex,
                  std::unordered_map<int, int> &ins);


/***************************************************
  Truth Table
***************************************************/

// Helper function to flip the bit
char flip(char c);

std::string to_binary(int dec);


std::string graytoBinary(std::string gray);


// Function to convert binary to decimal
int binaryToDecimal(int n);

/***************************************************/

/***************************************************
  Network conversion
***************************************************/
mig_ntk aig_to_mig(aig_names aig, int skip_edge_min);


mig_ntk part_to_mig(oracle::partition_view<mig_names> part, int skip_edge_min);


aig_ntk mig_to_aig(mig_names mig);

/***************************************************/

/***************************************************
  Network helpers
***************************************************/
template <typename Ntk>
bool is_po(Ntk const &ntk,
           typename Ntk::node const &n);

bool is_in_vector(std::vector<int> vec, int nodeIdx);


template<typename Ntk>
int get_output_index(Ntk const &ntk, int nodeIdx);


template<typename Ntk>
std::vector<int> get_output_indeces(Ntk const &ntk, int nodeIdx);

/***************************************************/

int get_index(std::vector<int> index, int nodeIdx);

/* Checks to see if a file has a specified extension
 *
 * params:
 * filename: The name of the file to check the extension of
 * ext: The extension that you are checking that filename has
 *
 */
bool checkExt(std::string filename, std::string ext);

struct function_counts
{
    int maj_num = 0;
    int xor_num = 0;
    int xnor_num = 0;
    int xor3_num = 0;
    int and_num = 0;
    int or_num = 0;
    int input_num = 0;
    int unknown_num = 0;
};

template <typename network> function_counts node_functions(const network &ntk);
template <typename network>
void update_counts(function_counts &counts,
		   const network &ntk,
		   const typename network::node &node);

    }
bool is_digits(const std::string &str);
std::vector<int> read_integer_file(string filename);
