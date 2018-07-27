#include <cstdio>
#include <percy/percy.hpp>
#include <kitty/kitty.hpp>

#define MAX_TESTS 256

using namespace percy;
using kitty::dynamic_truth_table;

/*******************************************************************************
    Verifies that our synthesizers' results are equivalent to each other.
*******************************************************************************/
void check_std_equivalence(int nr_in, bool full_coverage)
{
#ifdef USE_CMS

    bsat_wrapper bsat_solver;
    knuth_encoder bsat_enc(bsat_solver);
    cmsat_wrapper cmsat_solver;
    knuth_encoder cmsat_enc(cmsat_solver);

    spec spec;
    spec.verbosity = 0;

    chain c1, c1_cegar, c2, c2_cegar;

    // Don't run too many tests.
    auto max_tests = (1 << (1 << nr_in));
    if (!full_coverage) {
        max_tests = std::min(max_tests, MAX_TESTS);
    }
    dynamic_truth_table tt(nr_in);
    for (auto i = 1; i < max_tests; i++) {
        kitty::create_from_words(tt, &i, &i+1);

        spec[0] = tt;
        auto res1 = synthesize(spec, c1, bsat_solver, bsat_enc);
        assert(res1 == success);
        auto sim_tts1 = c1.simulate(spec);
        auto c1_nr_steps = c1.get_nr_steps();

        auto res1_cegar = synthesize(spec, c1_cegar, bsat_solver, bsat_enc, SYNTH_STD_CEGAR);
        assert(res1_cegar == success);
        auto sim_tts1_cegar = c1_cegar.simulate(spec);
        auto c1_cegar_nr_steps = c1_cegar.get_nr_steps();

        auto res2 = synthesize(spec, c2, cmsat_solver, cmsat_enc);
        assert(res2 == success);
        auto sim_tts2 = c2.simulate(spec);
        auto c2_nr_steps = c2.get_nr_steps();

        auto res2_cegar = synthesize(spec, c2_cegar, cmsat_solver, cmsat_enc, SYNTH_STD_CEGAR);
        assert(res2_cegar == success);
        auto sim_tts2_cegar = c2_cegar.simulate(spec);
        auto c2_cegar_nr_steps = c2_cegar.get_nr_steps();

        assert(c1_nr_steps == c2_nr_steps);
        assert(c1_nr_steps == c1_cegar_nr_steps);
        assert(c1_cegar_nr_steps == c2_cegar_nr_steps);
        assert(sim_tts1[0] == sim_tts2[0]);
        assert(sim_tts1[0] == sim_tts1_cegar[0]);
        assert(sim_tts1_cegar[0] == sim_tts2_cegar[0]);
        assert(c1.satisfies_spec(spec));
        assert(c2.satisfies_spec(spec));
        
        printf("(%d/%d)\r", i+1, max_tests);
        fflush(stdout);
    }
    printf("\n");
#endif
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
    
    check_std_equivalence(2, full_coverage);
    check_std_equivalence(3, full_coverage);
    check_std_equivalence(4, full_coverage);

    return 0;
}

