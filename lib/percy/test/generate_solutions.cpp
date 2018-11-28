#include <percy/percy.hpp>
#include <kitty/kitty.hpp>
#include <cassert>
#include <cstdio>
#include <fstream>

using namespace percy;
using kitty::static_truth_table;

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

    static_truth_table<2> tt2;

    for (int i = 0; i < 16; i++) {
        kitty::create_from_words(tt2, &i, &i + 1);
        spec[0] = tt2;

        printf("Generating solutions for function ");
        kitty::print_binary(tt2);
        printf("\n");

        encoder.reset();
        while (next_solution(spec, c, solver, encoder) == success) {
            assert(c.get_nr_steps() <= 1);

            printf("Next solution: ");
            c.to_expression(std::cout);
            printf("\n");

            assert(c.satisfies_spec(spec));
        }
    }

    static_truth_table<3> tt3;
    for (int i = 0; i < 256; i++) {
        kitty::create_from_words(tt3, &i, &i + 1);
        spec[0] = tt3;

        printf("Generating solutions for function ");
        kitty::print_binary(tt3);
        printf("\n");

        encoder.reset();
        while (next_solution(spec, c, solver, encoder) == success) {
            printf("Next solution: ");
            c.to_expression(std::cout);
            printf("\n");

            assert(c.satisfies_spec(spec));
        }
    }

    // Test generating solutions starting from a specified number of steps.
    spec.initial_steps = 3;
    for (int i = 0; i < 256; i++) {
        kitty::create_from_words(tt3, &i, &i + 1);
        spec[0] = tt3;

        printf("Generating solutions of size >= 3 for function ");
        kitty::print_binary(tt3);
        printf("\n");

        encoder.reset();
        while (next_solution(spec, c, solver, encoder) == success) {
            printf("Next solution: ");
            c.to_expression(std::cout);
            printf("\n");

            if (!is_trivial(tt3)) {
                assert(c.get_nr_steps() >= 3);
            }
            assert(c.satisfies_spec(spec));
        }
    }

    chain c3;
    spec.fanin = 3;
    static_truth_table<4> tt4;
    spec.initial_steps = 1;
    spec.verbosity = 0;
    for (int i = 0; i < 256; i++) {
        kitty::create_from_words(tt4, &i, &i + 1);
        spec[0] = tt4;

        printf("Generating solutions for function ");
        kitty::print_binary(tt4);
        printf("\n");

        encoder.reset();
        while (next_solution(spec, c, solver, encoder) == success) {
            assert(c.satisfies_spec(spec));
        }
    }

    return 0;
}

