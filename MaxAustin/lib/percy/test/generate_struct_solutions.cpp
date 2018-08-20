#include <percy/percy.hpp>
#include <kitty/kitty.hpp>
#include <cassert>
#include <cstdio>
#include <fstream>

using namespace percy;
using kitty::dynamic_truth_table;

/*******************************************************************************
    Tests the generation of multiple solutions from a single spec2ification.
*******************************************************************************/
int main(void)
{
    spec spec;
    spec.verbosity = 0;

    bsat_wrapper solver;
    knuth_encoder encoder(solver);
    chain c;

    dynamic_truth_table tt2(2);
    for (int i = 0; i < 16; i++) {
        kitty::create_from_words(tt2, &i, &i + 1);
        spec[0] = tt2;

        printf("Generating solutions for function ");
        kitty::print_binary(tt2);
        printf("\n");

        encoder.reset();
        while (next_struct_solution(spec, c, solver, encoder) == success) {
            assert(c.get_nr_steps() <= 1);

            printf("Next solution: ");
            c.to_expression(std::cout);
            printf("\n");

            assert(c.satisfies_spec(spec));
        }
    }

    dynamic_truth_table tt3(3);
    for (int i = 0; i < 256; i++) {
        kitty::create_from_words(tt3, &i, &i + 1);
        spec[0] = tt3;

        printf("Generating solutions for function ");
        kitty::print_binary(tt3);
        printf("\n");

        encoder.reset();
        while (next_struct_solution(spec, c, solver, encoder) == success) {
            printf("Next solution: ");
            c.to_expression(std::cout);
            printf("\n");

            assert(c.satisfies_spec(spec));
        }
    }

    spec.fanin = 3;

    dynamic_truth_table tt4(4);
    for (int i = 0; i < 256; i++) {
        kitty::create_from_words(tt4, &i, &i + 1);
        spec[0] = tt4;

        printf("Generating solutions for function ");
        kitty::print_binary(tt4);
        printf("\n");

        encoder.reset();
        while (next_struct_solution(spec, c, solver, encoder) == success) {
            printf("Next solution: (%d vertices)\n", c.get_nr_steps());
            assert(c.satisfies_spec(spec));
        }
    }
    
    // Compare the number of structurally different solutions to the number
    // of structurally OR operationally different ones. As the latter is less
    // restrictive, we expect to see more of these.
    spec.fanin = 2;
    spec.verbosity = 0;

    for (int i = 0; i < 16; i++) {
        kitty::create_from_words(tt2, &i, &i+1);
        spec[0] = tt2;

        auto nr_solutions = 0;
        encoder.reset();
        while (next_solution(spec, c, solver, encoder) == success) {
            ++nr_solutions;
        }

        auto nr_struct_solutions = 0;
        encoder.reset();
        while (next_struct_solution(spec, c, solver, encoder) == success) {
            ++nr_struct_solutions;
        }
        assert(nr_solutions >= nr_struct_solutions);
        printf("nr_solutions=%d, nr_struct_solutions=%d\n",
                nr_solutions, nr_struct_solutions);
    }

    for (int i = 0; i < 256; i++) {
        kitty::create_from_words(tt3, &i, &i+1);
        spec[0] = tt3;

        auto nr_solutions = 0;
        encoder.reset();
        while (next_solution(spec, c, solver, encoder) == success) {
            ++nr_solutions;
        }

        auto nr_struct_solutions = 0;
        encoder.reset();
        while (next_struct_solution(spec, c, solver, encoder) == success) {
            ++nr_struct_solutions;
        }
        assert(nr_solutions >= nr_struct_solutions);
        printf("nr_solutions=%d, nr_struct_solutions=%d\n",
                nr_solutions, nr_struct_solutions);
    }
  
    return 0;
}

