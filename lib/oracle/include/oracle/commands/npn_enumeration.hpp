/* adapted from example/npn_enumeration.cpp in kitty */
#include <unordered_map>
#include <string>
#include <regex>
#include <nlohmann/json.hpp>
#include <kitty/kitty.hpp>

namespace oracle
{
    void npn_enumeration(int num_vars){

        /* set to store all NPN representatives */
        std::unordered_set<kitty::dynamic_truth_table, kitty::hash<kitty::dynamic_truth_table>> classes;

        /* initialize truth table (constant 0) */
        kitty::dynamic_truth_table tt (num_vars);

        do
        {
            /* apply NPN canonization and add resulting representative to set */
            const auto res = kitty::exact_npn_canonization( tt );
            classes.insert( std::get<0>( res ) );

            /* increment truth table */
            kitty::next_inplace( tt );
        } while ( !kitty::is_const0( tt ) );

        std::cout << "enumeration of " << num_vars << " input functions into NPN classes\n";

        for (const auto& npnclass : classes){
            std::cout << kitty::to_hex(npnclass) << "\n";
        }
    
    } //npn_enumeration
} //namespace oracle