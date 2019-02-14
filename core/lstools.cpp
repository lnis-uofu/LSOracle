#include <lorina/lorina.hpp>
#include <alice/alice.hpp>
#include <kitty/kitty.hpp>
#include <mockturtle/mockturtle.hpp>
#include <oracle/oracle.hpp>
#include <libkahypar.h>

#include <mockturtle/views/mffc_view.hpp>
#include <mockturtle/views/fanout_view.hpp>
//should work even though I dont explictly include the xag here. Fix it
#include <mockturtle/algorithms/node_resynthesis/xag_npn.hpp>


#include <iostream>
#include <string>
#include <algorithm>
#include <queue>
#include <unordered_map>
#include <vector>
#include <time.h>
#include <fmt/format.h>
#include <stdio.h>
#include <stdlib.h>
#include <aig/gia/gia.h>
#include <aig/gia/giaAig.h>
#include <base/wlc/wlc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <iomanip>
#include <ot/timer/timer.hpp>

namespace alice{

    //object to hold STA configuration
    STA sta_cfg;

    // Helper function to flip the bit
    char flip(char c){
        return (c == '0') ? '1' : '0';
    }

    std::string graytoBinary(std::string gray){
        std::string binary = "";

        // MSB of binary code is same as gray code
        binary += gray[0];

        // Compute remaining bits
        for (int i = 1; i < gray.length(); i++) {
            // If current bit is 0, concatenate
            // previous bit
            if (gray[i] == '0')
                binary += binary[i - 1];

                // Else, concatenate invert of
                // previous bit
            else
                binary += flip(binary[i - 1]);
        }

        return binary;
    }

    // Function to convert binary to decimal
    int binaryToDecimal(int n){

        int num = n;
        int dec_value = 0;

        // Initializing base value to 1, i.e 2^0
        int base = 1;

        int temp = num;
        while (temp)
        {
            int last_digit = temp % 10;
            temp = temp/10;

            dec_value += last_digit*base;

            base = base*2;
        }

        return dec_value;
    }

    int get_output_index(mockturtle::aig_network aig, int nodeIdx){

        // std::cout << "output nodeIdx = " << nodeIdx << "\n";
        assert(aig.is_po(nodeIdx));
        // std::cout << "Outputs = ";
        // for(int i = 0; i < aig._storage->outputs.size(); i++){
        // 	std::cout << aig._storage->outputs.at(i).index << " ";
        // }
        // std::cout << "\n";

        for(int i = 0; i < aig._storage->outputs.size(); i++){
            if(aig._storage->outputs.at(i).index == nodeIdx){
                // std::cout << "index = " << i << "\n";
                return i;

            }
        }
    }

    std::vector<int> get_output_indeces(mockturtle::aig_network aig, int nodeIdx){

        assert(aig.is_po(nodeIdx));
        std::vector<int> indeces;
        for(int i = 0; i < aig._storage->outputs.size(); i++){
            if(aig._storage->outputs.at(i).index == nodeIdx){
                indeces.push_back(i);
            }
        }
        return indeces;
    }//get_output_indeces()

    std::vector<int> get_output_indeces_mig(mockturtle::mig_network mig, int nodeIdx){

        assert(mig.is_po(nodeIdx));
        std::vector<int> indeces;
        for(int i = 0; i < mig._storage->outputs.size(); i++){
            if(mig._storage->outputs.at(i).index == nodeIdx){
                indeces.push_back(i);
            }
        }
        return indeces;
    }//get_output_indeces_mig()

    int get_index(std::vector<int> index, int nodeIdx){

        std::vector<int>::iterator it = find(index.begin(), index.end(), nodeIdx);
        return std::distance(index.begin(), it);
    }

    int get_input_index(mockturtle::aig_network aig, int nodeIdx){

        // std::cout << "input nodeIdx = " << nodeIdx << "\n";
        // std::cout << "Inputs = ";
        // for(int i = 0; i < aig._storage->inputs.size(); i++){
        // 	std::cout << aig._storage->inputs.at(i) << " ";
        // }
        // std::cout << "\n";
        assert(aig.is_pi(nodeIdx));

        for(int i = 0; i < aig._storage->inputs.size(); i++){
            if(aig._storage->inputs.at(i) == nodeIdx){
                // std::cout << "index = " << i << "\n";
                return i;
            }
        }
    }

    bool is_in_vector(std::vector<int> vec, int nodeIdx){

        if(std::find(vec.begin(),vec.end(),nodeIdx) != vec.end())
            return true;
        else
            return false;
    }

    bool is_in_string_vector(std::vector<std::string> vec, std::string name){

        if(std::find(vec.begin(),vec.end(),name) != vec.end())
            return true;
        else
            return false;
    }

    bool is_in_map(std::map<int, std::vector<int>> _map, int nodeIdx){

        if(_map.find(nodeIdx) != _map.end())
            return true;
        else
            return false;
    }

    std::string to_binary(int dec){

        std::string bin;
        while(dec != 0){
            bin = (dec % 2 == 0 ? "0":"1") + bin;
            dec /= 2;
        }
        return bin;
    }

    void new_tt_blif(mockturtle::aig_network aig){


        std::string filename = aig._storage->net_name + "_test.blif";
        time_t rawtime;
        struct tm * timeinfo;

        time (&rawtime);
        timeinfo = localtime (&rawtime);
        std::ofstream output;
        output.open(filename);
        output << "# Blif file written by lstools on " << asctime(timeinfo);
        output << ".model " << aig._storage->net_name << "\n";

        output << ".inputs ";
        for(int i = 0; i < aig.num_pis(); i++){
            output << aig._storage->inputNames[i] << " ";
        }
        output << "\n";
        output << ".outputs ";
        for(int i = 0; i < aig.num_pos(); i++){
            output << aig._storage->outputNames[i] << " ";
        }
        output << "\n";

        for(int idx = 0; idx < aig._storage->num_partitions; idx++){
            int partition = idx;
            std::cout << "\npartition = " << partition << "\n\n";
            for(int i = 0; i < aig._storage->partitionOutputs[partition].size(); i++){

                int curr_output = aig._storage->partitionOutputs[partition].at(i);
                std::cout << "curr_output = " << curr_output << "\n";
                // std::cout << "tt hex = " << kitty::to_hex(aig._storage->output_tt[curr_output]) << "\n";
                std::string tt = kitty::to_binary(aig._storage->output_tt[curr_output]);
                // std::cout << "got binary\n";
                // std::cout << "tt binary = " << tt << "\n";
                // std::cout << "tt.length = " << tt.length() << "\n";
                char* tt_binary = malloc(sizeof(char)*(tt.length() + 1));
                // std::cout << "declared char array\n";
                strcpy(tt_binary, tt.c_str());
                // std::cout << "copied bits of binary\n";
                // std::cout << "curr_output = " << curr_output << "\n";
                std::string output_str;
                if(aig.is_po(curr_output)){
                    // std::cout << "PO\n";
                    output_str = aig._storage->outputNames[get_output_index(aig, curr_output)];
                    // std::cout << "output_str = " << output_str << "\n";
                }
                else {

                    output_str = std::to_string(curr_output);
                    // std::cout << "output_str = " << output_str << "\n";
                }

                output << ".names ";

                //primary output is connected directly to a primary input
                // if (aig.is_pi(curr_output) && aig.is_po(curr_output)){
                // 	output << aig._storage->inputNames[curr_output] << " " << output_str << "\n";
                // 	output << output_wanted << " " << output_wanted << "\n";

                // }

                //else{
                // std::cout << "partition = " << idx << "\n";
                // std::cout << "curr_output = " << curr_output << "\n";
                // std::cout << "index = {";
                // for(int j = 0; j < aig._storage->index[curr_output].size(); j++){
                // 	std::cout << aig._storage->index[curr_output].at(j) << " ";
                // }
                // std::cout << "}\n";
                if(aig.is_constant(curr_output)){
                    // std::cout << "constant\n";
                    if(aig.is_po(curr_output))
                        output << aig._storage->outputNames[get_output_index(aig, curr_output)] << "\n";
                    else
                        output << curr_output << "\n";
                }
                else{
                    // std::cout << "not constant\n";
                    for(int j = 0; j < aig._storage->logic_cone_inputs[curr_output].size(); j++){
                        int curr_index = aig._storage->logic_cone_inputs[curr_output].at(j);
                        // std::cout << "curr_index = " << curr_index << "\n";
                        std::string index;
                        if(aig.is_pi(curr_index))
                            index = aig._storage->inputNames[curr_index - 1];
                        else if(aig.is_po(curr_index)){

                            index = aig._storage->outputNames[get_output_index(aig, curr_index)];
                        }
                        else
                            index = std::to_string(curr_index);

                        output << index << " ";
                    }

                    if(aig.is_po(curr_output))
                        output << aig._storage->outputNames[get_output_index(aig, curr_output)];
                    else
                        output << curr_output;

                    output << "\n";
                }
                // std::cout << "got names\n";

                std::vector<std::string> onset_indeces;
                int indx = 0;
                for(int j = tt.length() - 1; j >= 0; j--){
                    int bit = (int)tt_binary[j] - 48;
                    if(bit == 1){
                        // std::cout << "index for 1 = " << indx << "\n";
                        onset_indeces.push_back(to_binary(indx));
                    }
                    indx++;
                }
                for(int j = 0; j < onset_indeces.size(); j++){
                    // std::cout << "number of inputs in partition " << partition << " = " << aig._storage->partitionInputs[partition].size() << "\n";
                    // std::cout << "orig binary = " << onset_indeces.at(j) << "\n";
                    while(onset_indeces.at(j).length() != aig._storage->logic_cone_inputs[curr_output].size()){
                        onset_indeces.at(j).insert(0, "0");
                    }
                    std::reverse(onset_indeces.at(j).begin(), onset_indeces.at(j).end());
                    output << onset_indeces.at(j) + " 1\n";
                }
                // std::cout << "got tt\n";
                //}
            }
        }


        output << ".end\n";
        output.close();
    }

    void test_tt_build(mockturtle::aig_network aig, int partition, int nodeIdx, int root){

        if(nodeIdx < 0 || nodeIdx > aig.size() || aig._storage->partitionMap[nodeIdx] != partition){
            return;
        }

        int child1Idx = aig._storage->nodes[nodeIdx].children[0].index;
        int child2Idx = aig._storage->nodes[nodeIdx].children[1].index;

        // std::cout << "child1Idx = " << child1Idx << "\n";
        // std::cout << "child2Idx = " << child2Idx << "\n";
        test_tt_build(aig, partition, aig._storage->nodes[nodeIdx].children[0].index, root);

        test_tt_build(aig, partition, aig._storage->nodes[nodeIdx].children[1].index, root);


        if(!is_in_vector(aig._storage->logic_cone_inputs[root], nodeIdx) ){
            // std::cout << "node = " << nodeIdx << "\n";
            // std::cout << "anding " << child1Idx << " and " << child2Idx << "'s truth tables\n";
            std::vector<kitty::dynamic_truth_table> child_tts;
            child_tts.push_back(aig._storage->tt_map[child1Idx]);
            child_tts.push_back(aig._storage->tt_map[child2Idx]);
            auto child1_sig = aig.make_signal(aig.index_to_node(child1Idx));
            auto child2_sig = aig.make_signal(aig.index_to_node(child2Idx));


            aig.foreach_fanin( aig.index_to_node(nodeIdx), [&]( auto const& conn, auto i ) {

                int childIdx = aig._storage->nodes[aig.index_to_node(nodeIdx)].children[i].index;
                if ( aig.is_complemented( conn )) {
                    // std::cout << "child " << childIdx << " is inverted\n";
                    child_tts.at(i) = ~aig._storage->tt_map[childIdx];
                    // kitty::print_hex(child_tts.at(i), std::cout);
                    // std::cout << "\n";

                }

                if(aig.is_po(childIdx) && is_in_vector(aig._storage->logic_cone_inputs[root], childIdx)){
                    // std::cout << "child " << childIdx << " is a po\n";
                    auto output = aig._storage->outputs.at(get_output_index(aig,childIdx));
                    if(output.data & 1){
                        // std::cout << "child output is inverted\n";
                        child_tts.at(i) = ~child_tts.at(i);
                    }
                }
            });
            kitty::dynamic_truth_table tt = kitty::binary_and(child_tts.at(0), child_tts.at(1));
            aig._storage->tt_map[nodeIdx] = tt;
            // kitty::print_hex(tt, std::cout);
            // std::cout << "\n";
        }

        if(aig.is_po(nodeIdx) && nodeIdx == root){
            // std::cout << "output\n";
            auto output = aig._storage->outputs.at(get_output_index(aig,nodeIdx));
            if(output.data & 1){
                // std::cout << "output inverted\n";
                aig._storage->tt_map[nodeIdx] = ~aig._storage->tt_map[nodeIdx];
            }

        }
        // std::cout << nodeIdx << " ";

    }

    mockturtle::aig_network create_aig_from_part(mockturtle::aig_network aig, int partition){

        mockturtle::aig_network new_aig;
        new_aig._storage->net_name = aig._storage->net_name + "_" + std::to_string(partition);
        std::vector<int> index;

        //BFS Traversal of all outputs
        for(int i = 0; i < aig._storage->partitionOutputs[partition].size(); i++){
            std::queue<int> net_queue;
            std::map<int, bool> visited;
            int output = aig._storage->partitionOutputs[partition].at(i);
            //Set all nodes to be unvisited
            aig.foreach_node( [&]( auto node ) {
                visited[aig.node_to_index(node)] = false;
            });

            net_queue.push(output);
            visited[output] = true;

            while(!net_queue.empty()){

                int curr_node = net_queue.front();
                //make sure there are no duplicates added
                if(!is_in_vector(index, curr_node)){
                    //Put inputs at the beginning of the index so they are added into the AIG first
                    if(is_in_vector(aig._storage->partitionInputs[partition], curr_node))
                        index.insert(index.begin(), curr_node);
                    else
                        index.push_back(curr_node);
                }
                net_queue.pop();
                auto node = aig.index_to_node(curr_node);

                //Make sure that the BFS traversal does not go past the inputs of the partition
                if(!is_in_vector(aig._storage->partitionInputs[partition],curr_node)){

                    for(int i = 0; i < aig._storage->nodes[aig.index_to_node(curr_node)].children.size(); i++){

                        int childIdx = aig._storage->nodes[aig.index_to_node(curr_node)].children[i].index;

                        if(!visited[childIdx]){
                            net_queue.push(childIdx);
                            visited[childIdx] = true;
                        }
                    }

                }
            }
        }

        for(int i = 0; i < index.size(); i++){
            int nodeIdx = index.at(i);
            auto node = aig.index_to_node(nodeIdx);

            //outputs tied directly to output
            if(is_in_vector(aig._storage->partitionInputs[partition], nodeIdx) && is_in_vector(aig._storage->partitionOutputs[partition], nodeIdx)){

                auto pi = new_aig.create_pi();
                std::string input_name;
                if(aig.is_pi(node)){
                    input_name = aig._storage->inputNames[nodeIdx - 1];
                }
                else
                    input_name = "part_" + std::to_string(nodeIdx);

                new_aig._storage->inputNames[new_aig.node_to_index(new_aig.get_node(pi) - 1)] = input_name;


                std::vector<int> output_name_indeces = get_output_indeces(aig, nodeIdx);
                std::vector<std::string> output_names;
                for(int k = 0; k < output_name_indeces.size(); k++){
                    output_names.push_back(aig._storage->outputNames[output_name_indeces.at(k)]);
                    if(aig._storage->outputs[get_output_index(aig, nodeIdx)].data & 1){
                        pi = aig.create_not(pi);
                    }
                    new_aig.create_po(pi);
                }
                std::vector<int> new_output_name_indeces = get_output_indeces(new_aig, new_aig.node_to_index(new_aig.get_node(pi)));
                for(int k = 0; k < output_names.size(); k++){
                    new_aig._storage->outputNames[new_output_name_indeces.at(k)] = output_names.at(k);
                }

            }
            //create pi
            else if(is_in_vector(aig._storage->partitionInputs[partition], nodeIdx)){

                auto pi = new_aig.create_pi();
                std::string input_name;
                if(aig.is_pi(node)){
                    input_name = aig._storage->inputNames[nodeIdx - 1];
                }
                else if(aig.is_po(node)){
                    input_name = "PO_" + aig._storage->outputNames[get_output_index(aig, nodeIdx)];
                }
                else
                    input_name = "part_" + std::to_string(nodeIdx);

                new_aig._storage->inputNames[new_aig.node_to_index(new_aig.get_node(pi) - 1)] = input_name;
            }
            else{
                std::vector<int>::iterator c1_it = std::find (index.begin(), index.end(), aig._storage->nodes[node].children[0].index);
                std::vector<int>::iterator c2_it = std::find (index.begin(), index.end(), aig._storage->nodes[node].children[1].index);
                int child1Idx = std::distance(index.begin(), c1_it);
                int child2Idx = std::distance(index.begin(), c2_it);
                auto child1_signal = aig.make_signal(aig.index_to_node(child1Idx + 1));
                auto child2_signal = aig.make_signal(aig.index_to_node(child2Idx + 1));
                if(aig._storage->nodes[node].children[0].data & 1){
                    child1_signal = new_aig.create_not(child1_signal);
                }
                if(aig._storage->nodes[node].children[1].data & 1){
                    child2_signal = new_aig.create_not(child2_signal);
                }
                auto gate = new_aig.clone_node(aig, aig.get_node(aig.make_signal(node)), {child1_signal, child2_signal});

                //Get truth table logic for each node
                auto func = new_aig.node_function(new_aig.get_node(gate));
                new_aig.foreach_fanin( new_aig.get_node(gate), [&]( auto const& conn, auto i ) {
                    if ( new_aig.is_complemented( conn ) ) {
                        kitty::flip_inplace( func, i );
                    }
                });

                if(is_in_vector(aig._storage->partitionOutputs[partition], nodeIdx)){

                    if(aig.is_po(nodeIdx)){
                        std::vector<std::string> output_names;
                        std::vector<int> output_name_indeces = get_output_indeces(aig, nodeIdx);

                        for(int k = 0; k < output_name_indeces.size(); k++){
                            output_names.push_back(aig._storage->outputNames[output_name_indeces.at(k)]);
                            if(aig._storage->outputs[get_output_index(aig, nodeIdx)].data & 1){
                                gate = aig.create_not(gate);
                            }
                            new_aig.create_po(gate);

                        }
                        std::vector<int> new_output_name_indeces = get_output_indeces(new_aig, new_aig.node_to_index(new_aig.get_node(gate)));
                        for(int k = 0; k < output_names.size(); k++){
                            new_aig._storage->outputNames[new_output_name_indeces.at(k)] = output_names.at(k);
                        }
                    }

                    else{
                        std::string output_name = "part_" + std::to_string(nodeIdx);
                        new_aig.create_po(gate);
                        new_aig._storage->outputNames[get_output_index(new_aig, new_aig.node_to_index(new_aig.get_node(gate)))] = output_name;
                    }

                }
            }
        }

        return new_aig;

    }//create_aig_from_part()

    mockturtle::aig_network create_aig_from_part(mockturtle::aig_network aig, int partition, int output){

        mockturtle::aig_network new_aig;
        new_aig._storage->net_name = aig._storage->net_name + "_" + std::to_string(partition);
        std::vector<int> index;

        //BFS Traversal of all outputs
        for(int i = 0; i < aig._storage->partitionOutputs[partition].size(); i++){
            std::queue<int> net_queue;
            std::map<int, bool> visited;
            int output = aig._storage->partitionOutputs[partition].at(i);
            //Set all nodes to be unvisited
            aig.foreach_node( [&]( auto node ) {
                visited[aig.node_to_index(node)] = false;
            });

            net_queue.push(output);
            visited[output] = true;

            while(!net_queue.empty()){

                int curr_node = net_queue.front();
                //make sure there are no duplicates added
                if(!is_in_vector(index, curr_node)){
                    //Put inputs at the beginning of the index so they are added into the AIG first
                    if(is_in_vector(aig._storage->partitionInputs[partition], curr_node))
                        index.insert(index.begin(), curr_node);
                    else
                        index.push_back(curr_node);
                }
                net_queue.pop();
                auto node = aig.index_to_node(curr_node);

                //Make sure that the BFS traversal does not go past the inputs of the partition
                if(!is_in_vector(aig._storage->partitionInputs[partition],curr_node)){

                    for(int i = 0; i < aig._storage->nodes[aig.index_to_node(curr_node)].children.size(); i++){

                        int childIdx = aig._storage->nodes[aig.index_to_node(curr_node)].children[i].index;

                        if(!visited[childIdx]){
                            net_queue.push(childIdx);
                            visited[childIdx] = true;
                        }
                    }

                }
            }
        }

        for(int i = 0; i < index.size(); i++){
            int nodeIdx = index.at(i);
            auto node = aig.index_to_node(nodeIdx);

            //outputs tied directly to output
            if(is_in_vector(aig._storage->partitionInputs[partition], nodeIdx) && is_in_vector(aig._storage->partitionOutputs[partition], nodeIdx)){

                auto pi = new_aig.create_pi();
                std::string input_name;
                if(aig.is_pi(node)){
                    input_name = aig._storage->inputNames[nodeIdx - 1];
                }
                else
                    input_name = "part_" + std::to_string(nodeIdx);

                new_aig._storage->inputNames[new_aig.node_to_index(new_aig.get_node(pi) - 1)] = input_name;


                std::vector<int> output_name_indeces = get_output_indeces(aig, nodeIdx);
                std::vector<std::string> output_names;
                for(int k = 0; k < output_name_indeces.size(); k++){
                    output_names.push_back(aig._storage->outputNames[output_name_indeces.at(k)]);
                    if(aig._storage->outputs[get_output_index(aig, nodeIdx)].data & 1){
                        pi = aig.create_not(pi);
                    }
                    new_aig.create_po(pi);
                }
                std::vector<int> new_output_name_indeces = get_output_indeces(new_aig, new_aig.node_to_index(new_aig.get_node(pi)));
                for(int k = 0; k < output_names.size(); k++){
                    new_aig._storage->outputNames[new_output_name_indeces.at(k)] = output_names.at(k);
                }

            }
            //create pi
            else if(is_in_vector(aig._storage->partitionInputs[partition], nodeIdx)){

                auto pi = new_aig.create_pi();
                std::string input_name;
                if(aig.is_pi(node)){
                    input_name = aig._storage->inputNames[nodeIdx - 1];
                }
                else if(aig.is_po(node)){
                    input_name = "PO_" + aig._storage->outputNames[get_output_index(aig, nodeIdx)];
                }
                else
                    input_name = "part_" + std::to_string(nodeIdx);

                new_aig._storage->inputNames[new_aig.node_to_index(new_aig.get_node(pi) - 1)] = input_name;
            }
            else{
                std::vector<int>::iterator c1_it = std::find (index.begin(), index.end(), aig._storage->nodes[node].children[0].index);
                std::vector<int>::iterator c2_it = std::find (index.begin(), index.end(), aig._storage->nodes[node].children[1].index);
                int child1Idx = std::distance(index.begin(), c1_it);
                int child2Idx = std::distance(index.begin(), c2_it);
                auto child1_signal = aig.make_signal(aig.index_to_node(child1Idx + 1));
                auto child2_signal = aig.make_signal(aig.index_to_node(child2Idx + 1));
                if(aig._storage->nodes[node].children[0].data & 1){
                    child1_signal = new_aig.create_not(child1_signal);
                }
                if(aig._storage->nodes[node].children[1].data & 1){
                    child2_signal = new_aig.create_not(child2_signal);
                }
                auto gate = new_aig.clone_node(aig, aig.get_node(aig.make_signal(node)), {child1_signal, child2_signal});

                //Get truth table logic for each node
                auto func = new_aig.node_function(new_aig.get_node(gate));
                new_aig.foreach_fanin( new_aig.get_node(gate), [&]( auto const& conn, auto i ) {
                    if ( new_aig.is_complemented( conn ) ) {
                        kitty::flip_inplace( func, i );
                    }
                });

                if(is_in_vector(aig._storage->partitionOutputs[partition], nodeIdx)){

                    if(aig.is_po(nodeIdx)){
                        std::vector<std::string> output_names;
                        std::vector<int> output_name_indeces = get_output_indeces(aig, nodeIdx);

                        for(int k = 0; k < output_name_indeces.size(); k++){
                            output_names.push_back(aig._storage->outputNames[output_name_indeces.at(k)]);
                            if(aig._storage->outputs[get_output_index(aig, nodeIdx)].data & 1){
                                gate = aig.create_not(gate);
                            }
                            new_aig.create_po(gate);

                        }
                        std::vector<int> new_output_name_indeces = get_output_indeces(new_aig, new_aig.node_to_index(new_aig.get_node(gate)));
                        for(int k = 0; k < output_names.size(); k++){
                            new_aig._storage->outputNames[new_output_name_indeces.at(k)] = output_names.at(k);
                        }
                    }

                    else{
                        std::string output_name = "part_" + std::to_string(nodeIdx);
                        new_aig.create_po(gate);
                        new_aig._storage->outputNames[get_output_index(new_aig, new_aig.node_to_index(new_aig.get_node(gate)))] = output_name;
                    }

                }
            }
        }

        return new_aig;

    }//create_aig_from_part()

    mockturtle::aig_network create_aig_from_part_test(mockturtle::aig_network aig, int partition){

        mockturtle::aig_network new_aig;
        new_aig._storage->net_name = aig._storage->net_name + "_" + std::to_string(partition);
        std::vector<int> index;

        //BFS Traversal of all outputs
        for(int i = 0; i < aig._storage->partitionOutputs[partition].size(); i++){
            std::queue<int> net_queue;
            std::map<int, bool> visited;
            int output = aig._storage->partitionOutputs[partition].at(i);
            //Set all nodes to be unvisited
            aig.foreach_node( [&]( auto node ) {
                visited[aig.node_to_index(node)] = false;
            });

            net_queue.push(output);
            visited[output] = true;

            while(!net_queue.empty()){

                int curr_node = net_queue.front();
                // std::cout << "curr_node = " << curr_node << "\n";
                //make sure there are no duplicates added
                if(!is_in_vector(index, curr_node)){
                    //Put inputs at the beginning of the index so they are added into the AIG first
                    if(is_in_vector(aig._storage->partitionInputs[partition], curr_node))
                        index.insert(index.begin(), curr_node);
                    else
                        index.push_back(curr_node);
                }
                net_queue.pop();
                auto node = aig.index_to_node(curr_node);

                //Make sure that the BFS traversal does not go past the inputs of the partition
                if(!is_in_vector(aig._storage->partitionInputs[partition],curr_node)){
                    //if(!is_in_vector(aig._storage->partitionInputs[partition], curr_node)){

                    for(int i = 0; i < aig._storage->nodes[aig.index_to_node(curr_node)].children.size(); i++){

                        int childIdx = aig._storage->nodes[aig.index_to_node(curr_node)].children[i].index;

                        if(!visited[childIdx]){
                            // std::cout << "childIdx = " << childIdx << "\n";
                            net_queue.push(childIdx);
                            visited[childIdx] = true;
                        }
                    }

                }
            }
        }
        std::cout << "index = {";
        for(int i = 0; i < index.size(); i++){
            std::cout << index.at(i) << " ";
        }
        std::cout << "}\n";

        for(int i = 0; i < index.size(); i++){
            int nodeIdx = index.at(i);
            // std::cout << "current node = " << nodeIdx << "\n";
            auto node = aig.index_to_node(nodeIdx);

            if(is_in_vector(aig._storage->partitionInputs[partition], nodeIdx)){
                //if(is_in_vector(aig._storage->partitionInputs[partition], nodeIdx)){
                auto pi = new_aig.create_pi();
                // std::cout << "create pi\n";
                std::string input_name;
                if(aig.is_pi(node)){
                    input_name = aig._storage->inputNames[nodeIdx - 1];
                }
                else
                    input_name = std::to_string(nodeIdx);

                std::cout << "input_name " << input_name << "\n";
                std::cout << "input index = " << new_aig.node_to_index(new_aig.get_node(pi) - 1) << "\n";

                // std::cout << "adding " << input_name << " as name for " << nodeIdx << "\n";
                new_aig.create_in_name(new_aig.node_to_index(new_aig.get_node(pi) - 1), input_name);
            }
            else{
                std::vector<int>::iterator c1_it = std::find (index.begin(), index.end(), aig._storage->nodes[node].children[0].index);
                std::vector<int>::iterator c2_it = std::find (index.begin(), index.end(), aig._storage->nodes[node].children[1].index);
                int child1Idx = std::distance(index.begin(), c1_it);
                // std::cout << "child1Idx = " << child1Idx << "\n";
                int child2Idx = std::distance(index.begin(), c2_it);
                // std::cout << "child2Idx = " << child2Idx << "\n";
                auto child1_signal = aig.make_signal(aig.index_to_node(child1Idx + 1));
                auto child2_signal = aig.make_signal(aig.index_to_node(child2Idx + 1));
                if(aig._storage->nodes[node].children[0].data & 1){
                    child1_signal = new_aig.create_not(child1_signal);
                }
                if(aig._storage->nodes[node].children[1].data & 1){
                    child2_signal = new_aig.create_not(child2_signal);
                }
                auto gate = new_aig.clone_node(aig, aig.get_node(aig.make_signal(node)), {child1_signal, child2_signal});
                // auto gate = new_aig.create_and(aig.make_signal(aig.index_to_node(child1Idx + 1)), aig.make_signal(aig.index_to_node(child2Idx + 1)));
                // std::cout << "create gate with children " << child1Idx + 1 << " and " << child2Idx + 1 << "\n";

                //Get truth table logic for each node
                auto func = new_aig.node_function(new_aig.get_node(gate));
                new_aig.foreach_fanin( new_aig.get_node(gate), [&]( auto const& conn, auto i ) {
                    if ( new_aig.is_complemented( conn ) ) {
                        kitty::flip_inplace( func, i );
                    }
                });

                // if(nodeIdx == output){
                if(is_in_vector(aig._storage->partitionOutputs[partition], nodeIdx)){

                    if(aig.is_po(nodeIdx) && aig._storage->outputs[get_output_index(aig, nodeIdx)].data & 1){
                        // std::cout << "HERE\n";
                        gate = aig.create_not(gate);
                    }
                    new_aig.create_po(gate);
                    std::string output_name;

                    if(aig.is_po(nodeIdx)){
                        std::cout << "nodeIdx " << nodeIdx << " output name = " << aig._storage->outputNames[get_output_index(aig, nodeIdx)] << "\n";
                        output_name = aig._storage->outputNames[get_output_index(aig, nodeIdx)];
                    }

                    else
                        output_name = std::to_string(nodeIdx);

                    std::cout << "output_name " << output_name << "\n";
                    std::cout << "output index = " << new_aig.node_to_index(new_aig.get_node(gate)) << "\n";
                    // std::cout << "adding " << std::to_string(nodeIdx) << " as output name for " << nodeIdx << "\n";
                    new_aig.create_out_name(new_aig.node_to_index(new_aig.get_node(gate)), output_name);
                }
            }
        }
        // std::cout << "Inputs:\n";

        // for(int i = 0; i < new_aig._storage->inputs.size(); i++){
        // 	std::cout << new_aig._storage->inputs.at(i) << ": " << new_aig._storage->inputNames[new_aig._storage->inputs.at(i)] << "\n";
        // }

        // std::cout << "Outputs:\n";

        // for(int i = 0; i < new_aig._storage->outputs.size(); i++){
        // 	std::cout << new_aig._storage->outputs.at(i).index << ": " << new_aig._storage->outputNames[new_aig._storage->outputs.at(i).index] << "\n";
        // }

        // new_aig.foreach_node( [&] (auto node){
        // 	std::cout << "node: " << new_aig.node_to_index(node) << " child[0] = " << new_aig._storage->nodes[node].children[0].index << "\n";
        // 	std::cout << "node: " << new_aig.node_to_index(node) << " child[1] = " << new_aig._storage->nodes[node].children[1].index << "\n";
        // });

        return new_aig;

    }

    void write_aig(mockturtle::aig_network aig, std::string filename){


        std::ofstream aigfile;
        aigfile.open (filename);

        // header info - MILOA
        auto _num_inputs = aig.num_pis();
        // auto _num_latches = aig.num_latches();
        auto _num_latches = 0;

        auto _num_outputs = aig.num_pos();
        auto _num_ands = aig.num_gates();
        auto _num_vertices = aig.num_pis() + aig.num_gates();

        //write aig header
        aigfile << "aag " << _num_vertices << " " << _num_inputs - _num_latches << " " << _num_latches << " " << (_num_outputs - _num_latches) << " " << _num_ands << std::endl;

        aig.foreach_pi([&] (auto node)
        {
            auto index = aig.node_to_index(node);
            aigfile << index * 2 << "\n";
        });

        //write aig latches
        auto lineIdx = ((aig.num_pis()-_num_latches)*2)+2;
        for(int i=0; i<_num_latches; i++){
            auto regIdx = aig.num_pos() - _num_latches + i;
            aigfile << lineIdx << " " << aig._storage->outputs[regIdx].data << "\n";
            lineIdx += 2;
        }

        //write aig outputs
        for(int i=0; i<aig.num_pos()- _num_latches; i++){
            aigfile << aig._storage->outputs[i].data << "\n";
        }

        auto skipLatches = 0;
        aig.foreach_gate([&] (auto node)
        {
            //skip latches in the nodes vector
            if(skipLatches>=_num_latches) {
                auto index = aig.node_to_index(node);
                auto left = aig._storage->nodes[index].children[0].data;
                auto right = aig._storage->nodes[index].children[1].data;
                aigfile << index*2 << " " << left << " " << right << "\n";
            }
            skipLatches+=1;
        });
        for(int i = 0; i < aig._storage->inputs.size(); i++){
            aigfile << "i" << i << " " << aig._storage->inputNames[aig._storage->inputs.at(i) - 1] << "\n";

        }
        for(int i = 0; i < aig._storage->outputs.size(); i++){
            aigfile << "o" << i << " " << aig._storage->outputNames[i] << "\n";

        }
        aigfile.close();
    }


    //testing

	/* Checks to see if a file has a specified extension
	 *
	 * params:
	 * filename: The name of the file to check the extension of
	 * ext: The extension that you are checking that filename has
	 *
	 */
	bool checkExt(std::string filename, std::string ext){

		if(filename.substr(filename.find_last_of(".") + 1) == ext) {
    		return true;
  		} else {
    		return false;
  		}
	}//end checkExt

    int computeLevelPart( mockturtle::aig_network aig, int index, int partition ) {
        //if node not visited
        if(aig._storage->nodes[index].data[1].h1==0) {

            //set node as visited
            aig._storage->nodes[index].data[1].h1=1;

            //if is input
            if (is_in_vector(aig._storage->partitionInputs[partition], index)) {
                return 0;
            }

            auto inIdx2 = aig._storage->nodes[index].children[1].data;
            if (inIdx2 & 1)
                inIdx2 = inIdx2 - 1;

            //calculate input node index
            auto inNode1 = inIdx2 >> 1;
            int levelNode1 = computeLevelPart(aig, inNode1, partition);

            auto inIdx = aig._storage->nodes[index].children[0].data;
            if (inIdx & 1)
                inIdx = inIdx - 1;

            //calculate input node index
            auto inNode0 = inIdx >> 1;
            int levelNode0 = computeLevelPart(aig, inNode0, partition);

            int level = 1 + std::max(levelNode0, levelNode1);
            return level;
        }
    }

    std::vector<std::vector<int>> get_logic_from_hex(std::string hex){

        //std::cout << "Logic Hex VAlue = " << hex << "\n";
        int hexValue = atoi(hex.c_str());
        switch(hexValue){
            case 8:
                return {{1, 1}};
                break;

            case 1:
                return {{0, 0}};
                break;

            case 4:
                return {{0, 1}};
                break;

            case 2:
                return {{1, 0}};
                break;

            default:
                std::cout << "Error in gate logic\n";

        }
    }

    std::vector<std::vector<int>> other_possible_outputs(std::vector<std::vector<int>> out){
        std::vector<std::vector<int>> possibles = {{0,0},{0,1},{1,0},{1,1}};

        for(int i = 0; i < out.size(); i++){
            for(int j = 0; j < possibles.size(); j++){

                if(possibles.at(j) == out.at(i)){
                    possibles.erase(possibles.begin() + j);
                }
            }
        }

        return possibles;

    }
    int computeLevel( mockturtle::aig_network aig, int index ) {
		//if node not visited
        if(aig._storage->nodes[index].data[1].h1==0) {

            //set node as visited
            aig._storage->nodes[index].data[1].h1=1;

            //if is input
            if (aig.is_ci(index)) {
                return 0;
            }

			auto inIdx2 = aig._storage->nodes[index].children[1].data;
			if (inIdx2 & 1)
				inIdx2 = inIdx2 - 1;

			//calculate input node index
			auto inNode1 = inIdx2 >> 1;
			int levelNode1 = computeLevel(aig, inNode1);

            auto inIdx = aig._storage->nodes[index].children[0].data;
            if (inIdx & 1)
                inIdx = inIdx - 1;

            //calculate input node index
            auto inNode0 = inIdx >> 1;
            int levelNode0 = computeLevel(aig, inNode0);

            int level = 1 + std::max(levelNode0, levelNode1);
            return level;
        }
	}

    int computeLevel( mockturtle::aig_network aig, int index, int partition ) {
        //if node not visited
        if(aig._storage->nodes[index].data[1].h1==0) {

            //set node as visited
            aig._storage->nodes[index].data[1].h1=1;

            //if is input
            if (is_in_vector(aig._storage->partitionInputs[partition], index)) {
                return 0;
            }

            auto inIdx2 = aig._storage->nodes[index].children[1].data;
            if (inIdx2 & 1)
                inIdx2 = inIdx2 - 1;

            //calculate input node index
            auto inNode1 = inIdx2 >> 1;
            int levelNode1 = computeLevel(aig, inNode1, partition);

            auto inIdx = aig._storage->nodes[index].children[0].data;
            if (inIdx & 1)
                inIdx = inIdx - 1;

            //calculate input node index
            auto inNode0 = inIdx >> 1;
            int levelNode0 = computeLevel(aig, inNode0, partition);

            int level = 1 + std::max(levelNode0, levelNode1);
            return level;
        }
    }

    /* add_to_truth
     *
     * @param aig: AIG network that the truth table is being made for
     * @param nodeIdx: The index of the gate that you are adding the truth table of
     * @param wantedOut: The output that you want the gate at nodeIdx to be
     * @param output: The node that the truth table is being made for
     * @param fork_flag: Can be one of four (4) values that are used to determine if a connection
     *					 is forked and logic being added to the truth table needs to be restricted.
     *					 0: No connection is forked
     *					 1: The left child is forked
     *					 2: The right child is forked
     *					 3: Both children are forked
     *
     * Adds to a truth table for the output by adding the truth table needed for the logic output
     * wanted for a given node(nodeIdx)
     */
    void add_to_truth(mockturtle::aig_network aig, int nodeIdx, int wantedOut, int output, int fork_flag, clock_t start){

        auto node = aig.index_to_node(nodeIdx);
        aig._storage->test_timeout = false;
        // std::cout << "\noutput node = " << nodeIdx << "\n\n";
        // std::cout << "child[0] = " << aig._storage->nodes[node].children[0].index << "\n";
        // std::cout << "child[1] = " << aig._storage->nodes[node].children[1].index << "\n";
        // std::cout << "wantedOut = " << wantedOut << " fork_flag = " << fork_flag << "\n\n";

        int child1Idx = aig._storage->nodes[node].children[0].index;

        int child2Idx = aig._storage->nodes[node].children[1].index ;


        // std::cout << "index = {";
        // for(int i = 0; i < aig._storage->index[output].size(); i++){
        // 	std::cout << aig._storage->index[output].at(i) << " ";
        // }
        // std::cout << "}\n";

        //Values that a forked connection MUST be for the truth table to be valid i.e. you can't have 2 different
        //values for the same wire at the same time
        int critical_val1;
        int critical_val2;

        if(!aig.is_constant(node)){
            if(wantedOut == 0){

                std::vector<std::vector<int>> offset_cpy;;
                int i = 0;

                while(i < aig._storage->tt[output].size() && !aig._storage->test_timeout){
                    float current_time = ((float)clock() - start)/CLOCKS_PER_SEC;
                    // std::cout << "check time " << current_time << "\n";
                    // std::cout << "fork_flag = " << fork_flag << "\n";
                    if(current_time >= 180){
                        std::cout << "TIMEOUT at " << current_time << " seconds\n";
                        aig._storage->test_timeout = true;
                        break;
                    }
                    //Copy the offset list so the original is not changed
                    offset_cpy = aig._storage->offset[nodeIdx];
                    //Check to make sure that you only affect rows of the truth table where the node's wanted output
                    //matches in order to merge its onset or offset truth table to the respevtive output
                    if(aig._storage->tt[output].at(i).at(get_index(aig._storage->index[output], nodeIdx)) == wantedOut){

                        if(fork_flag == 1){
                            //Checks the value already stored for the child index
                            //Since these values are added during a BFS, the value already there is more critical
                            //to the output and therefore needs to be matched
                            critical_val1 = aig._storage->tt[output].at(i).at( get_index(aig._storage->index[output], child1Idx));

                            //Erase from the offset truth table, logic combinations that don't match the critical value
                            for(int j = 0; j < offset_cpy.size(); j++){
                                if(offset_cpy.at(j).at(0) != critical_val1){
                                    //std::cout << "Erasing at " << std::distance(offset_cpy.begin(), offset_cpy.begin() + j) << "\n";
                                    offset_cpy.erase(offset_cpy.begin() + j);
                                    j--;
                                }
                            }
                            //If the new offset truth table is empty now, the current node's is not valid and should
                            //be erased from the truth table
                            if(offset_cpy.size() == 0){
                                aig._storage->tt[output].erase(aig._storage->tt[output].begin() + i);
                            }
                            else{
                                //Only add values for the right child since the left child is already taken care of
                                aig._storage->tt[output].at(i).at( get_index(aig._storage->index[output], child2Idx)) = offset_cpy.at(0).at(1);

                                int idx = i + 1;
                                //Copy the truth table down to add more combinations for the wanted output if needed
                                for(int j = 1; j < offset_cpy.size(); j++){

                                    aig._storage->tt[output].insert(aig._storage->tt[output].begin() + idx, aig._storage->tt[output].at(i));

                                    aig._storage->tt[output].at(idx).at(get_index(aig._storage->index[output], child2Idx)) = offset_cpy.at(j).at(1);

                                    idx++;
                                }
                                i += offset_cpy.size();
                            }
                        }
                        else if(fork_flag == 2){
                            //Checks the value already stored for the child index
                            //Since these values are added during a BFS, the value already there is more critical
                            //to the output and therefore needs to be matched
                            critical_val2 = aig._storage->tt[output].at(i).at( get_index(aig._storage->index[output], child2Idx));

                            //Erase from the offset truth table, logic combinations that don't match the critical value
                            for(int j = 0; j < offset_cpy.size(); j++){
                                if(offset_cpy.at(j).at(1) != critical_val2){
                                    offset_cpy.erase(offset_cpy.begin() + j);
                                    j--;
                                }
                            }
                            //If the new offset truth table is empty now, the current node's is not valid and should
                            //be erased from the truth table
                            if(offset_cpy.size() == 0){
                                aig._storage->tt[output].erase(aig._storage->tt[output].begin() + i);
                            }
                            else{
                                //Only add values for the left child since the right child is already taken care of
                                aig._storage->tt[output].at(i).at( get_index(aig._storage->index[output], child1Idx)) = offset_cpy.at(0).at(0);

                                int idx = i + 1;
                                //Copy the truth table down to add more combinations for the wanted output if needed
                                for(int j = 1; j < offset_cpy.size(); j++){

                                    aig._storage->tt[output].insert(aig._storage->tt[output].begin() + idx, aig._storage->tt[output].at(i));

                                    aig._storage->tt[output].at(idx).at(get_index(aig._storage->index[output], child1Idx)) = offset_cpy.at(j).at(0);

                                    idx++;
                                }
                                i += offset_cpy.size();
                            }
                        }
                        else if(fork_flag == 3){
                            //Checks the value already stored for both of the children
                            //Since these values are added during a BFS, the value already there is more critical
                            //to the output and therefore needs to be matched
                            critical_val1 = aig._storage->tt[output].at(i).at( get_index(aig._storage->index[output], child1Idx));
                            critical_val2 = aig._storage->tt[output].at(i).at( get_index(aig._storage->index[output], child2Idx));

                            //Erase from the offset truth table, logic combinations that don't match both of the critical values
                            for(int j = 0; j < offset_cpy.size(); j++){
                                if(!((offset_cpy.at(j).at(0) == critical_val1) && (offset_cpy.at(j).at(1) == critical_val2))){
                                    offset_cpy.erase(offset_cpy.begin() + j);
                                    j--;
                                }
                            }
                            //If the new offset truth table is empty now, the current node's is not valid and should
                            //be erased from the truth table
                            if(offset_cpy.size() == 0){
                                aig._storage->tt[output].erase(aig._storage->tt[output].begin() + i);
                            }
                            else{
                                i++;
                            }
                        }

                        else{

                            aig._storage->tt[output].at(i).at( get_index(aig._storage->index[output], child1Idx)) = offset_cpy.at(0).at(0);
                            aig._storage->tt[output].at(i).at( get_index(aig._storage->index[output], child2Idx)) = offset_cpy.at(0).at(1);
                            int idx = i + 1;
                            //Copy the truth table down to add more combinations for the wanted output if needed
                            for(int j = 1; j < offset_cpy.size(); j++){

                                aig._storage->tt[output].insert(aig._storage->tt[output].begin() + idx, aig._storage->tt[output].at(i));

                                aig._storage->tt[output].at(idx).at(get_index(aig._storage->index[output], child1Idx)) = offset_cpy.at(j).at(0);
                                aig._storage->tt[output].at(idx).at(get_index(aig._storage->index[output], child2Idx)) = offset_cpy.at(j).at(1);

                                idx++;
                            }
                            i += offset_cpy.size();
                        }

                    }
                    else{
                        i++;
                    }

                    //Update the wanted outputs for the children of the node
                    for(int i = 0; i < offset_cpy.size(); i++){
                        // std::cout << "offset row: ";
                        // for(int j = 0; j < offset_cpy.at(i).size(); j++){
                        // 	std::cout << offset_cpy.at(i).at(j) << " ";
                        // }
                        // std::cout << "\n";
                        if(fork_flag == 1){
                            if(!is_in_vector(aig._storage->wantedOut[child2Idx], offset_cpy.at(i).at(1))){
                                //std::cout << "1) Offset adding " << offset_cpy.at(i).at(1) << " as wantedOut for " << child2Idx << "\n";
                                aig._storage->wantedOut[child2Idx].push_back(offset_cpy.at(i).at(1));
                            }
                        }
                        else if(fork_flag == 2){
                            if(!is_in_vector(aig._storage->wantedOut[child1Idx], offset_cpy.at(i).at(0))){
                                //std::cout << "2) Offset adding " << offset_cpy.at(i).at(0) << " as wantedOut for " << child1Idx << "\n";
                                aig._storage->wantedOut[child1Idx].push_back(offset_cpy.at(i).at(0));
                            }
                        }
                        else{
                            if(!is_in_vector(aig._storage->wantedOut[child1Idx], offset_cpy.at(i).at(0))){
                                //std::cout << "ELSE) Offset adding " << offset_cpy.at(i).at(0) << " as wantedOut for " << child1Idx << "\n";
                                aig._storage->wantedOut[child1Idx].push_back(offset_cpy.at(i).at(0));
                            }
                            if(!is_in_vector(aig._storage->wantedOut[child2Idx], offset_cpy.at(i).at(1))){
                                //std::cout << "ELSE) Offset adding " << offset_cpy.at(i).at(1) << " as wantedOut for " << child2Idx << "\n";
                                aig._storage->wantedOut[child2Idx].push_back(offset_cpy.at(i).at(1));
                            }
                        }
                    }

                }//while


            }
            else{

                //Copy the onset list so the original is not changed
                std::vector<std::vector<int>> onset_cpy;
                int i = 0;
                while(i < aig._storage->tt[output].size() && !aig._storage->test_timeout){

                    float current_time = ((float)clock() - start)/CLOCKS_PER_SEC;
                    // std::cout << "check time " << current_time << "\n";
                    // std::cout << "fork_flag = " << fork_flag << "\n";
                    if(current_time >= 180){
                        std::cout << "TIMEOUT at " << current_time << " seconds\n";
                        aig._storage->test_timeout = true;
                        break;
                    }
                    onset_cpy = aig._storage->onset[nodeIdx];
                    //Check to make sure that you only affect rows of the truth table where the node's wanted output
                    //matches in order to merge its onset or offset truth table to the respevtive output
                    if(aig._storage->tt[output].at(i).at(get_index(aig._storage->index[output], nodeIdx)) == wantedOut){

                        if(fork_flag == 1){
                            //Checks the value already stored for the child index
                            //Since these values are added during a BFS, the value already there is more critical
                            //to the output and therefore needs to be matched
                            critical_val1 = aig._storage->tt[output].at(i).at( get_index(aig._storage->index[output], child1Idx));
                            //Erase from the onset truth table, logic combinations that don't match the critical value
                            for(int j = 0; j < onset_cpy.size(); j++){
                                if(onset_cpy.at(j).at(0) != critical_val1){
                                    onset_cpy.erase(onset_cpy.begin() + j);
                                    j--;
                                }
                            }
                            //If the new onset truth table is empty now, the current node's is not valid and should
                            //be erased from the truth table
                            if(onset_cpy.size() == 0){
                                aig._storage->tt[output].erase(aig._storage->tt[output].begin() + i);
                            }
                            else{
                                //Only add values for the right child since the left child is already taken care of
                                aig._storage->tt[output].at(i).at( get_index(aig._storage->index[output], child2Idx)) = onset_cpy.at(0).at(1);

                                int idx = i + 1;
                                //Copy the truth table down to add more combinations for the wanted output if needed
                                for(int j = 1; j < onset_cpy.size(); j++){

                                    aig._storage->tt[output].insert(aig._storage->tt[output].begin() + idx, aig._storage->tt[output].at(i));

                                    aig._storage->tt[output].at(idx).at(get_index(aig._storage->index[output], child2Idx)) = onset_cpy.at(j).at(1);

                                    idx++;
                                }
                                i += onset_cpy.size();
                            }
                        }
                        else if(fork_flag == 2){

                            //Checks the value already stored for the child index
                            //Since these values are added during a BFS, the value already there is more critical
                            //to the output and therefore needs to be matched
                            critical_val2 = aig._storage->tt[output].at(i).at( get_index(aig._storage->index[output], child2Idx));
                            //Erase from the offset truth table, logic combinations that don't match the critical value
                            for(int j = 0; j < onset_cpy.size(); j++){
                                if(onset_cpy.at(j).at(1) != critical_val2){
                                    onset_cpy.erase(onset_cpy.begin() + j);
                                    j--;
                                }
                            }
                            //If the new onset truth table is empty now, the current node's is not valid and should
                            //be erased from the truth table
                            if(onset_cpy.size() == 0){
                                aig._storage->tt[output].erase(aig._storage->tt[output].begin() + i);
                            }
                            else{

                                //Only add values for the left child since the right child is already taken care of
                                aig._storage->tt[output].at(i).at( get_index(aig._storage->index[output], child1Idx)) = onset_cpy.at(0).at(0);

                                int idx = i + 1;
                                //Copy the truth table down to add more combinations for the wanted output if needed
                                for(int j = 1; j < onset_cpy.size(); j++){

                                    aig._storage->tt[output].insert(aig._storage->tt[output].begin() + idx, aig._storage->tt[output].at(i));

                                    aig._storage->tt[output].at(idx).at(get_index(aig._storage->index[output], child1Idx)) = onset_cpy.at(j).at(0);

                                    idx++;
                                }
                                i += onset_cpy.size();
                            }

                        }
                        else if(fork_flag == 3){

                            //Checks the value already stored for both of the children
                            //Since these values are added during a BFS, the value already there is more critical
                            //to the output and therefore needs to be matched
                            critical_val1 = aig._storage->tt[output].at(i).at( get_index(aig._storage->index[output], child1Idx));
                            critical_val2 = aig._storage->tt[output].at(i).at( get_index(aig._storage->index[output], child2Idx));
                            //Erase from the onset truth table, logic combinations that don't match both of the critical values
                            for(int j = 0; j < onset_cpy.size(); j++){
                                if(!((onset_cpy.at(j).at(0) == critical_val1) && (onset_cpy.at(j).at(1) == critical_val2))){
                                    onset_cpy.erase(onset_cpy.begin() + j);
                                    j--;
                                }
                            }
                            //If the new onset truth table is empty now, the current node's is not valid and should
                            //be erased from the truth table
                            if(onset_cpy.size() == 0){
                                aig._storage->tt[output].erase(aig._storage->tt[output].begin() + i);
                            }
                            else{
                                i++;
                            }
                        }
                        else{

                            aig._storage->tt[output].at(i).at( get_index(aig._storage->index[output], child1Idx)) = onset_cpy.at(0).at(0);
                            aig._storage->tt[output].at(i).at( get_index(aig._storage->index[output], child2Idx)) = onset_cpy.at(0).at(1);
                            int idx = i + 1;
                            //Copy the truth table down to add more combinations for the wanted output if needed
                            for(int j = 1; j < onset_cpy.size(); j++){

                                aig._storage->tt[output].insert(aig._storage->tt[output].begin() + idx, aig._storage->tt[output].at(i));

                                aig._storage->tt[output].at(idx).at(get_index(aig._storage->index[output], child1Idx)) = onset_cpy.at(j).at(0);
                                aig._storage->tt[output].at(idx).at(get_index(aig._storage->index[output], child2Idx)) = onset_cpy.at(j).at(1);

                                idx++;
                            }
                            i += onset_cpy.size();
                        }
                    }
                    else{
                        i++;
                    }

                    //Update the wanted outputs for the children of the node
                    for(int i = 0; i < onset_cpy.size(); i++){
                        // std::cout << "onset row: ";
                        // for(int j = 0; j < onset_cpy.at(i).size(); j++){
                        // 	std::cout << onset_cpy.at(i).at(j) << " ";
                        // }
                        // std::cout << "\n";
                        if(fork_flag == 1){
                            if(!is_in_vector(aig._storage->wantedOut[child2Idx], onset_cpy.at(i).at(1))){
                                //std::cout << "1) Onset adding " << onset_cpy.at(i).at(1) << " as wantedOut for " << child2Idx << "\n";
                                aig._storage->wantedOut[child2Idx].push_back(onset_cpy.at(i).at(1));
                            }
                        }
                        else if(fork_flag == 2){
                            if(!is_in_vector(aig._storage->wantedOut[child1Idx], onset_cpy.at(i).at(0))){
                                //std::cout << "2) Onset adding " << onset_cpy.at(i).at(0) << " as wantedOut for " << child1Idx << "\n";
                                aig._storage->wantedOut[child1Idx].push_back(onset_cpy.at(i).at(0));
                            }
                        }
                        else{
                            if(!is_in_vector(aig._storage->wantedOut[child1Idx], onset_cpy.at(i).at(0))){
                                //std::cout << "ELSE) Onset adding " << onset_cpy.at(i).at(0) << " as wantedOut for " << child1Idx << "\n";
                                aig._storage->wantedOut[child1Idx].push_back(onset_cpy.at(i).at(0));
                            }
                            if(!is_in_vector(aig._storage->wantedOut[child2Idx], onset_cpy.at(i).at(1))){
                                //std::cout << "ELSE) Onset adding " << onset_cpy.at(i).at(1) << " as wantedOut for " << child2Idx << "\n";
                                aig._storage->wantedOut[child2Idx].push_back(onset_cpy.at(i).at(1));
                            }
                        }
                    }
                }
            }
        }
        else{//output is constant
            if(aig._storage->onset[nodeIdx].size() > aig._storage->offset[nodeIdx].size())
                aig._storage->tt[output].at(0).at(0) = 1;
            else
                aig._storage->tt[output].at(0).at(0) = 0;

            // std::cout << "constant\n";

        }
        // std::cout << "index = {";
        // for(int i = 0; i < aig._storage->index[output].size(); i++){
        // 	std::cout << aig._storage->index[output].at(i) << " ";
        // }
        // std::cout << "}\n";

        // for(int i = 0; i < aig._storage->tt[output].size(); i++){
        // 	for(int j = 0; j < aig._storage->tt[output].at(i).size(); j++){
        // 		std::cout << aig._storage->tt[output].at(i).at(j) << " ";
        // 	}
        // 	std::cout << "\n";
        // }
    }//add_to_truth()


    /* genTruthBFS
     *
     * @param aig: AIG network that is being traversed
     * @param output: Root node to begin traversal at (output that the truth table is being created for)
     * @param wantedOut: The outputs wanted for a node to make the output high or low
     * @param partition: The partition that the root node belongs to
     *
     * Performs a breadth first search of the network in order to keep nodes visited in topological order and merges
     * each node's truth table to create a truth table for the specified output.
     */
    void genTruthBFS(mockturtle::aig_network aig, int output, std::vector<int> wantedOut, int partition){

        clock_t start = clock();
        std::queue<int> net_queue;
        std::map<int, bool> visited;
        //Set all nodes to be unvisited
        aig.foreach_node( [&]( auto node ) {
            visited[aig.node_to_index(node)] = false;
        });

        net_queue.push(output);
        visited[output] = true;

        while(!net_queue.empty() && !aig._storage->test_timeout){

            int curr_node = net_queue.front();

            wantedOut = aig._storage->wantedOut[curr_node];
            net_queue.pop();
            auto node = aig.index_to_node(curr_node);

            //Flag to make sure that children are not added to the truth table more than once
            bool add_to_truth_flag = true;

            //Make sure that the BFS traversal does not go past the inputs of the partition
            if(!is_in_vector(aig._storage->partitionInputs[partition], curr_node)){


                for(int i = 0; i < aig._storage->nodes[node].children.size(); i++){

                    int childIdx = aig._storage->nodes[node].children[i].index;
                    bool is_valid = true;

                    //Adjust the child index if it is a primary input
                    // if(childIdx < (aig.num_pis() + aig.num_pos())){
                    // 			childIdx--;
                    // }

                    //Make sure a valid child index is found
                    if(childIdx < 0){
                        is_valid = false;
                    }

                    if(!visited[childIdx]){

                        if(is_valid){

                            net_queue.push(childIdx);
                            visited[childIdx] = true;

                            if(add_to_truth_flag && !is_in_vector(aig._storage->partitionInputs[partition], curr_node)){

                                //Add both children of a node to the truth table at once
                                int child1Idx = aig._storage->nodes[node].children[0].index;
                                int child2Idx = aig._storage->nodes[node].children[1].index;

                                int fork_flag;

                                //The first child has not been visited but the second one has
                                //The RIGHT child has a forked connection
                                if(visited[child2Idx]){
                                    aig._storage->index[output].push_back(child1Idx);
                                    fork_flag = 2;
                                }
                                else{//Neither of the children have a forked connection
                                    aig._storage->index[output].push_back(child1Idx);
                                    aig._storage->index[output].push_back(child2Idx);
                                    fork_flag = 0;
                                }

                                for(int j = 0; j < wantedOut.size(); j++){

                                    add_to_truth(aig, curr_node, wantedOut.at(j), output, fork_flag, start);
                                    //Make sure that no node and its children are added to the truth table more than
                                    //once
                                    add_to_truth_flag = false;

                                }
                            }
                        }

                    }
                    else{

                        if(add_to_truth_flag && !is_in_vector(aig._storage->partitionInputs[partition], curr_node)){

                            int child1Idx = aig._storage->nodes[node].children[0].index;
                            int child2Idx = aig._storage->nodes[node].children[1].index;

                            int fork_flag;

                            //If both children have been visited before
                            //BOTH children have forked connections
                            if(visited[child2Idx]){
                                fork_flag = 3;
                            }
                            else{//Only the first (LEFT) child has a forked connection
                                aig._storage->index[output].push_back(child2Idx);
                                fork_flag = 1;
                            }

                            for(int j = 0; j < wantedOut.size(); j++){
                                add_to_truth(aig, curr_node, wantedOut.at(j), output, fork_flag, start);
                                //Make sure that a node and its children are not added to the truth table more than once
                                add_to_truth_flag = false;

                            }
                        }
                    }
                }
            }
        }

        aig._storage->test_runtime[output] = ((float)clock() - start)/CLOCKS_PER_SEC;

    }//genTruthBFS()

    void map_part_io(mockturtle::aig_network aig){
        if(aig._storage-> num_partitions != 0){
            for(int i = 0; i < aig._storage->num_partitions; i++){
                // std::cout << "partition = " << i << "\n";
                aig.foreach_node( [&]( auto node ) {

                    int nodeIdx = aig.node_to_index(node);

                    //Check to see if the current node is in the partition
                    if(aig._storage->partitionMap[nodeIdx] == i){

                        // std::cout << "nodeIdx = " << nodeIdx << "\n";

                        //The current node is in the output list and it is a constant
                        if(aig.is_po(node) && aig.is_constant(node)
                           && !is_in_vector(aig._storage->partitionOutputs[i],nodeIdx)){

                            //std::cout << "OUTPUT\n";
                            aig._storage->partitionOutputs[i].push_back(nodeIdx);
                        }

                        //The current node is connected directly to one primary input
                        // if(aig.is_pi(node) && aig.is_po(node)
                        // 	&& !is_in_vector(aig._storage->partitionOutputs[i],nodeIdx)){
                        // 	// std::cout << "partition = " << i << "\n";
                        // 	// std::cout << "nodeIdx " << nodeIdx << " is po and pi\n";
                        // 	aig._storage->partitionOutputs[i].push_back(nodeIdx);
                        // }

                        if(aig.is_po(node) && aig.is_pi(node)){
                            if(!is_in_vector(aig._storage->partitionInputs[i],nodeIdx))
                                aig._storage->partitionInputs[i].push_back(nodeIdx);
                            if(!is_in_vector(aig._storage->partitionOutputs[i],nodeIdx))
                                aig._storage->partitionOutputs[i].push_back(nodeIdx);
                        }
                        for(int j = 0; j < aig._storage->partitionConn[i][nodeIdx].size(); j++){


                            //Add to the output list for the partition if (the node is a primary output OR (if output connections go to nodes
                            //that are not located in the partition AND it is not already included in the output list)) AND (the node is
                            //not an input to the partition)

                            if( (aig.is_po(node) || !is_in_map(aig._storage->partitionConn[i], aig._storage->partitionConn[i][nodeIdx].at(j)))
                                && !is_in_vector(aig._storage->partitionOutputs[i],nodeIdx)
                                && !aig.is_pi(nodeIdx) && !is_in_vector(aig._storage->partitionInputs[i],nodeIdx)){

                                aig._storage->partitionOutputs[i].push_back(nodeIdx);
                            }
                            //Primary output is tied directly to a primary input

                            int child1Idx = aig._storage->nodes[node].children[0].index;
                            int child2Idx = aig._storage->nodes[node].children[1].index;

                            bool child1 = is_in_map(aig._storage->partitionConn[i], child1Idx);
                            bool child2 = is_in_map(aig._storage->partitionConn[i], child2Idx);

                            // std::cout << "nodeIdx = " << nodeIdx << "\n";
                            //If the node has children, check if either of its children are in the partition but the other one is not
                            //add the child that is not in the partition to the list of inputs
                            if(!aig.is_pi(nodeIdx) && ((child1 && !child2) || (!child1 && child2))){

                                if(!child1 && !is_in_vector(aig._storage->partitionInputs[i],child1Idx)){
                                    // std::cout << "1) child1Idx = " << child1Idx << "\n";
                                    //std::cout << "INPUT\n";
                                    aig._storage->partitionInputs[i].push_back(child1Idx);
                                }
                                else if(!child2 && !is_in_vector(aig._storage->partitionInputs[i],child2Idx)){
                                    // std::cout << "1) child2Idx = " << child2Idx << "\n";
                                    //std::cout << "INPUT\n";
                                    aig._storage->partitionInputs[i].push_back(child2Idx);
                                }
                            }
                            //If the node is a primary input and it is not already in the list of inputs, add
                            //the node to the input list
                            if(aig.is_pi(nodeIdx) && !is_in_vector(aig._storage->partitionInputs[i],nodeIdx)){
                                // std::cout << "2) nodeIdx = " << nodeIdx << "\n";
                                //std::cout << "INPUT\n";
                                aig._storage->partitionInputs[i].push_back(nodeIdx);
                            }
                            //If the node is not a primary input and NEITHER children are in the partition,
                            //add both to the input list if they are not already there
                            if (!aig.is_pi(nodeIdx) && !child1 && !child2){

                                if(!is_in_vector(aig._storage->partitionInputs[i],child1Idx)){
                                    // std::cout << "3) child1Idx = " << child1Idx << "\n";
                                    //std::cout << "INPUT\n";
                                    aig._storage->partitionInputs[i].push_back(child1Idx);
                                }
                                if(!is_in_vector(aig._storage->partitionInputs[i],child2Idx)){
                                    // std::cout << "3) child2Idx = " << child2Idx << "\n";
                                    //std::cout << "INPUT\n";
                                    aig._storage->partitionInputs[i].push_back(child2Idx);
                                }

                            }


                            //If neither of the children of a node are in the partition or the node is a primary input and is not yet in
                            //the list of inputs, add to the list of inputs
                            // if( ( (!child1 && !child2) || aig.is_pi(node) ) && !is_in_vector(aig._storage->partitionInputs[i],nodeIdx)){

                            // 	aig._storage->partitionInputs[i].push_back(nodeIdx);
                            // }
                        }
                    }
                });

                std::cout << "Partition " << i << " Inputs: {";
                for(int j = 0; j < aig._storage->partitionInputs[i].size(); j++){
                    std::cout << aig._storage->partitionInputs[i].at(j) << " ";
                }
                std::cout << "}\n";
                std::cout << "Partition " << i << " Outputs: {";
                for(int j = 0; j < aig._storage->partitionOutputs[i].size(); j++){
                    std::cout << aig._storage->partitionOutputs[i].at(j) << " ";
                }
                std::cout << "}\n";
            }

        }
        else{
            std::cout << "Partitions not mapped yet\n";
        }
    }

    //Simple BFS Traversal to optain the depth of an output's logic cone before the truth table is built
    void BFS_traversal(mockturtle::aig_network aig, int output, int partition){
        std::queue<int> net_queue;
        std::queue<int> depth_queue;
        std::map<int, bool> visited;
        //Set all nodes to be unvisited
        aig.foreach_node( [&]( auto node ) {
            visited[aig.node_to_index(node)] = false;
        });

        net_queue.push(output);
        visited[output] = true;

        depth_queue.push(0);

        while(!net_queue.empty()){

            int curr_node = net_queue.front();
            int depth = depth_queue.front();
            net_queue.pop();
            depth_queue.pop();
            auto node = aig.index_to_node(curr_node);

            //Make sure that the BFS traversal does not go past the inputs of the partition
            if(!is_in_vector(aig._storage->partitionInputs[partition], curr_node)){

                for(int i = 0; i < aig._storage->nodes[node].children.size(); i++){

                    int childIdx = aig._storage->nodes[node].children[i].index;
                    bool is_valid = true;
                    //Make sure a valid child index is found
                    if(childIdx < 0){
                        is_valid = false;
                    }

                    if(!visited[childIdx]){

                        if(is_valid){

                            net_queue.push(childIdx);
                            depth_queue.push(depth + 1);
                            aig._storage->output_cone_depth[output] = depth + 1;
                            visited[childIdx] = true;

                        }

                    }

                }
            }
            else{
                aig._storage->logic_cone_inputs[output].push_back(curr_node);
            }
        }

    }//BFS_traversal()

    /*
     * Makes the stored truth table for a given output ready for writing to
     * a blif file by deleting columns that don't correspond to inputs of
     * the given partition
     */
    void finalize_truth(mockturtle::aig_network aig, int output, int partition){

        // std::cout << "partition inputs = ";
        // for(int i = 0; i < aig._storage->partitionInputs[partition].size(); i++){
        // 	std::cout << aig._storage->partitionInputs[partition].at(i) << " ";
        // }
        // std::cout << "\n";

        // std::cout << "tt size = " << aig._storage->tt[output].at(0).size() << "\n";
        // std::cout << "index size = " << aig._storage->index[output].size() << "\n";
        if(aig._storage->tt[output].at(0).size() > aig._storage->index[output].size()){
            for(int i = 0; i < aig._storage->tt[output].size(); i++){
                for(int j = aig._storage->index[output].size(); j < aig._storage->tt[output].at(i).size(); j++){
                    aig._storage->tt[output].at(i).erase(aig._storage->tt[output].at(i).begin() + j);
                    j--;

                }

            }
        }

        for(int i = 0; i < aig._storage->index[output].size(); i++){


            if(!aig.is_constant(aig._storage->index[output].at(i)) && !is_in_vector(aig._storage->partitionInputs[partition], aig._storage->index[output].at(i))){

                for(int j = 0; j < aig._storage->tt[output].size(); j++){
                    aig._storage->tt[output].at(j).erase(aig._storage->tt[output].at(j).begin() + i);

                }
                aig._storage->index[output].erase(aig._storage->index[output].begin() + i);
                i--;
            }
        }

        // for(int idx = 0; idx < aig._storage->tt[output].size(); idx++){
        // 	std::cout << "tt.at(" << idx << ") ";
        // 	for(int jdx = 0; jdx < aig._storage->tt[output].at(idx).size(); jdx++){
        // 		 std::cout << aig._storage->tt[output].at(idx).at(jdx);
        // 	}
        // 	std::cout << "\n";
        // }

        for(int i = 0; i < aig._storage->tt[output].size(); i++){
            for(int j = (i + 1); j < aig._storage->tt[output].size(); j++){
                if(aig._storage->tt[output].at(i) == aig._storage->tt[output].at(j)){
                    aig._storage->tt[output].erase(aig._storage->tt[output].begin() + j);
                    j--;
                }
            }
        }
        // std::cout << "deleting duplicates\n";
        // for(int idx = 0; idx < aig._storage->tt[output].size(); idx++){
        // 	std::cout << "tt.at(" << idx << ") ";
        // 	for(int jdx = 0; jdx < aig._storage->tt[output].at(idx).size(); jdx++){
        // 		 std::cout << aig._storage->tt[output].at(idx).at(jdx);
        // 	}
        // 	std::cout << "\n";
        // }
    }//finalize_truth()



    void blif_write(mockturtle::aig_network aig, int partition, int output_wanted, int output_num){

        time_t rawtime;
        struct tm * timeinfo;

        time (&rawtime);
        timeinfo = localtime (&rawtime);

        std::string filename = aig._storage->net_name;
        filename.append("_part_" + std::to_string(partition) + ".blif");

        std::ofstream output("./" + aig._storage->net_name + "_part/" + filename);
        output << "# Blif file written by lstools on " << asctime(timeinfo);
        output << ".model " << aig._storage->net_name << "\n";
        output << ".inputs ";

        for(int i = 0; i < aig._storage->partitionInputs[partition].size(); i++){
            int curr_input = aig._storage->partitionInputs[partition].at(i);
            int corrected_input = curr_input;
            std::string index = std::to_string(curr_input);
            if(aig.is_pi(curr_input)){
                index = aig._storage->inputNames[curr_input - 1];
            }

            output << index << " ";
        }
        output << "\n";
        output << ".outputs ";

        // std::cout << "partition inputs = ";
        // for(int i = 0; i < aig._storage->partitionInputs[partition].size(); i++){
        // 	std::cout << aig._storage->partitionInputs[partition].at(i) << " ";
        // }
        // std::cout << "\n";
        // std::cout << "partition outputs = ";
        // for(int i = 0; i < aig._storage->partitionOutputs[partition].size(); i++){
        // 	std::cout << aig._storage->partitionOutputs[partition].at(i) << " ";
        // }
        // std::cout << "\n";

        for(int i = 0; i < aig._storage->partitionOutputs[partition].size(); i++){

            int curr_output = aig._storage->partitionOutputs[partition].at(i);
            std::string output_str = std::to_string(curr_output);
            int corrected_output_num = curr_output;
            // std::cout << "curr_output = " << curr_output << "\n";
            if(aig.is_po(curr_output)){

                output_str = aig._storage->outputNames[get_output_index(aig, curr_output)];
            }
            else {
                if(curr_output < (aig.num_pis() + aig.num_pos()))
                    curr_output++;
                output_str = std::to_string(curr_output);


            }
            output << output_str << " ";
            // std::cout << "output_str = " << output_str << "\n";

        }
        output << "\n";


        for(int i = 0; i < aig._storage->partitionOutputs[partition].size(); i++){
            int curr_output = aig._storage->partitionOutputs[partition].at(i);

            std::string output_str = std::to_string(curr_output);
            int corrected_output_num = curr_output;
            // std::cout << "curr_output = " << curr_output << "\n";
            if(aig.is_po(curr_output)){
                output_str = aig._storage->outputNames[get_output_index(aig, curr_output)];
            }
            else if(is_in_vector(aig._storage->partitionInputs[partition], curr_output)){
                corrected_output_num++;
                output_str = std::to_string(corrected_output_num);
            }
            // std::cout << "output_str = " << output_str << "\n";

            output << ".names ";

            for(int j = 0; j < aig._storage->index[curr_output].size(); j++){
                int curr_index = aig._storage->index[curr_output].at(j);
                std::string index;
                if(aig.is_pi(curr_index))
                    index = aig._storage->inputNames[curr_index - 1];
                else if(aig.is_po(curr_index)){
                    index = aig._storage->outputNames[get_output_index(aig, curr_index)];
                }
                else
                    index = std::to_string(curr_index);

                output << index << " ";
            }

            output << output_str << "\n";

            for(int j = 0; j < aig._storage->tt[curr_output].size(); j++){

                for(int k = 0; k < aig._storage->tt[curr_output].at(j).size(); k++){
                    output << aig._storage->tt[curr_output].at(j).at(k);
                }

                output << " " << output_wanted << "\n";

            }
        }
        output << ".end\n";

        output.close();



    }

    void blif_test(mockturtle::aig_network aig, int output_wanted){

        time_t rawtime;
        struct tm * timeinfo;

        time (&rawtime);
        timeinfo = localtime (&rawtime);

        std::vector<int> outputs_defined;
        std::string filename = aig._storage->net_name;
        filename.append("_test.blif");

        std::ofstream output;
        output.open (filename);
        output << "# Blif file written by lstools on " << asctime(timeinfo);
        output << ".model " << aig._storage->net_name << "\n";

        output << ".inputs ";
        for(int i = 0; i < aig.num_pis(); i++){
            output << aig._storage->inputNames[i] << " ";
        }
        output << "\n";

        output << ".outputs ";
        for(int i = 0; i < aig.num_pos(); i++){
            output << aig._storage->outputNames[i] << " ";
        }
        output << "\n";

        for(int idx = 0; idx < aig._storage->num_partitions; idx++){
            int partition = idx;
            //std::cout << "\npartition = " << partition << "\n\n";
            for(int i = 0; i < aig._storage->partitionOutputs[partition].size(); i++){
                int curr_output = aig._storage->partitionOutputs[partition].at(i);
                // std::cout << "curr_output = " << curr_output << "\n";
                std::string output_str;
                if(aig.is_po(curr_output)){
                    output_str = aig._storage->outputNames[get_output_index(aig, curr_output)];
                }
                else {
                    output_str = std::to_string(curr_output);
                }

                output << ".names ";

                //primary output is connected directly to a primary input
                // if (aig.is_pi(curr_output) && aig.is_po(curr_output)){
                // 	output << aig._storage->inputNames[curr_output] << " " << output_str << "\n";
                // 	output << output_wanted << " " << output_wanted << "\n";

                // }

                //else{
                // std::cout << "partition = " << idx << "\n";
                // std::cout << "curr_output = " << curr_output << "\n";
                // std::cout << "index = {";
                // for(int j = 0; j < aig._storage->index[curr_output].size(); j++){
                // 	std::cout << aig._storage->index[curr_output].at(j) << " ";
                // }
                // std::cout << "}\n";
                for(int j = 0; j < aig._storage->index[curr_output].size(); j++){
                    int curr_index = aig._storage->index[curr_output].at(j);
                    //std::cout << "curr_index = " << curr_index << "\n";
                    std::string index;
                    if(aig.is_pi(curr_index))
                        index = aig._storage->inputNames[curr_index - 1];
                    else if(aig.is_po(curr_index)){

                        index = aig._storage->outputNames[get_output_index(aig, curr_index)];
                    }
                        //primary output connected directly to primary input
                        // else if(aig.is_po(curr_index) && aig.is_pi(curr_index)){
                        // 	//guaranteed to have 2 items in the index
                        // }
                    else
                        index = std::to_string(curr_index);

                    output << index << " ";
                }
                if(!aig.is_constant(curr_output))
                    output << output_str << "\n";
                else
                    output << "\n";

                for(int j = 0; j < aig._storage->tt[curr_output].size(); j++){

                    for(int k = 0; k < aig._storage->tt[curr_output].at(j).size(); k++){
                        output << aig._storage->tt[curr_output].at(j).at(k);
                    }

                    if(!aig.is_constant(curr_output))
                        output << " " << output_wanted << "\n";
                    else
                        output << "\n";

                }
                //}
            }
        }



        output << ".end\n";

        output.close();

    }

    mockturtle::aig_network simplify_merged(mockturtle::aig_network merged_aig){

        std::vector<std::string> unique_inputs;
        std::map<std::string, int> unique_inputs_indeces;
        std::vector<int> duplicate_inputs_indeces;
        std::map<int, std::string> duplicate_inputs;
        std::vector<int> orig_input_indeces;
        mockturtle::aig_network simplified;
        merged_aig._storage->nodes.reserve(merged_aig.size());

        for(int i = 0; i < merged_aig._storage->inputs.size(); i++){
            int nodeIdx = i;
            std::string name = merged_aig._storage->inputNames[nodeIdx];
            if(!is_in_string_vector(unique_inputs, name)){
                std::cout << merged_aig._storage->inputNames[nodeIdx] << " with index " << nodeIdx << " is unique\n";
                std::cout << "updated index = " << nodeIdx - duplicate_inputs.size() << "\n";
                unique_inputs.push_back(name);
                orig_input_indeces.push_back(nodeIdx);
                unique_inputs_indeces[name] = nodeIdx - duplicate_inputs.size();
            }
            else{
                std::cout << merged_aig._storage->inputNames[nodeIdx] << " with index " << nodeIdx << " is duplicated\n";
                duplicate_inputs_indeces.push_back(nodeIdx);
                duplicate_inputs[nodeIdx] = name;
            }
        }

        for(int i = 0; i < duplicate_inputs_indeces.size(); i++){
            int dup_idx = duplicate_inputs_indeces.at(i);
            std::string dup_name = duplicate_inputs[dup_idx];
            int new_idx = unique_inputs_indeces[dup_name];

            std::cout << dup_name << " at " << dup_idx << " -> " << new_idx + 1 << "\n";

            mockturtle::fanout_view fanout_aig{merged_aig};
            std::set<mockturtle::node<mockturtle::aig_network>> nodes;
            fanout_aig.foreach_fanout(merged_aig.index_to_node(dup_idx), [&](const auto& p){
                nodes.insert(p);
            });

            for(std::set<mockturtle::node<mockturtle::aig_network>>::iterator it = nodes.begin(); it != nodes.end(); ++it){
                std::cout << merged_aig.node_to_index(*it) << " child[0]: " << merged_aig._storage->nodes[*it].children[0].index << "\n";
                std::cout << merged_aig.node_to_index(*it) << " child[1]: " << merged_aig._storage->nodes[*it].children[1].index << "\n";
                if(merged_aig._storage->nodes[*it].children[0].index == dup_idx){
                    merged_aig._storage->nodes[*it].children[0].index = new_idx + 1;
                }
                else if(merged_aig._storage->nodes[*it].children[1].index == dup_idx){
                    merged_aig._storage->nodes[*it].children[1].index = new_idx + 1;
                }
            }


        }
        std::cout << "UNIQUE\n";
        for(int i = 0; i < unique_inputs.size(); i++){
            std::string name = unique_inputs.at(i);
            int old_idx = orig_input_indeces.at(i);
            int new_idx = unique_inputs_indeces[name];

            std::cout << name << " at " << old_idx << " -> " << new_idx + 1 << "\n";

            mockturtle::fanout_view fanout_aig{merged_aig};
            std::set<mockturtle::node<mockturtle::aig_network>> nodes;
            fanout_aig.foreach_fanout(merged_aig.index_to_node(old_idx), [&](const auto& p){
                nodes.insert(p);
            });

            for(std::set<mockturtle::node<mockturtle::aig_network>>::iterator it = nodes.begin(); it != nodes.end(); ++it){
                std::cout << merged_aig.node_to_index(*it) << " child[0]: " << merged_aig._storage->nodes[*it].children[0].index << "\n";
                std::cout << merged_aig.node_to_index(*it) << " child[1]: " << merged_aig._storage->nodes[*it].children[1].index << "\n";
                if(merged_aig._storage->nodes[*it].children[0].index == old_idx){
                    merged_aig._storage->nodes[*it].children[0].index = new_idx + 1;
                }
                else if(merged_aig._storage->nodes[*it].children[1].index == old_idx){
                    merged_aig._storage->nodes[*it].children[1].index = new_idx + 1;
                }
            }


        }

        std::map<int, std::string> orig_inputNames = merged_aig._storage->inputNames;
        int erase_offset = 0;
        for(int i = 0; i < duplicate_inputs_indeces.size(); i++){
            int dup_idx = duplicate_inputs_indeces.at(i);
            std::cout << "erasing " << duplicate_inputs[dup_idx] << " at " << dup_idx - erase_offset << "\n";

            merged_aig._storage->nodes.erase(merged_aig._storage->nodes.begin() + dup_idx - erase_offset);
            erase_offset++;
        }
        merged_aig._storage->inputNames.clear();
        std::vector<std::size_t> new_inputs;
        for(int i = 0; i < unique_inputs.size(); i++){
            std::string name = unique_inputs.at(i);
            int nodeIdx = unique_inputs_indeces[name];
            std::cout << "unique name = " << name << " at index " << nodeIdx << "\n";

            new_inputs.push_back(i + 1);
            simplified._storage->inputNames[i] = name;
        }
        simplified._storage->inputs = new_inputs;
        simplified._storage->nodes = merged_aig._storage->nodes;
        simplified._storage->outputs = merged_aig._storage->outputs;
        simplified._storage->outputNames = merged_aig._storage->outputNames;
        return simplified;

    }

    bool check_for_duplicates(std::vector<int> vec, int nodeIdx){

        std::vector<int>::iterator first_occurance = std::find(vec.begin(),vec.end(),nodeIdx);
        if(first_occurance != vec.end()){
            if(std::find(first_occurance,vec.end(),nodeIdx) != vec.end())
                return true;
            else
                return false;
        }
        else
            return false;
    }

    int computeLevelMig( mockturtle::mig_network mig, int index ) {
        //if node not visited
        if(mig._storage->nodes[index].data[1].h1==0) {

            //set node as visited
            mig._storage->nodes[index].data[1].h1=1;

            //if is input
            if (mig.is_ci(index)) {
                return 0;
            }

			auto inIdx3 = mig._storage->nodes[index].children[2].data;
			if (inIdx3 & 1)
				inIdx3 = inIdx3 - 1;

			//calculate input node index
			auto inNode3 = inIdx3 >> 1;
			int levelNode3 = computeLevelMig(mig, inNode3);

            auto inIdx2 = mig._storage->nodes[index].children[1].data;
            if (inIdx2 & 1)
                inIdx2 = inIdx2 - 1;

            //calculate input node index
            auto inNode1 = inIdx2 >> 1;
            int levelNode1 = computeLevelMig(mig, inNode1);

            auto inIdx = mig._storage->nodes[index].children[0].data;
            if (inIdx & 1)
                inIdx = inIdx - 1;

            //calculate input node index
            auto inNode0 = inIdx >> 1;
            int levelNode0 = computeLevelMig(mig, inNode0);

            int maximo = std::max(levelNode0, levelNode1);
            int max = std::max(maximo, levelNode3);

            int level = 1 + max;
            return level;
        }
    }

    void dfs (mockturtle::aig_network aig, uint64_t index, UnionFind uf){
        if(aig._storage->nodes[index].data[1].h1==0){
			//set node as visited
            aig._storage->nodes[index].data[1].h1=1;

            //traverse the left nodes to the PIs
            if(!aig.is_pi(aig._storage->nodes[index].children[0].index) && index > aig.num_pis() ){
            	auto inData0 = aig._storage->nodes[index].children[0].data;
            	//treat inverted and inputs
				if( inData0 & 1)
					inData0 = aig._storage->nodes[index].children[0].data - 1;

                //calculate input node index
                auto inIndex = inData0 >> 1;

                //ignores latches and PIs
                if(inIndex > aig.num_pis()) {
					uf.merge(index, inIndex);
					dfs(aig, inIndex, uf);
				}
            }

			//traverse the right nodes to the PIs
			if(!aig.is_pi(aig._storage->nodes[index].children[1].index) && index > aig.num_pis() ){
				auto inData1 = aig._storage->nodes[index].children[1].data;
				if( inData1 & 1)
					inData1 = aig._storage->nodes[index].children[1].data - 1;

				//calculate input node index
				auto inIndex = inData1 >> 1;

				//im ignoring latches
				if(inIndex > aig.num_pis()) {
					uf.merge(index, inIndex);
					dfs(aig, inIndex, uf);
				}
			}
        }
    }

	void stats (mockturtle::aig_network aig, uint64_t index, UnionFind uf, std::unordered_map<int, int> &numb_nodes){
		if(aig._storage->nodes[index].data[1].h1==0) {
			//set node as visited
			aig._storage->nodes[index].data[1].h1 = 1;

			std::unordered_map<int, int>::iterator it = numb_nodes.find(uf.find(index));
			if (it != numb_nodes.end()) {
				//std::cout << "Incrementing " << uf.find(index) << " for node index " << index << std::endl;
				it->second++;
			} else {
				numb_nodes.insert(std::make_pair(uf.find(index), 1));
			}

			//traverse the left nodes to the PIs
			if (!aig.is_pi(aig._storage->nodes[index].children[0].index) ) {
				if (aig._storage->nodes[index].children[0].data & 1)
					aig._storage->nodes[index].children[0].data = aig._storage->nodes[index].children[0].data - 1;

				//calculate input node index
				auto inIndex = aig._storage->nodes[index].children[0].data >> 1;

				//im ignoring latches
				if (inIndex > aig.num_pis()) {
					stats(aig, inIndex, uf, numb_nodes);
				}
			}

			//traverse the right nodes to the PIs
			if (!aig.is_pi(aig._storage->nodes[index].children[1].index) ) {
				if (aig._storage->nodes[index].children[1].data & 1)
					aig._storage->nodes[index].children[1].data = aig._storage->nodes[index].children[1].data - 1;
				//calculate input node index
				auto inIndex = aig._storage->nodes[index].children[1].data >> 1;
				//im ignoring latches
				if (inIndex > aig.num_pis()) {
					stats(aig, inIndex, uf, numb_nodes);
				}
			}
		}
	}

	void compute_cone(mockturtle::aig_network aig, uint64_t index, std::unordered_map<int, int> &nodes, int outindex, std::unordered_map<int, int> &ins){
		if(aig._storage->nodes[index].data[1].h1==0){

			//increment number of nodes in this cone
			std::unordered_map<int, int>::iterator it = nodes.find(outindex);

			if(it!=nodes.end() && index > aig.num_pis()) {
				//increment the number of nodes
				it->second++;
			}

			//set node as visited
			aig._storage->nodes[index].data[1].h1=1;

			//traverse one side to the PIs
			if(!aig.is_pi(aig._storage->nodes[index].children[0].index) && index > aig.num_pis() ){
				if(aig._storage->nodes[index].children[0].data & 1)
					aig._storage->nodes[index].children[0].data = aig._storage->nodes[index].children[0].data - 1;

				//calculate input node index
				auto inIndex = aig._storage->nodes[index].children[0].data >> 1;

				//im ignoring latches
				if(inIndex > aig.num_pis()) {
					//call recursion
					compute_cone(aig, inIndex, nodes, outindex, ins);
				}
			}

			//traverse the other side to the PIs
			if(!aig.is_pi(aig._storage->nodes[index].children[1].index)&& index > aig.num_pis() ){
				if(aig._storage->nodes[index].children[1].data & 1)
					aig._storage->nodes[index].children[1].data = aig._storage->nodes[index].children[1].data - 1;

				//calculate input node index
				auto inIndex = aig._storage->nodes[index].children[1].data >> 1;

				//im ignoring latches
				if(inIndex > aig.num_pis()) {
					//call recursion
					compute_cone(aig, inIndex, nodes, outindex, ins);
				}
			}

			//if my child is PI and was not visited yet, I increase the input counter
			if(aig.is_ci(aig._storage->nodes[index].children[0].index) && aig._storage->nodes[aig._storage->nodes[index].children[0].index].data[1].h1 == 0 ){
				aig._storage->nodes[aig._storage->nodes[index].children[0].index].data[1].h1 = 1;

				std::unordered_map<int, int>::iterator it = ins.find(outindex);
				if(it!=ins.end()) {
					//increment the number of inputs
					it->second++;
				}
			}

			//if my other child is PI and was not visited yet, I also increase the input counter
			if(aig.is_ci(aig._storage->nodes[index].children[1].index) && aig._storage->nodes[aig._storage->nodes[index].children[1].index].data[1].h1 == 0){
				aig._storage->nodes[aig._storage->nodes[index].children[1].index].data[1].h1 = 1;

				std::unordered_map<int, int>::iterator it = ins.find(outindex);
				if(it!=ins.end()) {
					//increment the number of inputs
					it->second++;
				}
			}
		}
	}



	/* Adds And-inverter graphs (Mockturtle type aig_network) as store element type to
	 * alice.
	 *
	 * One can access AIGs in general store commands using the long --aig flag or
	 * the short -a flag.
	 */
	ALICE_ADD_STORE( mockturtle::aig_network, "aig", "a", "aig", "AIGs" )

	/* Implements the short string to describe a store element in store -a */
	ALICE_DESCRIBE_STORE( mockturtle::aig_network, aig ){

	  const auto name = "aig_placeholder";
	  const auto pi_num = aig.num_pis();
	  const auto po_num = aig.num_pos();
	  return fmt::format( "{} i/o = {}/{}", name, pi_num, po_num );
	}//end aig_network describe store

	ALICE_LOG_STORE_STATISTICS( mockturtle::aig_network, aig){

	  return {
	  	{"nodes", aig.size()},
	  	{"inputs", aig.num_pis() - aig.num_latches()},
	  	{"latches", aig.num_latches()},
	  	{"outputs", aig.num_pos() - aig.num_latches()},
	  	{"AIG nodes", aig.num_gates()}};
	}//end aig_network log store statistics

	/* Implements the functionality of ps -b */
	ALICE_PRINT_STORE_STATISTICS( mockturtle::aig_network, os, aig ){
		os << "nodes: " << aig.size() << std::endl;
	  	os << "inputs: " << aig.num_pis() - aig.num_latches() << std::endl;
		os << "latches: " << aig.num_latches() << std::endl;
		os << "outputs: " << aig.num_pos() - aig.num_latches() << std::endl;
	  	os << "AIG nodes: " << aig.num_gates() << std::endl;

	}//end aig_network print store statistics

	ALICE_ADD_STORE( mockturtle::mig_network, "mig", "m", "mig", "MIGs" )

	/* Implements the short string to describe a store element in store -a */
	ALICE_DESCRIBE_STORE( mockturtle::mig_network, mig ){

		const auto name = "mig_placeholder";
		const auto pi_num = mig.num_pis();
		const auto po_num = mig.num_pos();

		return fmt::format( "{} i/o = {}/{}", name, pi_num, po_num );
	}//end aig_network describe store

	ALICE_LOG_STORE_STATISTICS( mockturtle::mig_network, mig){

		return {
				{"nodes", mig.size()},
				{"inputs", mig.num_pis() - mig.num_latches()},
				{"latches", mig.num_latches()},
				{"outputs", mig.num_pos() - mig.num_latches()},
				{"MAJ nodes", mig.num_gates()}};
	}//end aig_network log store statistics

	/* Implements the functionality of ps -b */
	ALICE_PRINT_STORE_STATISTICS( mockturtle::mig_network, os, mig ){
		os << "nodes: " << mig.size() << std::endl;
		os << "inputs: " << mig.num_pis() - mig.num_latches() << std::endl;
		os << "latches: " << mig.num_latches() << std::endl;
		os << "outputs: " << mig.num_pos() - mig.num_latches()<< std::endl;
		os << "MAJ nodes: " << mig.num_gates() << std::endl;
	}//end aig_network print store statistics


	ALICE_ADD_STORE(mockturtle::klut_network, "bench", "b", "BENCH", "BENCHes")

	ALICE_DESCRIBE_STORE(mockturtle::klut_network, bench){
	  const auto name = "bench_placeholder";
	  const auto pi_num = bench.num_pis();
	  const auto po_num = bench.num_pos();
	  return fmt::format( "{} i/o = {}/{}", name, pi_num, po_num );
	}//end klut_network describe store

	ALICE_LOG_STORE_STATISTICS( mockturtle::klut_network, bench){

	  return {
	      {"inputs", bench.num_pis()},
	      {"outputs", bench.num_pos()},
	      {"nodes", bench.size()},
	      {"gates", bench.num_gates()}};
	}//end klut_network log store statistics

	/* Implements the functionality of ps -b */
	ALICE_PRINT_STORE_STATISTICS( mockturtle::klut_network, os, bench ){

	  os << "inputs: " << bench.num_pis() << std::endl;
	  os << "outputs: " << bench.num_pos() << std::endl;
	  os << "nodes: " << bench.size() << std::endl;
	  os << "gates: " << bench.num_gates() << std::endl;
	}//end klut_network print store statistics

	ALICE_ADD_STORE(abc::Gia_Man_t*, "gia", "g", "AIG", "AIGs")

    /* Implements the short string to describe a store element in store -a */
    ALICE_DESCRIBE_STORE( abc::Gia_Man_t*, gia )
    {
        const auto name = abc::Gia_ManName( gia );
        const auto pi_num = abc::Gia_ManPiNum( gia );
        const auto po_num = abc::Gia_ManPoNum( gia );
        return fmt::format( "{} i/o = {}/{}", name, pi_num, po_num );
    }

    /* Implements the functionality of ps -a */
    ALICE_PRINT_STORE_STATISTICS( abc::Gia_Man_t*, os, gia )
    {
        abc::Gps_Par_t Pars{};
        abc::Gia_ManPrintStats( gia, &Pars );
    }

    /* Implements the log returned by ps -a */
    ALICE_LOG_STORE_STATISTICS( abc::Gia_Man_t*, gia )
    {
        return {
                {"name", abc::Gia_ManName( gia )},
                {"inputs", abc::Gia_ManPiNum( gia )},
                {"outputs", abc::Gia_ManPoNum( gia )},
                {"nodes", abc::Gia_ManAndNum( gia )},
                {"latches"}, abc::Gia_ManRegNum(gia),
                {"levels", abc::Gia_ManLevelNum( gia )}};
    }

    ALICE_ADD_FILE_TYPE( aiger, "Aiger" )

/* Implements the functionality of read_aiger -a */
    ALICE_READ_FILE( abc::Gia_Man_t*, aiger, filename, cmd )
    {
        return abc::Gia_AigerRead( (char*)filename.c_str(), 0, 0, 0 );
    }

/* Implements the functionality of write_aiger -a */
    ALICE_WRITE_FILE( abc::Gia_Man_t*, aiger, gia, filename, cmd )
    {
        abc::Gia_AigerWrite( gia, (char*)filename.c_str(), 1, 0 );
    }

/* Implements the command syn3 */
    ALICE_COMMAND( syn3, "Optimization", "Performs AIG optimization" )
    {
        auto& gia = store<abc::Gia_Man_t*>().current();

        auto aig_new = abc::Gia_ManAigSyn3( gia, 0, 0 );
        abc::Gia_ManStop( gia );
        gia = aig_new;
    }

	/*Reads an blif file and stores the CBA network in a store*/
	ALICE_COMMAND( get_blif, "Input", "Uses the lorina library to read in a blif file" ){

		std::cout << "Not yet fully implemented\n";

		/*std::string filename = "";
		std::cout << "Enter blif filename: ";
		std::cin >> filename;
		read_blif(filename, blif());*/
		/*TODO check to make sure that the file entered is valid and add store*/
	}//end get_blif

	class get_mig_command : public alice::command{

    public:
      explicit get_mig_command( const environment::ptr& env )
          : command( env, "Uses the lorina library to read in an aig file into a mig network" ){

        opts.add_option( "--filename,filename", filename, "AIG file to read in as an MIG" )->required();
      }

    protected:
        void execute(){

            if(checkExt(filename, "aig")){
                mockturtle::mig_network mig;
                lorina::read_aiger(filename, mockturtle::aiger_reader( mig ));
                std::cout << "AIG file = " << filename << " stored" << std::endl;
                store<mockturtle::mig_network>().extend() = mig;

                filename.erase(filename.end() - 4, filename.end());
                mig._storage->net_name = filename;

            }
            else{
                std::cout << "Not a valid AIG file\n";
            }
        }
    private:
      std::string filename{};
    };

    ALICE_ADD_COMMAND(get_mig, "Input");

ALICE_COMMAND( read_aig, "Input", "Uses the lorina library to read in an aig file" ){
	  std::string filename = "";
    std::cout << "Enter aig filename: ";
    std::cin >> filename;

    if(checkExt(filename, "aig")){
        mockturtle::aig_network aig;
        lorina::read_aiger(filename, mockturtle::aiger_reader( aig ));
        std::cout << "AIG file = " << filename << " stored" << std::endl;
        store<mockturtle::aig_network>().extend() = aig;
    }
}

    class write_aig_command : public alice::command{

    public:
      explicit write_aig_command( const environment::ptr& env )
          : command( env, "Writes the AIG in the ASCII format" ){

        opts.add_option( "--filename,filename", filename, "AAG file to write stored AIG to" )->required();
      }

    protected:
      void execute(){
        if(checkExt(filename, "aag")){
            if(!store<mockturtle::aig_network>().empty()){
                auto aig = store<mockturtle::aig_network>().current();

                std::ofstream aigfile;
                aigfile.open (filename);

                // header info - MILOA
                auto _num_inputs = aig.num_pis();
                auto _num_latches = aig.num_latches();
                auto _num_outputs = aig.num_pos();
                auto _num_ands = aig.num_gates();
                auto _num_vertices = aig.num_pis() + aig.num_gates();

                //write aig header
                aigfile << "aag " << _num_vertices << " " << _num_inputs - _num_latches << " " << _num_latches << " " << (_num_outputs - _num_latches) << " " << _num_ands << std::endl;

                aig.foreach_pi([&] (auto node){
                    auto index = aig.pi_index(node);
                    aigfile << (index+1)*2 << "\n";
                });

                //write aig latches
                auto lineIdx = ((aig.num_pis()-aig.num_latches())*2)+2;
                for(int i=0; i<aig.num_latches(); i++){
                    auto regIdx = aig.num_pos() - aig.num_latches() + i;
                    aigfile << lineIdx << " " << aig._storage->outputs[regIdx].data << "\n";
                    lineIdx += 2;
                }

                //write aig outputs
                for(int i=0; i<aig.num_pos()- aig.num_latches(); i++){
                    aigfile << aig._storage->outputs[i].data << "\n";
                }

                auto skipLatches = 0;
                aig.foreach_gate([&] (auto node){
                    //skip latches in the nodes vector
                    if(skipLatches>=aig.num_latches()) {
                        auto index = aig.node_to_index(node);
                        auto left = aig._storage->nodes[index].children[0].data;
                        auto right = aig._storage->nodes[index].children[1].data;
                        aigfile << index*2 << " " << left << " " << right << "\n";
                    }
                    skipLatches+=1;
                });

                for(int i = 0; i < aig._storage->inputs.size(); i++){
                    aigfile << "i" << i << " " << aig._storage->inputNames[aig._storage->inputs.at(i) - 1] << "\n";
                }
                for(int i = 0; i < aig._storage->outputs.size(); i++){
                    aigfile << "o" << i << " " << aig._storage->outputNames[i] << "\n";
                }

                aigfile.close();
            }
            else{
                std::cout << "No AIG stored\n";
            }
        }
        else{
            std::cout << "File not a vlid aag file\n";
        }
      }

    private:
      std::string filename{};
    };

    ALICE_ADD_COMMAND(write_aig, "Output");

	/*Reads a bench file and store the k-LUT network in a store*/
/*
	ALICE_COMMAND( get_bench, "Input", "Uses the lorina library to read in a bench file" ){

		std::string filename = "";
		std::cout << "Enter bench filename: ";
		std::cin >> filename;

		if(checkExt(filename, "bench")){
			mockturtle::klut_network bench;
			lorina::read_bench(filename, mockturtle::bench_reader( bench ));
			std::cout << "Bench file = " << filename << " stored" << std::endl;
			store<mockturtle::klut_network>().extend() = bench;
		}
		else{
			std::cout << filename << " is not a valid bench file\n";
		}

	}//end get_bench

	/*Adds the command "write_to_file" where you can specify what store you are writing a file out from*/
	class write_to_bench_command : public alice::command{

	public:
	  explicit write_to_bench_command( const environment::ptr& env )
	      : command( env, "Writes a stored file to an external bench file" ){

	    opts.add_option( "--filename,filename", filename, "bench file to write to" )->required();
	    add_flag("--mig,-m", "Write the stored AIG file to a bench file");
	    add_flag("--bench,-b", "Write the stored bench file to a bench file");
	  }

	protected:
	  void execute(){

	    if(is_set("mig")){
	    	mockturtle::mig_network mig = store<mockturtle::mig_network>().current();
		    mockturtle::write_bench(mig, filename);
		}
		else if(is_set("bench")){
			mockturtle::klut_network bench = store<mockturtle::klut_network>().current();
			mockturtle::write_bench(bench, filename);
		}
		else{
			std::cout << "Store to write out wasn't specified\n";
		}
	  }

	private:
	  std::string filename{};
	};

	ALICE_ADD_COMMAND(write_to_bench, "Output");

	//Max get AIG
    /*Reads an aig file and stores the AIG network in a store*/
    class get_aig_command : public alice::command{

    public:
      explicit get_aig_command( const environment::ptr& env )
          : command( env, "Uses the lorina library to read in an aig file" ){

        opts.add_option( "--filename,filename", filename, "AIG file to read in" )->required();
      }

    protected:
      void execute(){

        if(checkExt(filename, "aig")){
                mockturtle::aig_network aig;
                lorina::read_aiger(filename, mockturtle::aiger_reader( aig ));
                // std::cout << "AIG file = " << filename << " stored" << std::endl;
                store<mockturtle::aig_network>().extend() = aig;

                if(aig._storage->inputNames.size() == 0){

                    for(int i = 0; i < aig.num_pis(); i++){
                        std::string input_name = "pi";
                        input_name.append(std::to_string(i));
                        aig._storage->inputNames[i] = input_name;
                    }
                }
                if(aig._storage->outputNames.size() == 0){

                    for(int i = 0; i < aig.num_pos(); i++){
                        std::string output_name = "po";
                        output_name.append(std::to_string(i));
                        aig._storage->outputNames[i] = output_name;
                    }
                }

                std::cout << "Inputs:\n";

                for(int i = 0; i < aig._storage->inputs.size(); i++){
                    std::cout << aig._storage->inputs.at(i) << ": " << aig._storage->inputNames[i] << "\n";
                }

                std::cout << "Outputs:\n";

                for(int i = 0; i < aig._storage->outputs.size(); i++){
                    std::cout << aig._storage->outputs.at(i).index << ": " << aig._storage->outputNames[i] << "\n";
                }

                filename.erase(filename.end() - 4, filename.end());
                aig._storage->net_name = filename;


            }
            else{
                std::cout << filename << " is not a valid aig file\n";
            }

      }

    private:
      std::string filename{};
    };

    ALICE_ADD_COMMAND(get_aig, "Input");

    class get_all_partitions_command : public alice::command{


    public:
      explicit get_all_partitions_command( const environment::ptr& env )
          : command( env, "Exports every partition to a file format (default is AAG)" ){
            opts.add_option( "--directory,directory", dir, "Directory to write aag files to" )->required();
            add_flag("--cone,-c", "Writes out every cone of every partition");
            add_flag("--verilog,-v", "Writes every partition or cone to a verilog file");
      }

    protected:
      void execute(){

        if(!store<mockturtle::aig_network>().empty()){
            auto aig = store<mockturtle::aig_network>().current();
            std::cout << "\n";
            if(aig._storage->num_partitions != 0){
                for(int i = 0; i < aig._storage->num_partitions; i++){
                    std::vector<mockturtle::aig_network> parts;
                    std::vector<std::string> filenames;
                    int partition = i;
                    std::cout << "Partition " << i << ":\n";
                    std::cout << "Number of Logic Cones = " << aig._storage->partitionOutputs[i].size() << "\n";
                    mkdir(dir.c_str(), 0777);
                    if(is_set("cone")){

                        for(int j = 0; j < aig._storage->partitionOutputs[i].size(); j++){

                            int output = aig._storage->partitionOutputs[i].at(j);
                            BFS_traversal(aig, output, partition);
                            int num_inputs = aig._storage->logic_cone_inputs[output].size();
                            aig.foreach_node( [&]( auto node ) {
                                int index = aig.node_to_index(node);
                                aig._storage->nodes[index].data[1].h1 = 0;
                            });
                            int levels = computeLevel(aig, output, partition);
                            std::cout << "Cone[" << j << "]: num inputs = " << num_inputs << " logic depth = " << levels << "\n";

                            mockturtle::aig_network part = create_aig_from_part(aig, partition, output);
                            std::string filename;
                            if(is_set("verilog")){
                                filename = dir + "/" + aig._storage->net_name + "_" + std::to_string(partition) + "_" + std::to_string(output) + ".v";
                            }
                            else{
                                filename = dir + "/" + aig._storage->net_name + "_" + std::to_string(partition) + "_" + std::to_string(output) + ".aag";
                            }
                            filenames.push_back(filename);
                            parts.push_back(part);

                        }

                    }
                    else{
                        mockturtle::aig_network part = create_aig_from_part(aig, partition);
                        std::string filename;
                        if(is_set("verilog")){
                            filename = dir + "/" + aig._storage->net_name + "_" + std::to_string(partition) + ".v";
                        }
                        else{
                            filename = dir + "/" + aig._storage->net_name + "_" + std::to_string(partition) + ".aag";
                        }
                        filenames.push_back(filename);
                        parts.push_back(part);
                    }
                    assert(parts.size() == filenames.size());
                    for(int j = 0; j < parts.size(); j++){
                        if(is_set("verilog")){
                            mockturtle::write_verilog(parts.at(j), filenames.at(j));
                        }
                        else{
                            write_aig(parts.at(j), filenames.at(j));
                        }
                    }

                    std::cout << "\n";
                }

            }
            else{
                std::cout << "Partitions have not been mapped\n";
            }
        }
        else{
            std::cout << "There is no AIG network stored\n";
        }

    }

    private:
        std::string dir{};
    };

    ALICE_ADD_COMMAND(get_all_partitions, "Output");

    class get_logic_cone_command : public alice::command{

    public:
        explicit get_logic_cone_command( const environment::ptr& env )
                : command( env, "Uses the lorina library to read in an aig file" ){

            opts.add_option( "--output,output", output, "Root of the cone" )->required();
            opts.add_option( "--partition,partition", partition, "Partition cone belongs to" )->required();
            opts.add_option( "--filename,filename", filename, "File to write aag file to" )->required();
        }

    protected:
        void execute(){

            if(!store<mockturtle::aig_network>().empty()){
                auto aig = store<mockturtle::aig_network>().current();
                if(aig._storage->num_partitions != 0){
                    std::cout << "partition = " << partition << " output " << output << "\n";
                    mockturtle::aig_network cone = create_aig_from_part(aig, partition, output);
                    std::cout << "aig created\n";
                    write_aig(cone, filename);
                }
                else{
                    std::cout << "Partitions have not been mapped\n";
                }
            }
            else{
                std::cout << "There is no AIG network stored\n";
            }
        }

    private:
        std::string filename;
        int partition;
        int output;
    };

    ALICE_ADD_COMMAND(get_logic_cone, "Output");


	/*Converts an AIG network to a k-LUT network and prints it out*/
	ALICE_COMMAND( lutMap, "Optimization", "Performs LUT mapping on an AIG network and prints out the bench file representation"){

		mockturtle::aig_network aig = store<mockturtle::aig_network>().current();
		mockturtle::mapping_view<mockturtle::aig_network, true> mapped_aig{aig};
		mockturtle::lut_mapping<mockturtle::mapping_view<mockturtle::aig_network, true>, true>(mapped_aig);
		const auto klut = mockturtle::collapse_mapped_network<mockturtle::klut_network>( mapped_aig );
		mockturtle::write_bench( aig, "lutMapActual.bench" );
	}

	ALICE_COMMAND( cut_e, "Optimization", "Performs Cut Enumeration on a stored AIG network"){
		auto aig = store<mockturtle::aig_network>().current();
		const mockturtle::network_cuts cuts = cut_enumeration( aig );
		//store<mockturtle::network_cuts>().extend() = cuts;
		aig.foreach_node( [&]( auto node ) {
  			std::cout << cuts.cuts( aig.node_to_index( node ) ) << "\n";
		} );
	}

ALICE_COMMAND( opt_aig, "Optimization", "Test performing AIG db rewriting."){
	  auto& aig = store<mockturtle::aig_network>().current();

    mockturtle::cut_rewriting_params ps;
    mockturtle::cut_rewriting_stats st;

    ps.cut_enumeration_ps.cut_size = 4;

    mockturtle::xag_npn_resynthesis<mockturtle::aig_network> resyn;
    mockturtle::cut_rewriting( aig, resyn, ps, &st);
    aig = mockturtle::cleanup_dangling( aig );
	}

class test_part_view_command : public alice::command{

    public:
        explicit test_part_view_command( const environment::ptr& env )
                : command( env, "Test the new partition view" ){

            // opts.add_option( "--filename,filename", filename, "partitioning file to map AIG nodes to" )->required();
                opts.add_option( "--num_parts,p", num_parts, "Number of partitions to create" )->required();
        }

    protected:
        void execute(){
            //running on AIG now. Fix it (the same way you use to print_stats with the ntk as parameter I guess)
            if(!store<mockturtle::aig_network>().empty()){
                //reads in the current network in memory
                auto& aig = store<mockturtle::aig_network>().current();

                aig.clear_visited();
                oracle::partition_manager<mockturtle::aig_network> partitions(aig, num_parts);

                for(int i = 0; i < partitions.get_part_num(); i++){
                    oracle::partition_view<mockturtle::fanout_view<mockturtle::aig_network>> part = partitions.create_part(aig, i);
                    std::set<mockturtle::aig_network::node> nodes;

                    nodes = partitions.get_part_context(i);
                    std::cout << "Nodes size " << nodes.size() << "\n";
                    std::cout << "Nodes in partition " << "\n";
                    for (auto n : nodes ){
                        std::cout << n << " " << std::endl;
                    }

                    mockturtle::cut_rewriting_params ps;
                    mockturtle::cut_rewriting_stats st;

                    ps.cut_enumeration_ps.cut_size = 4;

                    mockturtle::xag_npn_resynthesis<mockturtle::aig_network> resyn;
                    mockturtle::part_rewriting(aig, nodes, resyn, ps, &st);

                    //mockturtle::cut_rewriting( aig, nodes, resyn, ps, &st);
//
                }
                aig = mockturtle::cleanup_dangling( aig );
                std::cout << "Nodes in network after opt " << " : " << aig.size() << "\n";

                // mockturtle::depth_view aig_depth2{aig};
                // std::cout << "new aig size = " << aig.num_gates() << " and depth = " << aig_depth2.depth() << "\n";
            }
            else{
                std::cout << "There is no stored AIG network\n";
            }
        }

    private:
        std::string filename{};
        int num_parts = 0;
    };

    ALICE_ADD_COMMAND(test_part_view, "Test");


    ALICE_COMMAND( partition_sizes, "Partitioning", "Shows the size of each partition"){

        if(!store<mockturtle::aig_network>().empty()){
            auto aig = store<mockturtle::aig_network>().current();
            for(int i = 0; i < aig._storage->num_partitions; i++){
                std::cout << "Partition=" << i << ": ";
                std::cout << aig._storage->partitionSize[i] << "\n";
            }
        }
        else{
            std::cout << "There is no AIG network stored\n";
        }
    }

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

    ALICE_COMMAND( partition_detail, "Partitioning", "Shows what nodes are in each partition"){

        if(!store<mockturtle::aig_network>().empty()){
            auto aig = store<mockturtle::aig_network>().current();
            for(int i = 0; i < aig._storage->num_partitions; i++){
                std::cout << "Partition " << i << ":\n";
                aig.foreach_node( [&]( auto node ) {
                    int nodeIdx = aig.node_to_index(node);
                    if(aig._storage->partitionMap[nodeIdx] == i){
                        std::cout << nodeIdx << "\n";
                    }
                } );
            }
        }
        else{
            std::cout << "There is no AIG network stored\n";
        }
    }

    class partitioning_command : public alice::command{

    public:
      explicit partitioning_command( const environment::ptr& env )
        : command( env, "Generates AIG partitions." )
        {
          opts.add_option( "--num,num", num_partitions, "Number of desired partitions" )->required();
        }

    protected:
      void execute(){

        auto ntk = store<mockturtle::aig_network>().current();
        oracle::partition_manager<mockturtle::aig_network> partitions(ntk, num_partitions);
      }

    private:
      int num_partitions{};
    };

    ALICE_ADD_COMMAND(partitioning, "Partitioning");


    class output_truth_table_command : public alice::command{

    public:
        explicit output_truth_table_command( const environment::ptr& env )
                : command( env, "Output the truth table for each partition in format" ){

            add_flag("--offset,-i", "Output the offset of all partition outputs. The onset is the default");
        }

    protected:
        void execute(){

            int output_wanted = 1;
            if(is_set("offset")){
                output_wanted = 0;
            }

            if(!store<mockturtle::aig_network>().empty()){

                auto aig = store<mockturtle::aig_network>().current();
                //Check to see if the AIG network's nodes have been mapped to their corresponding partitions
                if(aig._storage->partitionMap.size() != 0){



                    // for(int i = 0; i < aig._storage->num_partitions; i++){

                    // 	std::cout << "Partition " << i << " Inputs: ";
                    // 	for(int j = 0; j < aig._storage->partitionInputs[i].size(); j++){
                    // 		std::cout << aig._storage->partitionInputs[i].at(j) << " ";
                    // 	}
                    // 	std::cout << "\n";
                    // 	std::cout << "Partition " << i << " Outputs: ";
                    // 	for(int j = 0; j < aig._storage->partitionOutputs[i].size(); j++){
                    // 		std::cout << aig._storage->partitionOutputs[i].at(j) << " ";
                    // 	}
                    // 	std::cout << "\n";
                    // }

                    // auto new_aig = create_aig_from_part(aig, 4006);
                    // std::string filename = aig._storage->net_name + "_out_" + std::to_string(4006) + ".bench";
                    // mockturtle::write_bench(new_aig, filename);
                    // new_aig = create_aig_from_part(aig, 4137);
                    // filename = aig._storage->net_name + "_out_" + std::to_string(4137) + ".bench";
                    // mockturtle::write_bench(new_aig, filename);
                    // new_aig = create_aig_from_part(aig, 4255);
                    // filename = aig._storage->net_name + "_out_" + std::to_string(4255) + ".bench";
                    // mockturtle::write_bench(new_aig, filename);
                    // new_aig = create_aig_from_part(aig, 71, 4133);
                    // filename = aig._storage->net_name + "_part_" + std::to_string(71) + "_out_" + std::to_string(4133) + ".bench";
                    // mockturtle::write_bench(new_aig, filename);
                    // new_aig = create_aig_from_part(aig, 70, 3997);
                    // filename = aig._storage->net_name + "_part_" + std::to_string(70) + "_out_" + std::to_string(3997) + ".bench";
                    // mockturtle::write_bench(new_aig, filename);
                    // new_aig = create_aig_from_part(aig, 80, 4129);
                    // filename = aig._storage->net_name + "_part_" + std::to_string(80) + "_out_" + std::to_string(4129) + ".bench";
                    // mockturtle::write_bench(new_aig, filename);
                    // new_aig = create_aig_from_part(aig, 83, 4137);
                    // filename = aig._storage->net_name + "_part_" + std::to_string(83) + "_out_" + std::to_string(4137) + ".bench";
                    // mockturtle::write_bench(new_aig, filename);
                    // for(int i = 0; i < aig._storage->num_partitions; i++){
                    // 	for(int j = 0; j < aig._storage->partitionOutputs[i].size(); j++){
                    // 		auto new_aig = create_aig_from_part(aig, i, aig._storage->partitionOutputs[i].at(j));
                    // 		std::string filename = aig._storage->net_name + "_part_" + std::to_string(i) + "_out_" + std::to_string(aig._storage->partitionOutputs[i].at(j)) + ".bench";
                    // 		mockturtle::write_bench(new_aig, filename);
                    // 	}
                    // }



                    //Create the truth table for each output of the partition
                    for(int i = 0; i < aig._storage->num_partitions; i++){

                        std::cout << "\npartition = " << i << "\n";
                        std::cout << "-----------------------------------\n";
                        std::cout << "partition size = " << aig._storage->partitionSize[i] -  aig._storage->partitionInputs[i].size() - aig._storage->partitionOutputs[i].size()<< "\n";
                        std::cout << "partitionInputs num = " << aig._storage->partitionInputs[i].size() << "\n";
                        std::cout << "partitionOutputs num = " << aig._storage->partitionOutputs[i].size() << "\n\n";


                        for(int j = 0; j < aig._storage->partitionOutputs[i].size(); j++){
                            aig._storage->test_timeout = false;
                            // clock_t start, end;
                            //  			double cpu_time_used;

                            //  			start = clock();
                            int output = aig._storage->partitionOutputs[i].at(j);
                            aig._storage->tt[output].clear();
                            aig._storage->index[output].clear();

                            //Create a truth table that has as many indeces as nodes in the
                            //original AIG network and set the defaults for each to 1
                            std::vector<int> defaultSlate;
                            int size = aig._storage->partitionSize[i] + aig._storage->partitionInputs[i].size() + aig._storage->partitionOutputs[i].size();
                            for(int k = 0; k < size; k++){
                                defaultSlate.push_back(1);
                            }
                            aig._storage->tt[output].push_back(defaultSlate);

                            aig._storage->index[output].push_back(output);
                            aig._storage->wantedOut[output].push_back(output_wanted);
                            //Make the logic value for the root node (output) the output wanted
                            aig._storage->tt[output].at(0).at(0) = output_wanted;

                            std::cout << "\nOutput " << output << "\n";
                            std::cout << "Depth of Logic Cone = " << computeLevelPart(aig,output,i) << "\n";
                            std::cout << "Number of inputs in logic cone = " << aig.num_pis() << "\n\n";
                            // std::cout << "current output = " << output << "\n";
                            genTruthBFS(aig, output, aig._storage->wantedOut[output], i);
                            if(aig._storage->test_runtime[aig._storage->partitionOutputs[i].at(j)] != 0.0){
                                std::cout << "output truth table runtime = " << aig._storage->test_runtime[aig._storage->partitionOutputs[i].at(j)] << " seconds\n\n";
                            }
                            else
                                std::cout << "Truth Table Generation Algorithm finished really fast\n\n";
                            // std::cout << "index = {";
                            // for(int idx = 0; idx < aig._storage->index[output].size(); idx++){
                            // 	std::cout << aig._storage->index[output].at(idx) << " ";
                            // }
                            // std::cout << "}\n";
                            // for(int idx = 0; idx < aig._storage->tt[output].size(); idx++){
                            // 	std::cout << "tt.at(" << idx << ") ";
                            // 	for(int jdx = 0; jdx < aig._storage->tt[output].at(idx).size(); jdx++){

                            // 	 std::cout << aig._storage->tt[output].at(idx).at(jdx);
                            // 	}
                            // 	std::cout << "\n";
                            // }
                            if(!aig._storage->test_timeout)
                                finalize_truth(aig, output, i);

                            // std::cout << "index = {";
                            // for(int idx = 0; idx < aig._storage->index[output].size(); idx++){
                            // 	std::cout << aig._storage->index[output].at(idx) << " ";
                            // }
                            // std::cout << "}\n";
                            // std::cout << "Final truth table\n";
                            // for(int idx = 0; idx < aig._storage->tt[output].size(); idx++){
                            // 	std::cout << "tt.at(" << idx << ") ";
                            // 	for(int jdx = 0; jdx < aig._storage->tt[output].at(idx).size(); jdx++){

                            // 	 std::cout << aig._storage->tt[output].at(idx).at(jdx);
                            // 	}
                            // 	std::cout << "\n";
                            // }
                            // blif_write(aig, i, output_wanted, output);
                            // end = clock();
                            // double runtime = ((double) (end - start)) / CLOCKS_PER_SEC;
                            // //std::cout << "runtime = " << runtime << "\n";
                            // aig._storage->test_runtime[output] = runtime;
                        }


                    }

                    //print_part_stats(aig);

                    blif_test(aig, output_wanted);
                    /*

                    for(int i = 0; i < aig._storage->num_partitions; i++){

                        std::cout << "Partition " << 0 << " truth table logic\n";
                        for(int j = 0; j < aig._storage->partitionOutputs[0].size(); j++){
                            for(int k = 0; k < aig._storage->truth[0][j].size(); k++){
                                std::cout << aig._storage->truth[0][j].at(k);
                            }
                            std::cout << " 1\n";
                        }
                    }
                    */

                }
                else{
                    std::cout << "The partitions have not been mapped to the stored aig network\n";
                }
            }
            else{
                std::cout << "There is no stored AIG network\n";
            }

        }

    };

    ALICE_ADD_COMMAND(output_truth_table, "Output");

	ALICE_COMMAND(show_mig, "Output", "Writes the the mig dot file"){
		auto mig = store<mockturtle::mig_network>().current();

		for (int j =1; j < mig._storage->nodes.size(); j++) {
			for (int i = 0; i < mig._storage->nodes.data()->children.size(); i++) {
				std::cout << "node index " << j << " node fan in " << mig._storage->nodes[j].children[i].data << std::endl;
			}
		}
		for (unsigned k = mig.num_pis()+1; k<= mig._storage->inputs.size(); k++ ){
			auto node = mig.index_to_node(k);
			std::cout << " reg " << k << " fan out size " << mig.fanout_size(node) << std::endl;
            //mig._storage->nodes[node].data
        }
        for (unsigned l=0; l< mig._storage->outputs.size(); l++){
            std::cout << " outputs " << std::endl;
            //std::cout << " node fan in index " << mig._storage->outputs[l].index << std::endl;
            std::cout << " node fan in data " << mig._storage->outputs[l].data << std::endl;
        }
	}

    ALICE_COMMAND(show_aig, "Output", "Writes the the aig dot file"){

//		std::string filename = " ";
//		std::cout << "Enter filename: ";
//		std::cin >> filename;
//
//		std::ofstream dump;
//		dump.open (filename);

        auto aig = store<mockturtle::aig_network>().current();

        for (int j =1; j < aig._storage->nodes.size(); j++) {
            for (int i = 0; i < aig._storage->nodes.data()->children.size(); i++) {
				std::cout << "node index " << j << " node fan in " << aig._storage->nodes[j].children[i].data << std::endl;
            }
        }

        for (unsigned k = aig.num_pis()+1; k<= (aig._storage->inputs.size() - aig.num_latches()); k++ ){
        	auto node = aig.index_to_node(k);
			std::cout << " reg " << k << " fan out size " << aig.fanout_size(node) << std::endl;
        }

		for( int outIndex=0; outIndex<aig.num_pos()- aig.num_latches(); outIndex++){
			std::cout << "Output " << outIndex << " data " << aig._storage->outputs[outIndex].data << std::endl;
			//std::cout << " node fan in index " << aig._storage->outputs[l].index << std::endl;
		}

		aig.foreach_ri([&](auto fi) {
			std::cout << "Register " << aig.ri_index(fi) << " data " << fi.data << std::endl;
		});
		//dump.close();
    }

    ALICE_COMMAND(print_eachout, "Output", "Testing foreach") {
		auto aig = store<mockturtle::aig_network>().current();
		static int counter = 0;
		aig.foreach_po([&](auto fi) {
			std::cout << "Out index " << aig.po_index(fi) << " Counter " << counter << std::endl;
		});
		counter++;
	}

	ALICE_COMMAND(depth, "Output", "Testing depth") {
		auto aig = store<mockturtle::aig_network>().current();

		mockturtle::depth_view aig_depth{aig};

		std::cout << "Aig level " << aig_depth.depth()  << std::endl;

	}

	ALICE_COMMAND(get_cones, "Output", "Get logical cones for each PO") {
//        std::string filename = " ";
//        std::cout << "Enter filename: ";
//        std::cin >> filename;
//
//        std::ofstream conesfile;
//        conesfile.open (filename);

		auto aig = store<mockturtle::aig_network>().current();

		//map with number of nodes in each logical cone
		std::unordered_map<int, int> po_nodes;
		std::unordered_map<int, int> ri_nodes;

		//number of inputs for each cone
		std::unordered_map<int, int> po_ins;
		std::unordered_map<int, int> ri_ins;

		//first processing logical cones for POs
		//aig.foreach_po([&](auto fi) {
		for(int outIndex=0; outIndex<aig.num_pos()- aig.num_latches(); outIndex++) {

			aig.foreach_node([&](auto node) {
				//set all nodes as not visited
				aig._storage->nodes[node].data[1].h1 = 0;
			});

			//PO index to keep track of each cone on the map structure
			//auto outIndex = aig.po_index(fi);

			//start counter for a given output index
			po_nodes.insert(std::make_pair(outIndex, 0));

			//starting the counter of inputs
			po_ins.insert(std::make_pair(outIndex, 0));

			//calculate the index of the node driving the output
			auto inIdx = aig._storage->outputs[outIndex].data;
			if (aig._storage->outputs[outIndex].data & 1)
				inIdx = aig._storage->outputs[outIndex].data - 1;

			inIdx = inIdx >> 1;

			//call DFS
			compute_cone(aig, inIdx, po_nodes, outIndex, po_ins);

			aig.foreach_node([&](auto node) {
				//set all nodes as not visited
				aig._storage->nodes[node].data[1].h1 = 0;
			});

			int level = computeLevel(aig, inIdx);
			int nodes = 0;
			int inputs = 0;

			// for each output prints index, nodes, depth and number of inputs, respectively
			std::unordered_map<int, int>::iterator it;
			it = po_nodes.find(outIndex);

			if (it != po_nodes.end())
				nodes = it->second;

			std::unordered_map<int, int>::iterator init;
			init = po_ins.find(outIndex);

			if (it != po_nodes.end())
				inputs = init->second;

			std::cout << "Output " << outIndex << " " << nodes << " " << level << " " << inputs << std::endl;
			//});
		}

		//processing logical cones for registers - pseudo POs
		aig.foreach_ri([&](auto fi) {
			aig.foreach_node([&](auto node) {
				//set all nodes as not visited
				aig._storage->nodes[node].data[1].h1 = 0;
			});

            //std::cout << "Entrando no foreach RI " << std::endl;

            //PO index to keep track of each cone on the map structure
			auto outIndex = aig.ri_index(fi);

			//start counter for a given ri index
			ri_nodes.insert(std::make_pair(outIndex, 0));
			ri_ins.insert(std::make_pair(outIndex, 0));

			//calculate the index of the node driving the output
			auto inIndex = fi.data;
			if(fi.data & 1)
				inIndex = fi.data - 1;

			//calculate latch input node index
			inIndex = inIndex >> 1;

			//call DFS
            compute_cone(aig, inIndex, ri_nodes, outIndex, ri_ins);

            aig.foreach_node([&](auto node) {
                //set all nodes as not visited
                aig._storage->nodes[node].data[1].h1 = 0;
            });

            int level = computeLevel(aig, inIndex);
            int nodes = 0;
			int inputs = 0;

			// for each output prints index, nodes, depth and number of inputs, respectively
			std::unordered_map<int, int>::iterator it;
			it = ri_nodes.find(outIndex);

			if(it != po_nodes.end())
				nodes = it->second;

			std::unordered_map<int, int>::iterator init;
			init = ri_ins.find(outIndex);

			if(it != po_nodes.end())
				inputs = init->second;

			std::cout << "Register " << outIndex << " " << nodes << " " << level << " " << inputs << std::endl;
		});
		//conesfile.close ();
    }

    ALICE_COMMAND(disjoint_clouds, "Output", "Writes the status for each disjoint combinational cloud on the current AIG"){
		std::string filename = " ";
		std::cout << "Enter filename: ";
		std::cin >> filename;

		std::ofstream dset;
		dset.open (filename);

		auto aig = store<mockturtle::aig_network>().current();

		//maps to keep the depth and numb of nodes for each comb cloud
		std::unordered_map<int, int> outputs;
		std::unordered_map<int, int> depth;
		std::unordered_map<int, int> number_nodes;


		//copy sets in a local vector
		std::vector<int> clouds;

	    //number of aig inputs + 1 to start at the nodes index
	    uint64_t inSize = aig.num_pis()+1;

	    //union find constructor - inSize sets the vector index where and nodes start
        UnionFind uf(aig.size()-1, inSize);

		//set all nodes as not visited for the DFS
		aig.foreach_node([&] (auto node) {
            aig._storage->nodes[node].data[1].h1 = 0;
        });

	    //first looking to latches
	    aig.foreach_ri( [&] (auto ri)
        {
	    	auto indata = ri.data;

           if(indata & 1)
               indata = indata - 1;

           //calculate latch input node index
           auto inIndex = indata >> 1;

           if(inIndex>aig.num_pis()) {
			   dfs(aig, inIndex, uf);
		   }
		});

		aig.foreach_node([&] (auto node) {
			//set all nodes as not visited
			aig._storage->nodes[node].data[1].h1 = 0;
		});

	    //then considering POs
	    for(int i = 0; i<aig.num_pos()-aig.num_latches(); i++){
			auto inIdx = aig._storage->outputs[i].data;
			if(aig._storage->outputs[i].data & 1)
				inIdx = aig._storage->outputs[i].data - 1;
			//calculate the index of the node driving the output
			inIdx = inIdx >> 1;
			if(inIdx>aig.num_pis()) {
				dfs(aig, inIdx, uf);
			}
	    }

	    uf.get_sets(clouds);

	    for (int j = 0; j < clouds.size() ; ++j) {
			auto it = number_nodes.find (clouds[j]);
			//element not in the table yet
			if(it==number_nodes.end()){
				int count = std::count(std::begin(clouds), std::end(clouds), clouds[j]);
				number_nodes.insert(std::make_pair(clouds[j],count));
			}
		}

		for (std::pair<int, int> element : number_nodes)
		{
			dset << element.second << std::endl;
		}

		std::cout << "Number of disjoint graphs: " << uf.getNumbOfSets() << std::endl;

		dset.close();
	}

    class write_verilog_command : public alice::command{

    public:
      explicit write_verilog_command( const environment::ptr& env )
          : command( env, "Writes the Boolean network into structural verilog" ){

        opts.add_option( "--filename,filename", filename, "Verilog file to write out to" )->required();
      }

    protected:
        void execute(){
            if(checkExt(filename, "v")){
                if(!store<mockturtle::mig_network>().empty()){
                    auto& mig = store<mockturtle::mig_network>().current();
                    mockturtle::write_verilog(mig, filename);
                }
                else{
                    std::cout << "There is not an MIG network stored.\n";
                }
            }
            else{
                std::cout << filename << " is not a valid verilog file\n";
            }
        }

    private:
      std::string filename{};
    };

    ALICE_ADD_COMMAND(write_verilog, "Output");

	ALICE_COMMAND(tmap, "Transformation", "Performs LUT techmapping") {
		auto mig = store<mockturtle::mig_network>().current();;
		mockturtle::mapping_view<mockturtle::mig_network, true> mapped_mig{mig};

		mockturtle::lut_mapping_params ps;
		ps.cut_enumeration_ps.cut_size = 6;
		mockturtle::lut_mapping(mapped_mig, ps);
		//mockturtle::lut_mapping<mapped_view<mig_network, true>, true>( mapped_mig );
	}

    ALICE_COMMAND(interleaving, "Modification", "NPN + depth MIG rewriting") {
        auto& mig = store<mockturtle::mig_network>().current();

        mockturtle::mig_npn_resynthesis resyn;
        mockturtle::cut_rewriting_params ps;

        ps.cut_enumeration_ps.cut_size = 4;

        mockturtle::cut_rewriting(mig, resyn, ps);
        mig = mockturtle::cleanup_dangling( mig );

    }


	ALICE_COMMAND(migscript, "Modification", "Exact NPN MIG rewriting") {
		auto& opt = store<mockturtle::mig_network>().current();
        mockturtle::depth_view mig_depth{opt};

		//DEPTH REWRITING
        std::cout << "MIG logic depth " << mig_depth.depth() << " Majority nodes " << opt.num_gates() << std::endl;

        mockturtle::mig_script migopt;
        opt = migopt.run(opt);

	}

	ALICE_COMMAND(depthr, "Modification", "Logic depth oriented MIG rewriting"){
	    auto& mig = store<mockturtle::mig_network>().current();
		std::cout << "Mig gates " << mig.num_gates() << std::endl;

		//to compute at level
		mockturtle::depth_view mig_depth{mig};

		std::cout << "Mig level " << mig_depth.depth() << " mig gates " << mig.num_gates() << std::endl;

		mockturtle::mig_algebraic_depth_rewriting_params pm;
		pm.aggressive;
		mockturtle::mig_algebraic_depth_rewriting(mig_depth, pm);

		mig = mockturtle::cleanup_dangling( mig );
		std::cout << "Mig level after algebraic rewriting " << mig_depth.depth() << " Mig gates " << mig.num_gates() << std::endl;

//		mockturtle::mig_npn_resynthesis resyn;
//		mockturtle::cut_rewriting_params ps;
//		ps.cut_enumeration_ps.cut_size = 4;
//		mockturtle::cut_rewriting(mig, resyn, ps);
//		//mig = mockturtle::cleanup_dangling(mig);
//
//        std::cout << " mig gates after rewriting " << mig.num_gates() << std::endl;

//		mockturtle::refactoring(mig, resyn);
//		//mockturtle::resubstitution (mig);
//		mig = cleanup_dangling( mig );
//
//		std::cout << " mig gates after refact 2" << mig.num_gates() << std::endl;
//
//		//mockturtle::resubstitution (mig);
//		//mig = cleanup_dangling( mig );
//
//		std::cout << " mig gates after 2 resub " << mig.num_gates() << std::endl;
	}

	ALICE_COMMAND(test_ci, "Output", "Testing mig is ci"){
		auto mig = store<mockturtle::mig_network>().current();

		mig.foreach_ci([&] (auto node) {
			if(mig.is_ci(node)){
				std::cout << "Found CI " << std::endl;
			}
		});
	}

    ALICE_CONVERT( mockturtle::mig_network, element, mockturtle::aig_network )
    {
	    auto aig = mockturtle::aig_network();
		std::unordered_set<uint32_t> toinvert;
		std::unordered_set<uint32_t> duplicated;
		std::map<uint32_t , mockturtle::aig_network::signal> update_output ;
		std::vector<int> missing;

        //create aig inputs
		for (int i = 0; i < element.num_pis() - element.num_latches(); ++i) {
			aig.create_pi();
		}

        //create registers output (PIs in the AIG)
        for (int i=0; i<element.num_latches(); i++){
        	aig.create_ro();
        }

        //convert majs into ands
		element.foreach_gate([&] (auto gate) {
			auto index = element.node_to_index(gate);

			//ignore PIs and Latch inputs
			if(index > element.num_pis()) {

				if(element._storage->nodes[index].children[0].data == 1){

					auto child2 = element._storage->nodes[index].children[1];
					auto child3 = element._storage->nodes[index].children[2];

					auto search = toinvert.find(2 * child2.index);
					if (search == toinvert.end()) {
						child2 = aig.create_not(child2);
					}

					auto search1 = toinvert.find(2 * child3.index);
					if (search1 == toinvert.end()) {
						child3 = aig.create_not(child3);
					}

					//add complemented nodes. If a complemented node (first child equals to 1) is fan-in of a node, it should be complemented while generating the aig
					toinvert.insert(2 * index);

					aig.create_maj(aig.child_to_signal(0), child2, child3);
				}

				else if (element._storage->nodes[index].children[0].data == 0){
					auto child2 = element._storage->nodes[index].children[1];
					auto child3 = element._storage->nodes[index].children[2];

					auto search = toinvert.find(2 * child2.index);
					if (search != toinvert.end()) {
						child2 = aig.create_not(child2);
						duplicated.insert(child2.index * 2);
						toinvert.erase(2*index);
					}

					auto search1 = toinvert.find(2 * child3.index);
					if (search1 != toinvert.end()) {
						child3 = aig.create_not(child3);
						duplicated.insert(child3.index * 2);
					}

					aig.create_maj(aig.child_to_signal(0), child2, child3);
				}

				else{
					auto child1 = element._storage->nodes[index].children[0];
					auto child2 = element._storage->nodes[index].children[1];
					auto child3 = element._storage->nodes[index].children[2];

					auto search = toinvert.find(2 * child1.index);
					auto search0 = duplicated.find(2 * child1.index);
					if (search != toinvert.end() && search0 == duplicated.end()) {
						child1 = aig.create_not(child1);
					}

					auto search1 = duplicated.find(2 * child2.index);
					auto search2 = toinvert.find(2 * child2.index);
					if (search2 != toinvert.end() && search1 == duplicated.end()) {
						child2 = aig.create_not(child2);
					}

					auto search3 = toinvert.find(2 * child3.index);
					auto search4 = duplicated.find(2*child3.index);
					if (search3 != toinvert.end() && search4 == duplicated.end()) {
						child3 = aig.create_not(child3);
					}

					auto maj = aig.create_maj(child1, child2, child3);

					std::cout << "Inserting in the table " << 2*index << " " << maj.index << std::endl;
					update_output.insert(std::make_pair(2*index, maj));

				}
			}
		});

		aig.foreach_gate([&] (auto gate) {
			auto index = aig.node_to_index(gate);

			//ignore PIs and Latch inputs
			if(index > aig.num_pis()) {

				auto child1 = aig._storage->nodes[index].children[0];
				auto child2 = aig._storage->nodes[index].children[1];

				auto it = update_output.find(2*child1.index);

				if(it!=update_output.end()){
					if(index < (it->first)/2) {
						//it->second.complement = it->second.complement ^ 1;
						std::cout << "Child1! My index " << index <<std::endl;
						std::cout << "Replacing " << 2*child1.index << " by " << it->second.index << " polarity " << it->second.complement << std::endl;
						aig._storage->nodes[index].children[0] = it->second;
						//aig.update();
					}
				}

				auto it1 = update_output.find(2*child2.index);

				if(it1!=update_output.end()){
					if(index < (it1->first)/2) {
						//it1->second.complement = it1->second.complement ^ 1;
						std::cout << "Child2! My index " << index <<std::endl;
						std::cout << "Replacing " << 2*child2.index << " by " << it1->second.index << " polarity " << it1->second.complement << std::endl;
						aig._storage->nodes[index].children[1] = it1->second;
						//aig.update();
					}
				}
			}
		});

        //create aig outputs
		for(int i = 0; i < element.num_pos() - element.num_latches(); i++){
			auto in = element._storage->outputs[i].data;
			auto signal = element.child_to_signal(in);
			auto signalOut = mockturtle::aig_network::signal(signal);

			auto it = update_output.find(in);

			if(it!=update_output.end()){
				//std::cout << "Need to replace node " << in << " by " << it->second.index << std::endl;
				signalOut = it->second;
			}

			else {
				auto search = toinvert.find(1 ^ in);
				if (search != toinvert.end()) {
					signalOut = aig.create_not(signalOut);
				}

				auto search1 = toinvert.find(in);
				if (search1 != toinvert.end()) {
					signalOut = aig.create_not(signalOut);
				}
			}
			aig.create_po(signalOut);
		}

		//create registers input as AIG PO
		for (unsigned int i = 0; i < element.num_latches() ; ++i) {
			int8_t reset = 0;
			std::string name = "";
			auto regIdx = element.num_pos() - element.num_latches() + i;
			auto signal = element.ri_at(regIdx);

			bool comp = false;
			auto temp = signal.data;

			auto it = update_output.find(2*signal.index);

			if(it!=update_output.end()){
				//std::cout << "Need to replace node " << in << " by " << it->second.index << std::endl;
				auto ri = it->second;
				aig.create_ri(ri, reset, name);

			}

			else{
				if (signal.data & 1) {
					auto temp = signal.data - 1;
				}

				auto nodeIn = signal.data / 2;
				if (element._storage->nodes[nodeIn].children[0].data == 1) {
					comp = true;
				}

				auto ri = mockturtle::aig_network::signal(signal);

				if (comp) {
					ri = aig.create_not(ri);
				}

				aig.create_ri(ri, reset, name);
			}
		}
		return aig;
    }

   /* ALICE_COMMAND(rewrite_mig, "Optimization", "Runs MIG rewrite"){
        auto mig = store<mockturtle::mig_network>().current();
        mockturtle::mig_algebraic_depth_rewriting_params ps;
        ps.strategy = mockturtle::mig_algebraic_depth_rewriting_params::selective;
        mockturtle::mig_algebraic_depth_rewriting(mig, ps);
    }*/

	ALICE_COMMAND(test_aig, "Output", "Writes the the mig dot file"){
		auto aig = store<mockturtle::aig_network>().current();

		aig.foreach_node( [&] (auto node)
						  {
							  for (int i=0; i < aig._storage->nodes.data()->children.size() ; i++ ){
								  std::cout << "node index " << node << " node fan in " << aig._storage->nodes.data()->children[i].data  << std::endl;
							  }
						  });
	}

    ALICE_COMMAND(print_part_stats, "Stats", "Prints overall statistics of the paritions"){

        if(!store<mockturtle::aig_network>().empty()){
            auto aig = store<mockturtle::aig_network>().current();
            // int avg_size = 0;
            // int max_size = 0;
            // for(int i = 0; i < aig._storage->num_partitions; i++){
            // 	avg_size += aig._storage->partitionSize[i];
            // 	if(max_size < aig._storage->partitionSize[i])
            // 		max_size = aig._storage->partitionSize[i];
            // }

            // avg_size = avg_size / aig._storage->num_partitions;
            // std::cout << "average part size = \n";
            // std::cout << avg_size << "\n";
            // std::cout << "max part size = \n";
            // std::cout << max_size << "\n";


            int avg_in_size = 0;
            int max_in_size = 0;
            for(int i = 0; i < aig._storage->num_partitions; i++){
                avg_in_size += aig._storage->partitionInputs[i].size();
                if(max_in_size < aig._storage->partitionInputs[i].size())
                    max_in_size = aig._storage->partitionInputs[i].size();
            }
            avg_in_size = avg_in_size / aig._storage->num_partitions;

            std::cout << "average part input size = \n";
            std::cout << avg_in_size << "\n";
            std::cout << "max part input size = \n";
            std::cout << max_in_size << "\n";
        }
        else{
            std::cout << "No AIG network stored\n";
        }

    }

    ALICE_COMMAND( test_partition_size, "Test", "Checks to see if any of the logic cones are too big for a truth table to be generated i.e. inputs >= 25"){
        if(!store<mockturtle::aig_network>().empty()){
            auto aig = store<mockturtle::aig_network>().current();
            if(aig._storage->num_partitions != 0){
                for(int i = 0; i < aig._storage->num_partitions; i++){
                    for(int j = 0; j < aig._storage->partitionOutputs[i].size(); j++){
                        int curr_output = aig._storage->partitionOutputs[i].at(j);
                        BFS_traversal(aig, curr_output, i);
                        if(aig._storage->logic_cone_inputs[curr_output].size() >= 25){
                            std::cout << "Cone for " << curr_output << " is too big with size " << aig._storage->logic_cone_inputs[curr_output].size() << "\n";
                        }
                    }
                }
            }
            else{
                std::cout << "Partitions not mapped yet\n";
            }
        }
        else{
            std::cout << "No AIG network stored\n";
        }
    }

    ALICE_COMMAND( test_new_tt_algo, "Test", "Test alternative tt generation algorithm"){
        clock_t start, end;
        if(!store<mockturtle::aig_network>().empty()){
            auto aig = store<mockturtle::aig_network>().current();
            if(aig._storage->num_partitions != 0){
                for(int i = 0; i < aig._storage->num_partitions; i++){
                    // std::cout << "Partition = " << i << "\n";
                    // std::cout << "-----------------------\n";
                    // std::cout << "size = " << aig._storage->partitionInputs[i].size() << "\n";


                    for(int j = 0; j < aig._storage->partitionOutputs[i].size(); j++){
                        start = clock();
                        int curr_output = aig._storage->partitionOutputs[i].at(j);
                        BFS_traversal(aig, curr_output, i);
                        // std::cout << "BFS done at " << ((float)clock() - start)/CLOCKS_PER_SEC << "\n";
                        // std::cout << "curr_output = " << curr_output << "\n";
                        // std::cout << "size = " << aig._storage->logic_cone_inputs[curr_output].size() << "\n";
                        // std::cout << "logic cone inputs: \n";
                        if(aig.is_constant(curr_output)){
                            kitty::dynamic_truth_table tt (0);
                            if(aig._storage->onset[curr_output].size() > aig._storage->offset[curr_output].size()){
                                // std::cout << "set constant to 1\n";
                                std::string bin = "1";
                                kitty::create_from_binary_string(tt, bin);
                                // std::cout << "tt created\n";
                                aig._storage->output_tt[curr_output] = tt;
                                // kitty::print_hex(aig._storage->output_tt[curr_output], std::cout);
                                // std::cout << "\n";
                            }
                            else{
                                // std::cout << "set constant to 0\n";
                                std::string bin = "0";
                                kitty::create_from_binary_string(tt, bin);
                                aig._storage->output_tt[curr_output] = tt;
                                // kitty::print_hex(aig._storage->output_tt[curr_output], std::cout);
                                // std::cout << "\n";
                            }
                            // std::cout << "partition=" << i << " ";
                            // std::cout << "output=" << curr_output << " ";
                            aig.foreach_node( [&]( auto node ) {
                                int index = aig.node_to_index(node);
                                aig._storage->nodes[index].data[1].h1 = 0;
                            });
                            // std::cout << computeLevelPart(aig,curr_output,i) << " ";
                            // std::cout << aig._storage->logic_cone_inputs[curr_output].size() << " ";
                            // float runtime = ((float)clock() - start)/CLOCKS_PER_SEC;
                            // std::cout << std::fixed << std::setprecision(12) << runtime << "\n";
                        }
                        else if(aig._storage->logic_cone_inputs[curr_output].size() <= 25 && !aig.is_constant(curr_output)){
                            std::sort(aig._storage->logic_cone_inputs[curr_output].begin(), aig._storage->logic_cone_inputs[curr_output].end());
                            // std::cout << "sort done at " << ((float)clock() - start)/CLOCKS_PER_SEC << "\n";
                            for(int k = 0; k < aig._storage->logic_cone_inputs[curr_output].size(); k++){


                                int nodeIdx = aig._storage->logic_cone_inputs[curr_output].at(k);
                                // std::cout << "input = " << nodeIdx << "\n";
                                kitty::dynamic_truth_table tt( aig._storage->logic_cone_inputs[curr_output].size() );

                                kitty::create_nth_var(tt, k);
                                // kitty::print_hex(tt, std::cout);
                                // std::cout << "\n";

                                aig._storage->tt_map[nodeIdx] = tt;
                            }
                            // std::cout << "truth table declared at " << ((float)clock() - start)/CLOCKS_PER_SEC << "\n";
                            // std::cout << "\n";
                            test_tt_build(aig, i, curr_output, curr_output);

                            aig._storage->output_tt[curr_output] = aig._storage->tt_map[curr_output];
                            // std::cout << "truth table built at " << ((float)clock() - start)/CLOCKS_PER_SEC << "\n";
                            float runtime = ((float)clock() - start)/CLOCKS_PER_SEC;
                            // std::cout << "partition=" << i << " ";
                            // std::cout << "output=" << curr_output << " ";
                            aig.foreach_node( [&]( auto node ) {
                                int index = aig.node_to_index(node);
                                aig._storage->nodes[index].data[1].h1 = 0;
                            });
                            // std::cout << computeLevelPart(aig,curr_output,i) << " ";
                            // std::cout << aig._storage->logic_cone_inputs[curr_output].size() << " ";
                            // std::cout << std::fixed << std::setprecision(12) << runtime << "\n";
                        }
                        else{
                            std::cout << "Logic Cone too big\n";
                        }
                        // kitty::print_hex(aig._storage->output_tt[curr_output], std::cout);
                        // std::cout << "\n";
                        // std::cout << "tt for " << curr_output << " = " << kitty::to_hex(aig._storage->output_tt[curr_output]) << "\n";
                    }
                }

            }
            else{
                std::cout << "Partitions not mapped yet\n";
            }
        }
        else{
            std::cout << "No AIG network stored\n";
        }
    }

    class print_karnaugh_command : public alice::command{

    public:
        explicit print_karnaugh_command( const environment::ptr& env )
                : command( env, "Prints all the partitioned truth tables as Karnaugh maps" ){

            opts.add_option( "--filename,filename", filename, "Classification File to read from" );
            add_flag("--tensor,-t", "Write the k-maps to tensor dataset depending on <filename>");

        }

    protected:
        void execute(){
            if(!store<mockturtle::aig_network>().empty()){
                auto aig = store<mockturtle::aig_network>().current();
                if(aig._storage->num_partitions != 0){
                    if(!aig._storage->output_tt.empty()){

                        if(is_set("tensor")){
                            std::ifstream input(filename);
                            if(!input){
                                std::cout << "Unable to open file\n";
                            }

                            std::string line;
                            while(std::getline(input, line)){
                                // std::cout << "line = " << line << "\n";
                                std::istringstream iss(line);
                                std::vector<std::string> line_parts((std::istream_iterator<std::string>(iss)), std::istream_iterator<std::string>());
                                // std::cout << "line_parts.at(0) = " << line_parts.at(0) << " line_parts.at(1) = " << line_parts.at(1) << "\n";
                                int output = atoi(line_parts.at(0).c_str());
                                int partition = aig._storage->partitionMap[output];
                                int class_cat = atoi(line_parts.at(1).c_str());
                                std::string file_out = aig._storage->net_name + "_kar_part_" + std::to_string(partition) + "_out_" +
                                                       std::to_string(output) + ".txt";
                                std::string dir;

                                if(class_cat == 1){//AIG optimizer
                                    std::size_t found_delay = filename.find("delay");
                                    std::size_t found_area = filename.find("area");
                                    if(found_delay != std::string::npos){
                                        // std::cout << "delay\n";
                                        dir = "/home/u0812227/LSOracle/MaxAustin/build/testing/tensor/dataset/ABC_AIG_DELAY/";
                                    }
                                    else if(found_area != std::string::npos){
                                        // std::cout << "area\n";
                                        dir = "/home/u0812227/LSOracle/MaxAustin/build/testing/tensor/dataset/ABC_AIG_AREA/";
                                    }
                                    else{
                                        // std::cout << "delay and area\n";
                                        dir = "/home/u0812227/LSOracle/MaxAustin/build/testing/tensor/dataset/ABC_AIG/";
                                    }
                                }
                                else{//MIG optimizer
                                    std::size_t found_delay = filename.find("delay");
                                    std::size_t found_area = filename.find("area");
                                    if(found_delay != std::string::npos){
                                        // std::cout << "delay\n";
                                        dir = "/home/u0812227/LSOracle/MaxAustin/build/testing/tensor/dataset/CIRKIT_MIG_DELAY/";
                                    }
                                    else if(found_area != std::string::npos){
                                        // std::cout << "area\n";
                                        dir = "/home/u0812227/LSOracle/MaxAustin/build/testing/tensor/dataset/CIRKIT_MIG_AREA/";
                                    }
                                    else{
                                        // std::cout << "delay and area\n";
                                        dir = "/home/u0812227/LSOracle/MaxAustin/build/testing/tensor/dataset/CIRKIT_MIG/";
                                    }
                                }
                                std::ofstream output_file;
                                output_file.open(dir + file_out);
                                std::string tt = kitty::to_binary(aig._storage->output_tt[output]);
                                char* tt_binary = malloc(sizeof(char) * (tt.length() + 1));
                                strcpy(tt_binary, tt.c_str());

                                std::sort(aig._storage->partitionInputs[partition].begin(), aig._storage->partitionInputs[partition].end());
                                // std::cout << "partition inputs: ";
                                // for(int k = 0; k < aig._storage->partitionInputs[partition].size(); k++){
                                // 	std::cout << aig._storage->partitionInputs[partition].at(k) << " ";
                                // }
                                // std::cout << "\n";
                                // std::cout << "logic cone inputs: ";
                                // for(int k = 0; k < aig._storage->logic_cone_inputs[output].size(); k++){
                                // 	std::cout << aig._storage->logic_cone_inputs[output].at(k) << " ";
                                // }
                                // std::cout << "\n";
                                std::vector<std::string> onset_indeces;
                                int indx = 0;
                                for(int k = tt.length() - 1; k >= 0; k--){
                                    int bit = (int)tt_binary[k] - 48;
                                    if(bit == 1){
                                        // std::cout << "index for 1 = " << indx << "\n";
                                        onset_indeces.push_back(to_binary(indx));
                                    }
                                    indx++;
                                }
                                for(int k = 0; k < onset_indeces.size(); k++){
                                    // std::cout << "number of inputs in partition " << partition << " = " << aig._storage->partitionInputs[partition].size() << "\n";
                                    // std::cout << "orig binary = " << onset_indeces.at(k) << "\n";
                                    while(onset_indeces.at(k).length() != aig._storage->logic_cone_inputs[output].size()){
                                        onset_indeces.at(k).insert(0, "0");
                                    }
                                    std::reverse(onset_indeces.at(k).begin(), onset_indeces.at(k).end());
                                }
                                int num_inputs = aig._storage->logic_cone_inputs[output].size();
                                int columns = num_inputs / 2;
                                int rows;
                                if(num_inputs % 2 != 0){
                                    rows = columns + 1;
                                }
                                else{
                                    rows = columns;
                                }

                                int row_num = pow(2, rows);
                                int col_num = pow(2, columns);
                                //output_file << row_num << " " << col_num << "\n";
                                char **k_map = malloc(sizeof(char *) * row_num);
                                for(int k = 0; k < row_num; k++)
                                    k_map[k] = malloc(sizeof(char) * col_num);

                                for(int k = 0; k < row_num; k++){
                                    for(int l = 0; l < col_num; l++){
                                        k_map[k][l] = '1';
                                    }
                                }
                                for(int k = 0; k < onset_indeces.size(); k++){
                                    std::string row_index_gray = onset_indeces.at(k).substr(0, rows);
                                    std::string col_index_gray = onset_indeces.at(k).substr(rows, onset_indeces.at(k).size() - 1);
                                    std::string row_index_bin = graytoBinary(row_index_gray);
                                    std::string col_index_bin = graytoBinary(col_index_gray);
                                    int row_index = std::stoi(row_index_bin,nullptr,2);
                                    int col_index = std::stoi(col_index_bin,nullptr,2);
                                    k_map[row_index][col_index] = '2';
                                }
                                // std::cout << "k-map for " << output << ":\n";
                                // std::cout << "------------------------------\n";
                                for(int k = 0; k < row_num; k++){
                                    for(int l = 0; l < col_num; l++){
                                        // std::cout << k_map[k][l] << " ";
                                        output_file << k_map[k][l];
                                        if(l != col_num - 1){
                                            // std::cout << ",";
                                            output_file << ",";
                                        }
                                    }
                                    // std::cout << "\n";
                                    output_file << "\n";
                                }

                                output_file.close();

                            }

                        }
                        else{
                            for(int i = 0; i < aig._storage->num_partitions; i++){
                                for(int j = 0; j < aig._storage->partitionOutputs[i].size(); j++){

                                    std::string dir = aig._storage->net_name + "_testing/";
                                    mkdir(dir.c_str(), 0777);

                                    std::string filename = aig._storage->net_name + "_kar_part_" + std::to_string(i) + "_out_" +
                                                           std::to_string(aig._storage->partitionOutputs[i].at(j)) + ".txt";
                                    std::ofstream output;
                                    output.open("./" + dir + filename);

                                    int curr_output = aig._storage->partitionOutputs[i].at(j);
                                    std::string tt = kitty::to_binary(aig._storage->output_tt[curr_output]);
                                    char* tt_binary = malloc(sizeof(char) * (tt.length() + 1));
                                    strcpy(tt_binary, tt.c_str());

                                    std::sort(aig._storage->partitionInputs[i].begin(), aig._storage->partitionInputs[i].end());
                                    // std::cout << "partition inputs: ";
                                    // for(int k = 0; k < aig._storage->partitionInputs[i].size(); k++){
                                    // 	std::cout << aig._storage->partitionInputs[i].at(k) << " ";
                                    // }
                                    // std::cout << "\n";
                                    // std::cout << "logic cone inputs: ";
                                    // for(int k = 0; k < aig._storage->logic_cone_inputs[curr_output].size(); k++){
                                    // 	std::cout << aig._storage->logic_cone_inputs[curr_output].at(k) << " ";
                                    // }
                                    // std::cout << "\n";
                                    std::vector<std::string> onset_indeces;
                                    int indx = 0;
                                    for(int k = tt.length() - 1; k >= 0; k--){
                                        int bit = (int)tt_binary[k] - 48;
                                        if(bit == 1){
                                            // std::cout << "index for 1 = " << indx << "\n";
                                            onset_indeces.push_back(to_binary(indx));
                                        }
                                        indx++;
                                    }
                                    for(int k = 0; k < onset_indeces.size(); k++){
                                        // std::cout << "number of inputs in partition " << partition << " = " << aig._storage->partitionInputs[partition].size() << "\n";
                                        // std::cout << "orig binary = " << onset_indeces.at(k) << "\n";
                                        while(onset_indeces.at(k).length() != aig._storage->logic_cone_inputs[curr_output].size()){
                                            onset_indeces.at(k).insert(0, "0");
                                        }
                                        std::reverse(onset_indeces.at(k).begin(), onset_indeces.at(k).end());
                                    }
                                    int num_inputs = aig._storage->logic_cone_inputs[curr_output].size();
                                    int columns = num_inputs / 2;
                                    int rows;
                                    if(num_inputs % 2 != 0){
                                        rows = columns + 1;
                                    }
                                    else{
                                        rows = columns;
                                    }

                                    int row_num = pow(2, rows);
                                    int col_num = pow(2, columns);
                                    char **k_map = malloc(sizeof(char *) * row_num);
                                    for(int k = 0; k < row_num; k++)
                                        k_map[k] = malloc(sizeof(char) * col_num);

                                    for(int k = 0; k < row_num; k++){
                                        for(int l = 0; l < col_num; l++){
                                            k_map[k][l] = '1';
                                        }
                                    }
                                    for(int k = 0; k < onset_indeces.size(); k++){
                                        std::string row_index_gray = onset_indeces.at(k).substr(0, rows);
                                        std::string col_index_gray = onset_indeces.at(k).substr(rows, onset_indeces.at(k).size() - 1);
                                        std::string row_index_bin = graytoBinary(row_index_gray);
                                        std::string col_index_bin = graytoBinary(col_index_gray);
                                        int row_index = std::stoi(row_index_bin,nullptr,2);
                                        int col_index = std::stoi(col_index_bin,nullptr,2);
                                        k_map[row_index][col_index] = '2';
                                    }
                                    // std::cout << "k-map for " << curr_output << ":\n";
                                    // std::cout << "------------------------------\n";
                                    for(int k = 0; k < row_num; k++){
                                        for(int l = 0; l < col_num; l++){
                                            // std::cout << k_map[k][l];
                                            output << k_map[k][l];
                                            if(l != col_num - 1){
                                                // std::cout << ",";
                                                output << ",";
                                            }
                                        }
                                        // std::cout << "\n";
                                        output << "\n";
                                    }

                                    output.close();
                                }
                            }

                        }
                    }
                    else{
                        std::cout << "Truth tables not built yet\n";
                    }
                }
                else{
                    std::cout << "Partitions not mapped yet\n";
                }
            }
            else{
                std::cout << "No AIG network stored\n";
            }
        }

    private:
        std::string filename{};
    };

    ALICE_ADD_COMMAND(print_karnaugh, "IO");

    // ALICE_COMMAND( print_karnaugh, "IO", "Prints all the partitioned truth tables as Karnaugh maps"){
    // 	if(!store<mockturtle::aig_network>().empty()){
    // 		auto aig = store<mockturtle::aig_network>().current();
    // 		if(aig._storage->num_partitions != 0){
    // 			if(!aig._storage->output_tt.empty()){
    // 				for(int i = 0; i < aig._storage->num_partitions; i++){
    // 					for(int j = 0; j < aig._storage->partitionOutputs[i].size(); j++){

    // 						std::string dir = aig._storage->net_name + "_testing/";
    // 						mkdir(dir.c_str(), 0777);

    // 						std::string filename = aig._storage->net_name + "_kar_part_" + std::to_string(i) + "_out_" +
    // 							std::to_string(aig._storage->partitionOutputs[i].at(j)) + ".txt";
    // 						std::ofstream output;
    //  						output.open("./" + dir + filename);

    // 						int curr_output = aig._storage->partitionOutputs[i].at(j);
    // 						std::string tt = kitty::to_binary(aig._storage->output_tt[curr_output]);
    // 			  			char* tt_binary = malloc(sizeof(char) * (tt.length() + 1));
    // 			  			strcpy(tt_binary, tt.c_str());

    // 			  			std::sort(aig._storage->partitionInputs[i].begin(), aig._storage->partitionInputs[i].end());
    // 			  			std::cout << "partition inputs: ";
    // 			  			for(int k = 0; k < aig._storage->partitionInputs[i].size(); k++){
    // 			  				std::cout << aig._storage->partitionInputs[i].at(k) << " ";
    // 			  			}
    // 			  			std::cout << "\n";
    // 			  			std::cout << "logic cone inputs: ";
    // 			  			for(int k = 0; k < aig._storage->logic_cone_inputs[curr_output].size(); k++){
    // 			  				std::cout << aig._storage->logic_cone_inputs[curr_output].at(k) << " ";
    // 			  			}
    // 			  			std::cout << "\n";
    // 			  			std::vector<std::string> onset_indeces;
    // 				  		int indx = 0;
    // 				  		for(int k = tt.length() - 1; k >= 0; k--){
    // 				  			int bit = (int)tt_binary[k] - 48;
    // 				  			if(bit == 1){
    // 				  				// std::cout << "index for 1 = " << indx << "\n";
    // 				  				onset_indeces.push_back(to_binary(indx));
    // 				  			}
    // 				  			indx++;
    // 				  		}
    // 				  		for(int k = 0; k < onset_indeces.size(); k++){
    // 				  			// std::cout << "number of inputs in partition " << partition << " = " << aig._storage->partitionInputs[partition].size() << "\n";
    // 				  			// std::cout << "orig binary = " << onset_indeces.at(k) << "\n";
    // 				  			while(onset_indeces.at(k).length() != aig._storage->logic_cone_inputs[curr_output].size()){
    // 				  				onset_indeces.at(k).insert(0, "0");
    // 				  			}
    // 				  			std::reverse(onset_indeces.at(k).begin(), onset_indeces.at(k).end());
    // 				  		}
    // 				  		int num_inputs = aig._storage->logic_cone_inputs[curr_output].size();
    // 			  			int columns = num_inputs / 2;
    // 			  			int rows;
    // 			  			if(num_inputs % 2 != 0){
    // 							rows = columns + 1;
    // 			  			}
    // 						else{
    // 							rows = columns;
    // 						}

    // 						int row_num = pow(2, rows);
    // 						int col_num = pow(2, columns);
    // 						output << row_num << " " << col_num << "\n";
    // 						char **k_map = malloc(sizeof(char *) * row_num);
    // 						for(int k = 0; k < row_num; k++)
    // 							k_map[k] = malloc(sizeof(char) * col_num);

    // 						for(int k = 0; k < row_num; k++){
    // 							for(int l = 0; l < col_num; l++){
    // 								k_map[k][l] = '0';
    // 							}
    // 						}
    // 						for(int k = 0; k < onset_indeces.size(); k++){
    // 							std::string row_index_gray = onset_indeces.at(k).substr(0, rows);
    // 							std::string col_index_gray = onset_indeces.at(k).substr(rows, onset_indeces.at(k).size() - 1);
    // 							std::string row_index_bin = graytoBinary(row_index_gray);
    // 							std::string col_index_bin = graytoBinary(col_index_gray);
    // 							int row_index = std::stoi(row_index_bin,nullptr,2);
    // 							int col_index = std::stoi(col_index_bin,nullptr,2);
    // 							k_map[row_index][col_index] = '1';
    // 						}
    // 						std::cout << "k-map for " << curr_output << ":\n";
    // 						std::cout << "------------------------------\n";
    // 						for(int k = 0; k < row_num; k++){
    // 							for(int l = 0; l < col_num; l++){
    // 								std::cout << k_map[k][l] << " ";
    // 								output << k_map[k][l] << " ";
    // 							}
    // 							std::cout << "\n";
    // 							output << "\n";
    // 						}

    // 						output.close();
    // 					}
    // 				}

    // 			}

    // 			else{
    // 				std::cout << "Truth tables not built yet\n";
    // 			}
    // 		}
    // 		else{
    // 			std::cout << "Partitions not mapped yet\n";
    // 		}
    // 	}
    // 	else{
    // 		std::cout << "No AIG network stored\n";
    // 	}
    // }

    ALICE_COMMAND( print_full_blif, "IO", "Prints all the partitioned truth tables in one blif file"){
        if(!store<mockturtle::aig_network>().empty()){
            auto aig = store<mockturtle::aig_network>().current();
            if(aig._storage->num_partitions != 0){
                if(!aig._storage->output_tt.empty()){
                    new_tt_blif(aig);
                }
                else{
                    std::cout << "Truth tables not built yet\n";
                }
            }
            else{
                std::cout << "Partitions not mapped yet\n";
            }
        }
        else{
            std::cout << "No AIG network stored\n";
        }
    }

    ALICE_COMMAND( test_merge_aigs, "Test", "Merge stored AIG with tiny test AIG"){
        if(!store<mockturtle::aig_network>().empty()){
            auto aig = store<mockturtle::aig_network>().current();
            std::string filename2 = "test_aig2.aag";
            std::string str = "10";
            std::cout << "beginning of str = " << str.at(0) << "\n";
            std::cout << "isdigit = " << isdigit(str.at(0)) << "\n";
            mockturtle::aig_network aig2;
            const auto a = aig2.create_pi();
            aig2.create_in_name(aig2.node_to_index(aig2.get_node(a)), "10");
            const auto b = aig2.create_pi();
            aig2.create_in_name(aig2.node_to_index(aig2.get_node(b)), "b");
            const auto c = aig2.create_pi();
            aig2.create_in_name(aig2.node_to_index(aig2.get_node(c)), "c");

            const auto e = aig2.create_pi();
            aig2.create_in_name(aig2.node_to_index(aig2.get_node(e)), "e");
            const auto f = aig2.create_pi();
            aig2.create_in_name(aig2.node_to_index(aig2.get_node(f)), "f");

            const auto f1 = aig2.create_and( b, c );
            const auto f2 = aig2.create_and( a, f1 );
            aig2.create_po( f2 );
            aig2.create_out_name(aig2.node_to_index(aig2.get_node(f2)), "f2");

            const auto f3 =  aig2.create_and( e, f );
            aig2.create_po(f3);
            aig2.create_out_name(aig2.node_to_index(aig2.get_node(f3)), "10");

            aig2.substitute_node(aig2.get_node(a), f3);
            int inputIdx = get_input_index(aig2, aig2.node_to_index(aig2.get_node(a)));
            int outputIdx = get_output_index(aig2, aig2.node_to_index(aig2.get_node(f3)));

            std::cout << "Inputs = ";
            for(int i = 0; i < aig2._storage->inputs.size(); i++){
                std::cout << aig2._storage->inputs.at(i) << " ";
            }
            std::cout << "\n";

            std::cout << "Outputs = ";
            for(int i = 0; i < aig2._storage->outputs.size(); i++){
                std::cout << aig2._storage->outputs.at(i).index << " ";
            }
            std::cout << "\n";

            std::cout << "erasing input\n";
            aig2._storage->inputs.erase(aig2._storage->inputs.begin() + inputIdx);
            std::cout << "erasing output\n";
            aig2._storage->outputs.erase(aig2._storage->outputs.begin() + outputIdx);

            std::cout << "Inputs = ";
            for(int i = 0; i < aig2._storage->inputs.size(); i++){

                aig2._storage->inputs.at(i) -= 1;

                std::cout << aig2._storage->inputs.at(i) << " ";
            }
            std::cout << "\n";

            std::cout << "Outputs = ";
            for(int i = 0; i < aig2._storage->outputs.size(); i++){
                std::cout << aig2._storage->outputs.at(i).index << " ";
            }
            std::cout << "\n";


            write_aig(aig2, filename2);

        }
        else{
            std::cout << "No AIG network stored\n";
        }
    }

    ALICE_COMMAND( create_paritioned_aigs, "Test", "Writes an aag representation of each partition of the stored AIG"){
        if(!store<mockturtle::aig_network>().empty()){
            auto aig = store<mockturtle::aig_network>().current();
            std::vector<mockturtle::aig_network> aig_parts;
            if(aig._storage->num_partitions != 0){
                for(int i = 0; i < aig._storage->num_partitions; i++){
                    for(int j = 0; j < aig._storage->partitionOutputs[i].size(); j++){
                        mockturtle::aig_network temp = create_aig_from_part(aig, i, aig._storage->partitionOutputs[i].at(j));
                        aig_parts.push_back(temp);
                    }
                }
                std::string filename;
                std::string path = "./" + aig._storage->net_name + "_aig_parts/";
                for(int i = 0; i < aig_parts.size(); i++){
                    filename = path + aig_parts.at(i)._storage->net_name + ".aag";
                    std::cout << "writing to " << filename << "\n";
                    write_aig(aig_parts.at(i), filename);
                }
            }
            else{
                std::cout << "Partitions not mapped yet\n";
            }
        }
        else{
            std::cout << "No AIG network stored\n";
        }
    }

    ALICE_COMMAND( calc_partition_level, "Test", "Returns the depth of every logic cone in every partition"){
        if(!store<mockturtle::aig_network>().empty()){
            auto aig = store<mockturtle::aig_network>().current();
            if(aig._storage->num_partitions != 0){

                for(int i = 0; i < aig._storage->num_partitions; i++){
                    for(int j = 0; j < aig._storage->partitionOutputs[i].size(); j++){
                        //Mark all nodes as not visited
                        aig.foreach_node( [&]( auto node ) {
                            int index = aig.node_to_index(node);
                            aig._storage->nodes[index].data[1].h1 = 0;
                        });
                        std::cout << /*"partition = " <<*/ i << " ";
                        std::cout << /*"output = " <<*/ aig._storage->partitionOutputs[i].at(j) << " ";
                        std::cout << /*"level = " <<*/ computeLevelPart(aig, aig._storage->partitionOutputs[i].at(j), i) << "\n";
                    }
                }
            }
            else{
                std::cout << "Partitions not mapped yet\n";
            }
        }
        else{
            std::cout << "No AIG network stored\n";
        }
    }



    ALICE_COMMAND( merge_aig, "Test", "Merges partitioned AIGs in the original AIG's respective folder"){

        mockturtle::aig_network merged_aig;

        if(!store<mockturtle::aig_network>().empty()){
            auto aig = store<mockturtle::aig_network>().current();
            // mockturtle::aig_network
            if(aig._storage->num_partitions != 0){

                std::string root = aig._storage->net_name;
                std::string ext = ".aig";
                std::string path = "./" + root + "_aig_parts_full/" + root + "_" + std::to_string(aig._storage->num_partitions) + "_aig/";
                std::string filename;
                std::vector<mockturtle::aig_network> aig_parts;
                int size_of_merge = 0;
                for(int i = 0; i < aig._storage->num_partitions; i++){
                    filename = path + root + "_" + std::to_string(i) + ext;
                    // for(int j = 0; j < aig._storage->partitionOutputs[i].size(); j++){

                    mockturtle::aig_network temp;
                    std::cout << "filename = " << filename << "\n";
                    lorina::read_aiger(filename, mockturtle::aiger_reader( temp ));
                    aig_parts.push_back(temp);
                    temp.foreach_node( [&]( auto node ) {
                        int nodeIdx = temp.node_to_index(node);
                        std::cout << "node " << nodeIdx << " child[0] = " << temp._storage->nodes[node].children[0].index << "\n";
                        std::cout << "node " << nodeIdx << " child[1] = " << temp._storage->nodes[node].children[1].index << "\n";
                    });
                    std::cout << "Inputs = ";
                    for(int i = 0; i < temp._storage->inputs.size(); i++){
                        std::cout << temp._storage->inputNames[i] << " ";
                    }
                    std::cout << "\n";
                    std::cout << "Outputs = ";
                    for(int i = 0; i < temp._storage->outputs.size(); i++){
                        std::cout << temp._storage->outputNames[i] << " ";
                    }
                    std::cout << "\n";
                    size_of_merge += temp._storage->nodes.size();
                    //}

                }
                merged_aig._storage->nodes.reserve(size_of_merge);
                std::cout << "size of merged aig = " << size_of_merge << "\n";
                std::cout << "num_inputs merged_aig = " << aig.num_pis() << "\n";
                std::cout << "num aigs = " << aig_parts.size() << "\n";
                int max_index = aig.num_pis();
                int max_input = 0;
                std::vector<std::string> node_connection_name;

                // std::vector<std::string> inputs_visited;
                // std::map<std::string, int> visited_input_indeces;
                // std::map<std::string, std::vector<std::vector<int>>> connected_input;

                std::map<std::string, int> node_connection_index;
                bool first_gate = true;
                int index_offset = 0;

                int input_offset = 0;
                int output_offset = 0;
                for(int idx = 0; idx < aig_parts.size(); idx++){
                    mockturtle::aig_network curr_aig = aig_parts.at(idx);
                    int temp_max = 0;
                    int temp_max_input = 0;
                    std::cout << "max_input = " << max_input << "\n";
                    std::vector<int> nodes_to_erase;

                    curr_aig.foreach_node( [&]( auto node ) {
                        int nodeIdx = curr_aig.node_to_index(node);

                        // std::vector<int> erased_node_fanout;
                        if(nodeIdx + max_index > temp_max)
                            temp_max = nodeIdx + max_index;

                        //Check if either a primary output or input is shared with another AIG
                        if(curr_aig.is_po(nodeIdx)){
                            std::string name = curr_aig._storage->outputNames[get_output_index(curr_aig, nodeIdx)];
                            std::cout << "name = " << name << "\n";
                            bool connected_to_other_aig_in = (name.find_first_not_of("0123456789") == std::string::npos);
                            if(connected_to_other_aig_in){
                                std::cout << "NUMBER\n";
                                if(!is_in_string_vector(node_connection_name, name)){/* && connected_input.find(name) == connected_input.end()){*/
                                    std::cout << "adding " << std::stoi(name) << " to node_connection_name\n";
                                    std::cout << "adding " << nodeIdx << " to node_connection_index\n";
                                    node_connection_name.push_back(name);
                                    if(first_gate){
                                        index_offset = (aig.num_pis() + 1) - nodeIdx;
                                        first_gate = false;
                                    }
                                    node_connection_index[name] = nodeIdx + index_offset;
                                }
                                // else{
                                // 	std::cout << "Already an input somewhere\n";
                                // 	for(int i = 0; i < connected_input[name].size(); i++){
                                // 		std::cout << "connected aig index = " << connected_input[name].at(i).at(0) << "\n";
                                // 		mockturtle::aig_network conn_aig = aig_parts.at(connected_input[name].at(i).at(0));
                                // 		std::cout << "fanout size = " << connected_input[name].at(i).size() << "\n";
                                // 		for(int j = 1; j < connected_input[name].at(i).size(); j++){
                                // 			std::cout << "fanout index = " << connected_input[name].at(i).at(j) << "\n";
                                // 			std::cout << "name at child[0] = " << conn_aig._storage->inputNames[conn_aig._storage->nodes[node].children[0].index - 1] << "\n";
                                // 			std::cout << "name at child[1] = " << conn_aig._storage->inputNames[conn_aig._storage->nodes[node].children[1].index - 1] << "\n";
                                // 			auto node = conn_aig.index_to_node(connected_input[name].at(i).at(j));
                                // 			if(conn_aig._storage->inputNames[conn_aig._storage->nodes[node].children[0].index - 1].compare(name) == 0){
                                // 				std::cout << "updating child [0] of " << connected_input[name].at(i).at(j) << "\n";
                                // 				conn_aig._storage->nodes[node].children[0].index += nodeIdx + index_offset;
                                // 			}
                                // 			else if(conn_aig._storage->inputNames[conn_aig._storage->nodes[node].children[1].index - 1].compare(name) == 0){
                                // 				std::cout << "updating child [1] of " << connected_input[name].at(i).at(j) << "\n";
                                // 				conn_aig._storage->nodes[node].children[1].index += nodeIdx + index_offset;
                                // 			}
                                // 		}
                                // 	}
                                // 	node_connection_index[name] = nodeIdx + index_offset;
                                // }
                            }
                        }
                        if(curr_aig.is_pi(nodeIdx)){

                            std::string name = curr_aig._storage->inputNames[nodeIdx - 1];
                            std::cout << "name = " << name << "\n";

                            if(nodeIdx > temp_max_input)
                                temp_max_input = nodeIdx;
                            bool connected_to_other_aig_out = (name.find_first_not_of("0123456789") == std::string::npos);
                            if(connected_to_other_aig_out){

                                if(!is_in_string_vector(node_connection_name, name)){
                                    std::cout << "adding " << nodeIdx << " to node_connection_index\n";
                                    // std::vector<int> temp;
                                    std::cout << "aig index = " << idx << "\n";
                                    node_connection_index[name] = nodeIdx;
                                    node_connection_name.push_back(name);
                                    // temp.push_back(idx);
                                    // mockturtle::fanout_view fanout_aig{curr_aig};
                                    // std::set<mockturtle::node<mockturtle::aig_network>> nodes;
                                    // fanout_aig.foreach_fanout(curr_aig.index_to_node(nodeIdx), [&](const auto& p){
                                    // 	nodes.insert(p);
                                    // });
                                    // std::cout << "fanout: ";
                                    // for(std::set<mockturtle::node<mockturtle::aig_network>>::iterator it = nodes.begin(); it != nodes.end(); ++it){
                                    // 	std::cout << curr_aig.node_to_index(*it) - 1 << " ";
                                    // 	temp.push_back(curr_aig.node_to_index(*it) - 1);
                                    // }
                                    // std::cout << "\n";
                                    // connected_input[name].push_back(temp);
                                }
                                //Erase from nodes so the merged AIG does not have duplicate nodes

                                std::cout << "marking to erase " << nodeIdx << "\n";
                                nodes_to_erase.push_back(nodeIdx);
                                max_input++;
                                // curr_aig._storage->nodes.erase(curr_aig._storage->nodes.begin() + nodeIdx);

                            }
                            // else{
                            // 	// if(!is_in_string_vector(inputs_visited, name)){
                            // 	// 	inputs_visited.push_back(name);
                            // 	// 	std::cout << "adding " << nodeIdx << " as input index for " << name << "\n";
                            // 	// 	visited_input_indeces[name] = nodeIdx + max_input;
                            // 	// }
                            // 	// else{
                            // 	// 	std::cout << "Duplicate Input\n";
                            // 		if(!is_in_string_vector(node_connection_name, name)){
                            // 			std::cout << "adding " << name << " to node_connection_name\n";
                            // 			std::cout << "adding " << nodeIdx << " to node_connection_index\n";
                            // 			node_connection_index[name] = visited_input_indeces[name];
                            // 			node_connection_name.push_back(name);
                            // 		}

                            // 		std::cout << "marking to erase " << nodeIdx << "\n";
                            // 		nodes_to_erase.push_back(nodeIdx);
                            // 		max_input++;

                            // 	//}
                            // }
                        }
                        std::cout << "node " << nodeIdx << " child[0] = " << curr_aig._storage->nodes[node].children[0].index << "\n";
                        std::cout << "node " << nodeIdx << " child[1] = " << curr_aig._storage->nodes[node].children[1].index << "\n";


                        if(!curr_aig.is_pi(nodeIdx) && !curr_aig.is_constant(nodeIdx)){

                            if(first_gate){
                                std::cout << "num_pis = " << aig.num_pis() + 1 << " nodeIdx " << nodeIdx << "\n";
                                index_offset = (aig.num_pis() + 1) - nodeIdx;
                                first_gate = false;
                            }
                            std::cout << "index_offset = " << index_offset << "\n";
                            if(is_in_vector(nodes_to_erase, curr_aig._storage->nodes[node].children[0].index)
                               && node_connection_index.find(curr_aig._storage->inputNames[curr_aig._storage->nodes[node].children[0].index - 1]) != node_connection_index.end()){
                                std::cout << "child 0 is connected with name " << curr_aig._storage->inputNames[curr_aig._storage->nodes[node].children[0].index - 1] << "\n";
                                curr_aig._storage->nodes[node].children[0].index = node_connection_index[curr_aig._storage->inputNames[curr_aig._storage->nodes[node].children[0].index - 1]];

                                // if(!curr_aig.is_pi(curr_aig._storage->nodes[node].children[1].index)){
                                // 	int offset = 0;
                                // 	for(int i = 0; i < nodes_to_erase.size(); i++){
                                // 		if(curr_aig._storage->nodes[node].children[1].index > nodes_to_erase.at(i))
                                // 			offset++;
                                // 	}
                                // 	curr_aig._storage->nodes[node].children[1].index += (max_index - offset);
                                // }
                                // else{
                                if(!curr_aig.is_pi(curr_aig._storage->nodes[node].children[1].index))
                                    curr_aig._storage->nodes[node].children[1].index += index_offset;
                                else{
                                    int offset = 0;
                                    for(int i = 0; i < nodes_to_erase.size(); i++){
                                        if(curr_aig._storage->nodes[node].children[1].index > nodes_to_erase.at(i))
                                            offset++;
                                    }
                                    curr_aig._storage->nodes[node].children[1].index += (max_input - offset);
                                }

                                //}
                            }
                            else if(is_in_vector(nodes_to_erase, curr_aig._storage->nodes[node].children[1].index)
                                    && node_connection_index.find(curr_aig._storage->inputNames[curr_aig._storage->nodes[node].children[1].index - 1]) != node_connection_index.end()){
                                std::cout << "child 1 is connected with name " << curr_aig._storage->inputNames[curr_aig._storage->nodes[node].children[1].index - 1] << "\n";
                                // if(!curr_aig.is_pi(curr_aig._storage->nodes[node].children[0].index)){
                                // 	int offset = 0;
                                // 	for(int i = 0; i < nodes_to_erase.size(); i++){
                                // 		if(curr_aig._storage->nodes[node].children[0].index > nodes_to_erase.at(i))
                                // 			offset++;
                                // 	}
                                // 	curr_aig._storage->nodes[node].children[0].index += (max_index - offset);
                                // }
                                // else{
                                if(!curr_aig.is_pi(curr_aig._storage->nodes[node].children[0].index))
                                    curr_aig._storage->nodes[node].children[0].index += index_offset;
                                else{
                                    int offset = 0;
                                    for(int i = 0; i < nodes_to_erase.size(); i++){
                                        if(curr_aig._storage->nodes[node].children[0].index > nodes_to_erase.at(i))
                                            offset++;
                                    }
                                    curr_aig._storage->nodes[node].children[0].index += (max_input - offset);
                                }

                                //}
                                curr_aig._storage->nodes[node].children[1].index = node_connection_index[curr_aig._storage->inputNames[curr_aig._storage->nodes[node].children[1].index - 1]];
                            }
                            else if(is_in_vector(nodes_to_erase, curr_aig._storage->nodes[node].children[0].index) && is_in_vector(nodes_to_erase, curr_aig._storage->nodes[node].children[1].index)
                                    && node_connection_index.find(curr_aig._storage->inputNames[curr_aig._storage->nodes[node].children[0].index - 1]) != node_connection_index.end()
                                    && node_connection_index.find(curr_aig._storage->inputNames[curr_aig._storage->nodes[node].children[1].index - 1]) != node_connection_index.end()){
                                std::cout << "child 0 and 1 is connected\n";
                                curr_aig._storage->nodes[node].children[0].index = node_connection_index[curr_aig._storage->inputNames[curr_aig._storage->nodes[node].children[0].index - 1]];
                                curr_aig._storage->nodes[node].children[1].index = node_connection_index[curr_aig._storage->inputNames[curr_aig._storage->nodes[node].children[1].index - 1]];
                            }
                            else{
                                std::cout << "Neither are connected\n";
                                // if(!curr_aig.is_pi(curr_aig._storage->nodes[node].children[0].index)){
                                // 	int offset1 = 0;

                                // 	for(int i = 0; i < nodes_to_erase.size(); i++){
                                // 		if(curr_aig._storage->nodes[node].children[0].index > nodes_to_erase.at(i))
                                // 			offset1++;
                                // 	}
                                // 	curr_aig._storage->nodes[node].children[0].index += (max_index - offset1);
                                // }
                                // else{
                                if(!curr_aig.is_pi(curr_aig._storage->nodes[node].children[0].index))
                                    curr_aig._storage->nodes[node].children[0].index += index_offset;
                                else{
                                    int offset = 0;
                                    for(int i = 0; i < nodes_to_erase.size(); i++){
                                        if(curr_aig._storage->nodes[node].children[0].index > nodes_to_erase.at(i))
                                            offset++;
                                    }
                                    curr_aig._storage->nodes[node].children[0].index += (max_input - offset);
                                }

                                //}
                                // if(!curr_aig.is_pi(curr_aig._storage->nodes[node].children[1].index)){
                                // 	int offset2 = 0;
                                // 	for(int i = 0; i < nodes_to_erase.size(); i++){
                                // 		if(curr_aig._storage->nodes[node].children[1].index > nodes_to_erase.at(i))
                                // 			offset2++;
                                // 	}

                                // 	curr_aig._storage->nodes[node].children[1].index += (max_index - offset2);
                                // }
                                // else{
                                if(!curr_aig.is_pi(curr_aig._storage->nodes[node].children[1].index))
                                    curr_aig._storage->nodes[node].children[1].index += index_offset;
                                else{
                                    int offset = 0;
                                    for(int i = 0; i < nodes_to_erase.size(); i++){
                                        if(curr_aig._storage->nodes[node].children[1].index > nodes_to_erase.at(i))
                                            offset++;
                                    }
                                    curr_aig._storage->nodes[node].children[1].index += (max_input - offset);
                                }

                                //}
                            }

                        }

                        std::cout << "node " << nodeIdx << " child[0] = " << curr_aig._storage->nodes[node].children[0].index << "\n";
                        std::cout << "node " << nodeIdx << " child[1] = " << curr_aig._storage->nodes[node].children[1].index << "\n";

                    });
                    int offset = 0;
                    for(int i = 0; i < nodes_to_erase.size(); i++){
                        std::cout << "erasing " << nodes_to_erase.at(i) << "\n";
                        curr_aig._storage->inputs.erase(curr_aig._storage->inputs.begin() + nodes_to_erase.at(i) - offset - 1);
                        curr_aig._storage->nodes.erase(curr_aig._storage->nodes.begin() + nodes_to_erase.at(i) - offset);
                        offset++;
                    }
                    if(temp_max > max_index)
                        max_index = temp_max;

                    if(temp_max_input > max_input)
                        max_input = temp_max_input;

                    std::cout << "Inputs:\n";

                    std::map<int,std::string>::iterator it;
                    for (it=curr_aig._storage->inputNames.begin(); it!=curr_aig._storage->inputNames.end(); ++it){
                        std::cout << it->first << ": " << it->second << "\n";

                        // bool exists = false;
                        // std::map<int,std::string>::iterator it_merge;
                        // for (it_merge=merged_aig._storage->inputNames.begin(); it_merge!=merged_aig._storage->inputNames.end(); ++it_merge){
                        // 	if(it_merge->second == it->second)
                        // 		exists = true;

                        // }
                        if((!is_in_vector(nodes_to_erase, it->first + 1))){
                            merged_aig._storage->inputNames[input_offset] = it->second;
                            merged_aig._storage->inputs.emplace_back( input_offset + 1 );
                            std::cout << "merged_aig = " << merged_aig._storage->inputNames[input_offset + 1] << "\n";
                            input_offset++;
                        }


                    }

                    std::cout << "Outputs:\n";

                    for (it=curr_aig._storage->outputNames.begin(); it!=curr_aig._storage->outputNames.end(); ++it){
                        std::cout << it->first << ": " << it->second << "\n";

                    }

                }

                for(int i = 0; i < aig_parts.size(); i++){
                    mockturtle::aig_network curr_aig = aig_parts.at(i);
                    std::cout << "num pi = " << curr_aig.num_pis() << "\n";

                    merged_aig._storage->nodes.insert(merged_aig._storage->nodes.end(), curr_aig._storage->nodes.begin() + 1, curr_aig._storage->nodes.begin() + curr_aig.num_pis() + 1);
                }
                for(int i = 0; i < aig_parts.size(); i++){
                    mockturtle::aig_network curr_aig = aig_parts.at(i);
                    merged_aig._storage->nodes.insert(merged_aig._storage->nodes.end(), curr_aig._storage->nodes.begin() + curr_aig.num_pis() + 1, curr_aig._storage->nodes.end());
                }

                merged_aig._storage->outputs = aig._storage->outputs;
                merged_aig._storage->outputNames = aig._storage->outputNames;

                std::cout << "MERGED\n";
                merged_aig.foreach_node( [&]( auto node ) {
                    int nodeIdx = merged_aig.node_to_index(node);
                    std::cout << "node " << nodeIdx << " child[0] = " << merged_aig._storage->nodes[node].children[0].index << "\n";
                    std::cout << "node " << nodeIdx << " child[1] = " << merged_aig._storage->nodes[node].children[1].index << "\n";

                });
                std::cout << "Inputs:\n";

                for(int i = 0; i < merged_aig._storage->inputs.size(); i++){
                    std::cout << merged_aig._storage->inputs.at(i) << ": " << merged_aig._storage->inputNames[merged_aig._storage->inputs.at(i) - 1] << "\n";
                }

                std::cout << "Outputs:\n";

                for(int i = 0; i < merged_aig._storage->outputs.size(); i++){
                    std::cout << merged_aig._storage->outputs.at(i).index << ": " << merged_aig._storage->outputNames[i] << "\n";
                }
                write_aig(merged_aig, path + "../" + root + "_merged.aag");

                mockturtle::aig_network merged_aig_simp = simplify_merged(merged_aig);

                std::cout << "SIMPLIFIED MERGED\n";
                merged_aig_simp.foreach_node( [&]( auto node ) {
                    int nodeIdx = merged_aig_simp.node_to_index(node);
                    std::cout << "node " << nodeIdx << " child[0] = " << merged_aig_simp._storage->nodes[node].children[0].index << "\n";
                    std::cout << "node " << nodeIdx << " child[1] = " << merged_aig_simp._storage->nodes[node].children[1].index << "\n";

                });
                std::cout << "Inputs:\n";

                for(int i = 0; i < merged_aig_simp._storage->inputs.size(); i++){
                    std::cout << merged_aig_simp._storage->inputs.at(i) << ": " << merged_aig_simp._storage->inputNames[merged_aig_simp._storage->inputs.at(i) - 1] << "\n";
                }

                std::cout << "Outputs:\n";

                for(int i = 0; i < merged_aig_simp._storage->outputs.size(); i++){
                    std::cout << merged_aig_simp._storage->outputs.at(i).index << ": " << merged_aig_simp._storage->outputNames[i] << "\n";
                }
                write_aig(merged_aig_simp, path + "../" + root + "_merged_simp.aag");
            }
            else{
                std::cout << "Partitions not mapped yet\n";
            }
        }
        else{
            std::cout << "No AIG network stored\n";
        }
    }
    ALICE_COMMAND( aig_in_size, "Stat", "Returns the number of inputs in an AIG network" ){

        if(!store<mockturtle::aig_network>().empty()){
            auto aig = store<mockturtle::aig_network>().current();
            std::cout << aig.num_pis() << "\n";
        }
        else{
            std::cout << "No AIG network stored\n";
        }

    }

    ALICE_COMMAND( tt_algo_stats, "Stat", "Returns various statistics for the truth table generation algorithm" ){

        if(!store<mockturtle::aig_network>().empty()){
            auto aig = store<mockturtle::aig_network>().current();
            for(int i = 0; i < aig._storage->num_partitions; i++){
                std::cout << "\nPartion " << i << "\n";
                std::cout << "---------------------------------------\n";
                std::cout << "Number of Inputs = " << aig._storage->partitionInputs[i].size() << "\n";
                std::cout << "Number of Outputs = " << aig._storage->partitionOutputs[i].size() << "\n\n";
                for(int j = 0; j < aig._storage->partitionOutputs[i].size(); j++){

                    std::cout << "Output " << aig._storage->partitionOutputs[i].at(j) << "\n";
                    std::cout << "Partition Size = " << aig._storage->partitionSize[i] << "\n";
                    std::cout << "Number of Inputs for Output = " << aig._storage->index[aig._storage->partitionOutputs[i].at(j)].size() << "\n";
                    std::cout << "Depth of Logic Cone = " << aig._storage->output_cone_depth[aig._storage->partitionOutputs[i].at(j)] << "\n";
                    if(aig._storage->test_runtime[aig._storage->partitionOutputs[i].at(j)] != 0.0){
                        std::cout << aig._storage->test_runtime[aig._storage->partitionOutputs[i].at(j)] << "\n\n";
                    }
                    else
                        std::cout << "Truth Table Generation Algorithm finished really fast\n\n";
                }
            }

        }
        else{
            std::cout << "No AIG network stored\n";
        }

    }

/*
	ALICE_COMMAND(output_truth_table, "Output", "Output the truth table for each partition in blif format"){

		if(!store<mockturtle::aig_network>().empty()){
			auto aig = store<mockturtle::aig_network>().current();
			if(aig._storage->partitionMap.size() != 0){

				for(int i = 0; i < aig._storage->num_partitions; i++){

					aig.foreach_node( [&]( auto node ) {

						int nodeIdx = aig.node_to_index(node);
						if(aig._storage->partitionMap[nodeIdx] == i){

							std::map<int,std::vector<int>>::iterator it;
							for(int j = 0; j < aig._storage->partitionConn[i][nodeIdx].size(); j++){

								if((aig.is_po(node) || (!is_in_map(aig._storage->partitionConn[i], aig._storage->partitionConn[i][nodeIdx].at(j))))
								 && !is_in_vector(aig._storage->partitionOutputs[i],nodeIdx)){
									aig._storage->partitionOutputs[i].push_back(nodeIdx);
								}
								int child1Idx = aig._storage->nodes[node].children[0].index;
								int child2Idx = aig._storage->nodes[node].children[1].index;
								if(child1Idx < (aig.num_pis() + aig.num_pos()))
	  								child1Idx--;
	  							if(child2Idx < (aig.num_pis() + aig.num_pos()))
	  								child2Idx--;
								bool child1 = is_in_map(aig._storage->partitionConn[i], child1Idx);
								bool child2 = is_in_map(aig._storage->partitionConn[i], child2Idx);

								if((aig._storage->nodes[node].children.size() != 0) && ((child1 && !child2) || (!child1 && child2))){
									if(!child1 && !is_in_vector(aig._storage->partitionInputs[i],child1Idx))
										aig._storage->partitionInputs[i].push_back(child1Idx);
									else if(!child2 && !is_in_vector(aig._storage->partitionInputs[i],child2Idx))
										aig._storage->partitionInputs[i].push_back(child2Idx);
								}

								if( ( (!child1 && !child2) || aig.is_pi(node) ) && !is_in_vector(aig._storage->partitionInputs[i],nodeIdx)){

									aig._storage->partitionInputs[i].push_back(nodeIdx);
								}
							}
						}
					});
				}

				for(int i = 0; i < aig._storage->num_partitions; i++){

					std::cout << "Partition " << i << " Inputs: ";
					for(int j = 0; j < aig._storage->partitionInputs[i].size(); j++){
						std::cout << aig._storage->partitionInputs[i].at(j) << " ";
					}
					std::cout << "\n";
					std::cout << "Partition " << i << " Outputs: ";
					for(int j = 0; j < aig._storage->partitionOutputs[i].size(); j++){
						std::cout << aig._storage->partitionOutputs[i].at(j) << " ";
					}
					std::cout << "\n";
				}

				for(int i = 0; i < aig._storage->num_partitions; i++){

					for(int j = 0; j < aig._storage->partitionOutputs[i].size(); j++){

						aig._storage->truth.clear();
						genTruth(aig, aig._storage->partitionOutputs[0].at(j), 1, i);

					}
				}

				for(int i = 0; i < aig._storage->num_partitions; i++){

					std::cout << "Partition " << 0 << " truth table logic\n";
					for(int j = 0; j < aig._storage->partitionOutputs[0].size(); j++){
						for(int k = 0; k < aig._storage->truth[0][j].size(); k++){
							std::cout << aig._storage->truth[0][j].at(k);
						}
						std::cout << " 1\n";
					}
				}

			}
			else{
				std::cout << "The partitions have not been mapped to the stored aig network\n";
			}
		}
		else{
			std::cout << "There is no stored AIG network\n";
		}
	}
	*/

} // namespace alice

/* Main method for the Alice shell (with prefix) */
ALICE_MAIN( lsoracle )
