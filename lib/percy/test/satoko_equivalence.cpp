#include <cstdio>
#include <percy/percy.hpp>
#include <unordered_set>
#include <ctime>

#define MAX_TESTS 256

using namespace percy;

/*******************************************************************************
    Verifies that our synthesizers' results are equivalent to each other.
*******************************************************************************/
template<int nr_in>
void check_std_equivalence()
{
#ifndef DISABLE_SATOKO

    bsat_wrapper bsat_solver;
    ssv_encoder bsat_enc(bsat_solver);
    satoko_wrapper satoko_solver;
    ssv_encoder satoko_enc(satoko_solver);

    percy::spec spec;
    spec.verbosity = 0;

    chain c1, c1_cegar, c2, c2_cegar;

    using truth_table = kitty::static_truth_table<nr_in>;
    std::unordered_set<truth_table, kitty::hash<truth_table>> classes;
    truth_table repr;
    do
    {
        const auto res = kitty::exact_npn_canonization(repr);
        classes.insert(std::get<0>(res));
        kitty::next_inplace(repr);
    } while (!kitty::is_const0(repr));

    const auto nr_classes = classes.size();
    printf("Testing on %zu NPN classes\n", nr_classes);
    auto ctr = 0;

    auto synth_time1 = 0;
    auto synth_time1_cegar = 0;
    auto synth_time2 = 0;
    auto synth_time2_cegar = 0;

    for (const auto& tt : classes) {
        spec[0] = tt;
        auto start = std::clock();
        auto res1 = synthesize(spec, c1, bsat_solver, bsat_enc);
        const auto elapsed1 = std::clock() - start;
        assert(res1 == success);
        auto sim_tts1 = c1.simulate();
        auto c1_nr_steps = c1.get_nr_steps();

        start = std::clock();
        auto res1_cegar = synthesize(spec, c1_cegar, bsat_solver, bsat_enc, SYNTH_STD_CEGAR);
        const auto elapsed1_cegar = std::clock() - start;
        assert(res1_cegar == success);
        auto sim_tts1_cegar = c1_cegar.simulate();
        auto c1_cegar_nr_steps = c1_cegar.get_nr_steps();

        start = std::clock();
        auto res2 = synthesize(spec, c2, satoko_solver, satoko_enc);
        const auto elapsed2 = std::clock() - start;
        assert(res2 == success);
        auto sim_tts2 = c2.simulate();
        auto c2_nr_steps = c2.get_nr_steps();

        start = std::clock();
        auto res2_cegar = synthesize(spec, c2_cegar, satoko_solver, satoko_enc, SYNTH_STD_CEGAR);
        const auto elapsed2_cegar = std::clock() - start;
        assert(res2_cegar == success);
        auto sim_tts2_cegar = c2_cegar.simulate();
        auto c2_cegar_nr_steps = c2_cegar.get_nr_steps();

        assert(c1_nr_steps == c2_nr_steps);
        assert(c1_nr_steps == c1_cegar_nr_steps);
        assert(c1_cegar_nr_steps == c2_cegar_nr_steps);
        assert(sim_tts1[0] == sim_tts2[0]);
        assert(sim_tts1[0] == sim_tts1_cegar[0]);
        assert(sim_tts1_cegar[0] == sim_tts2_cegar[0]);
        assert(c1.satisfies_spec(spec));
        assert(c2.satisfies_spec(spec));
        
        printf("(%d/%zu)\r", ++ctr, nr_classes);
        fflush(stdout);

        synth_time1 += elapsed1;
        synth_time1_cegar += elapsed1_cegar;
        synth_time2 += elapsed2;
        synth_time2_cegar += elapsed2_cegar;
    }
    printf("\n");
    printf("Total bsat2 time: %.2f ms\n", (1000.0 * synth_time1) / CLOCKS_PER_SEC);
    printf("Total bsat2 time (CEGAR): %.2f ms\n", (1000.0 * synth_time1_cegar) / CLOCKS_PER_SEC);
    printf("Total satoko time: %.2f ms\n", (1000.0 * synth_time2) / CLOCKS_PER_SEC);
    printf("Total satoko time (CEGAR): %.2f ms\n", (1000.0 * synth_time2_cegar) / CLOCKS_PER_SEC);
#endif // DISABLE_SATOKO
}

/*******************************************************************************
    By default, does not check for full equivalence of all n-input functions.
    Users can specify a arbitrary runtime argument, which removes the limit on
    the number of equivalence tests.
*******************************************************************************/
int main(int argc, char **)
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
    
    check_std_equivalence<2>();
    check_std_equivalence<3>();
    check_std_equivalence<4>();

    //printf("Press Enter to continue...\n");
    //std::cin.ignore();

    return 0;
}

