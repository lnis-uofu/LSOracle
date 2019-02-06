#include <percy/percy.hpp>
#include <cassert>
#include <cstdio>
#include <fstream>

using namespace percy;
using kitty::dynamic_truth_table;
using kitty::static_truth_table;

int main(void)
{
    {
        bsat_wrapper solver;
        ssv_encoder encoder(solver);
        percy::spec s;
        s.initial_steps = 4;
        static_truth_table<3> maj_tt;
        kitty::create_majority(maj_tt);
        s[0] = maj_tt;
        chain c;
        while (next_struct_solution(s, c, solver, encoder) == success) {
            printf("got sol\n");
        }
    }

    return 0;
}

