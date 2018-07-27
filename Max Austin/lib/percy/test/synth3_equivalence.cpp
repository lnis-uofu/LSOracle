#include <cstdio>
#include <percy/percy.hpp>
#include <kitty/kitty.hpp>

#define MAX_TESTS 256

using namespace percy;
using kitty::static_truth_table;
using kitty::dynamic_truth_table;

/// Verifies that our synthesizing chains with different fanins results in chains
/// that compute equivalent functions.
void check_equivalence(int nr_in, bool full_coverage)
{
    spec spec;
    spec.verbosity = 0;

    // don't run too many tests.
    auto max_tests = (1 << (1 << nr_in));
    if (!full_coverage) {
        max_tests = std::min(max_tests, MAX_TESTS);
    }
    dynamic_truth_table tt(nr_in);

    chain c1, c1_cegar;
    chain c2, c2_cegar;
    chain c3, c3_cegar;

    bsat_wrapper solver;
    knuth_encoder encoder(solver);

    for (auto i = 1; i < max_tests; i++) {
        kitty::create_from_words(tt, &i, &i+1);

        spec[0] = tt;
        auto res1 = synthesize(spec, c1);
        assert(res1 == success);
        auto sim_tts1 = c1.simulate(spec);

        auto res1_cegar = synthesize(spec, c1_cegar, solver, encoder, SYNTH_STD_CEGAR);
        assert(res1_cegar == success);
        auto sim_tts1_cegar = c1_cegar.simulate(spec);

        spec.fanin = 3;
        auto res2 = synthesize(spec, c2, solver, encoder);
        assert(res2 == success);
        auto sim_tts2 = c2.simulate(spec);
        auto c2_nr_vertices = c2.get_nr_steps();

        auto res2_cegar = synthesize(spec, c2_cegar, solver, encoder, SYNTH_STD_CEGAR);
        assert(res2_cegar == success);
        auto sim_tts2_cegar = c2_cegar.simulate(spec);
        auto c2_cegar_nr_vertices = c2.get_nr_steps();

        assert(c2_nr_vertices == c2_cegar_nr_vertices);
        assert(sim_tts1[0] == sim_tts2[0]);
        assert(sim_tts1[0] == sim_tts1_cegar[0]);
        assert(sim_tts1_cegar[0] == sim_tts2_cegar[0]);

        if (nr_in >= 4) {
            spec.fanin = 4;
            auto res3 = synthesize(spec, c3, solver, encoder);
            assert(res3 == success);
            auto sim_tts3 = c3.simulate(spec);
            auto c3_nr_vertices = c3.get_nr_steps();

            auto res3_cegar = synthesize(spec, c3_cegar, solver, encoder, SYNTH_STD_CEGAR);
            assert(res3_cegar == success);
            auto sim_tts3_cegar = c3_cegar.simulate(spec);
            auto c3_cegar_nr_vertices = c3.get_nr_steps();

            assert(c3_nr_vertices == c3_cegar_nr_vertices);
            assert(sim_tts3[0] == sim_tts2[0]);
            assert(sim_tts3_cegar[0] == sim_tts2[0]);
        }
        
        printf("(%d/%d)\r", i+1, max_tests);
        fflush(stdout);
    }
    printf("\n");
}


/*******************************************************************************
    By default, does not check for full equivalence of all n-input functions.
    Users can specify a arbitrary runtime argument, which removes the limit on
    the number of equivalence tests.
*******************************************************************************/
int main(int argc, char **argv)
{
    bool full_coverage = false;
    if (argc > 1) {
        full_coverage = true;
    }
    if (full_coverage) {
        printf("Doing full equivalence check\n");
    } else {
        printf("Doing partial equivalence check\n");
    }

    check_equivalence(3, full_coverage);
    check_equivalence(4, full_coverage);
    
    return 0;
}

