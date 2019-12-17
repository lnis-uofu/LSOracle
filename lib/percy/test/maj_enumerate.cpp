#include <cstdio>
#include <percy/percy.hpp>
#include <chrono>

#define MAX_TESTS 256

using namespace percy;
using kitty::dynamic_truth_table;


int main()
{
    {
        // Synthesize MAJ-3
        mig mig;
        spec spec;
        bsat_wrapper solver;
        maj_encoder encoder(solver);
        kitty::dynamic_truth_table tt(3);
        kitty::create_majority(tt);
        spec[0] = tt;
        
        printf("enumerating solutions for MAJ-3\n");

        int nr_solutions = 0;
        while (next_solution(spec, mig, solver, encoder) == success) {
            printf("got solution: ");
            mig.to_expression(std::cout);
            printf("\n");
            nr_solutions++;
        }
        printf("found %d solutions\n", nr_solutions);
    }
    {
        // Exact synthesis of MAJ-5
        mig mig;
        spec spec;
        bsat_wrapper solver;
        maj_encoder encoder(solver);
        kitty::dynamic_truth_table tt(5);
        kitty::create_majority(tt);
        spec[0] = tt;

        printf("enumerating solutions for MAJ-5\n");

        int nr_solutions = 0;
        while (next_solution(spec, mig, solver, encoder) == success) {
            printf("got solution: ");
            mig.to_expression(std::cout);
            printf("\n");
            nr_solutions++;
        }
        printf("found %d solutions\n", nr_solutions);
    }

    return 0;
}

