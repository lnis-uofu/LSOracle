#include <percy/percy.hpp>
#include <percy/io.hpp>

#define MAX_TESTS 256

using namespace percy;
using kitty::dynamic_truth_table;

#if 0
void gen_check_equivalence(int nr_in, bool full_coverage)
{
    dag<2> g;
    unbounded_dag_generator<sat_solver*> ugen;

    auto synth1 = new_std_synth();
    auto synth2 = new_dag_synth();

    spec spec;
    spec.verbosity = 0;


    // don't run too many tests.
    auto max_tests = (1 << (1 << nr_in));
    if (!full_coverage) {
        max_tests = std::min(max_tests, MAX_TESTS);
    }
    dynamic_truth_table tt(nr_in);

    chain c1;
    chain c2;

    for (auto i = 1; i < max_tests; i++) {
        kitty::create_from_words(tt, &i, &i+1);

        // We skip the trivial functions
        if (is_trivial(tt)) {
            continue;
        }
        spec[0] = tt;
        auto res1 = synth1->synthesize(spec, c1);
        assert(res1 == success);
        auto sim_tts1 = chain_simulate(c1, spec);
        auto c1_nr_steps = c1.get_nr_steps();

        const auto dag_found = find_dag(spec, g, nr_in);
        assert(dag_found == success);
        auto result = synth2->synthesize(spec, g, c2);
        assert(result == success);
        auto c2_nr_steps = c2.get_nr_steps();
        auto sim_tts2 = chain_simulate(c2, spec);
        assert(c1_nr_steps == c2_nr_steps);
        assert(sim_tts1[0] == sim_tts2[0]);

        printf("(%d/%d)\r", i + 1, max_tests);
        fflush(stdout);
    }
    printf("\n");
}

void check_equivalence(int nr_in, bool full_coverage)
{
    dag<2> g;
    unbounded_dag_generator<sat_solver*> ugen;

    auto synth1 = new_std_synth();
    auto synth2 = new_dag_synth();
       
    spec spec;

    spec.verbosity = 0;

    // don't run too many tests.
    auto max_tests = (1 << (1 << nr_in));
    if (!full_coverage) {
        max_tests = std::min(max_tests, MAX_TESTS);
    }
    dynamic_truth_table tt(nr_in);

    chain c1;
    chain c2;

    for (auto i = 1; i < max_tests; i++) {
        kitty::create_from_words(tt, &i, &i+1);

        // We skip the trivial functions
        if (is_trivial(tt)) {
            continue;
        }
        spec[0] = tt;

        auto res1 = synth1->synthesize(spec, c1);
        assert(res1 == success);
        auto sim_tts1 = chain_simulate(c1, spec);
        auto c1_nr_steps = c1.get_nr_steps();

        ugen.reset(nr_in);
        int min_size = -1;
        while (ugen.next_dag(g)) {
            if (min_size != -1 && g.get_nr_vertices() > min_size) {
                break;
            }
            auto result = synth2->synthesize(spec, g, c2);
            if (result == success) {
                auto c2_nr_steps = c2.get_nr_steps();
                if (min_size == -1) {
                    min_size = c2_nr_steps;
                }
                auto sim_tts2 = chain_simulate(c2, spec);
                assert(c1_nr_steps == c2_nr_steps);
                assert(sim_tts1[0] == sim_tts2[0]);
            } else {
                assert(result == failure);
            }
        }
        printf("(%d/%d)\r", i + 1, max_tests);
        fflush(stdout);
    }
    printf("\n");
}
/*
template<int nr_in>
auto
get_npn_classes()
{
    std::unordered_set<static_truth_table<nr_in>, kitty::hash<static_truth_table<nr_in>>> classes;
    static_truth_table<1 << nr_in> map;
    std::transform(map.cbegin(), map.cend(), map.begin(), 
            []( auto w ) { return ~w; } );

    int64_t index = 0;
    static_truth_table<nr_in> tt;
    while (index != -1) {
        kitty::create_from_words(tt, &index, &index + 1);
        const auto res = kitty::exact_npn_canonization(
                tt, [&map]( const auto& tt ) { 
                    kitty::clear_bit( map, *tt.cbegin() ); 
                } 
            );
        classes.insert( std::get<0>( res ) );
        index = find_first_one_bit( map );
    }

    printf("[i] enumerated %lu functions into %lu classes\n",
            map.num_bits(), classes.size());

    return classes;
}
*/

auto
get_npn_classes(int nr_in)
{
    std::unordered_set<dynamic_truth_table, kitty::hash<dynamic_truth_table>> classes;
    dynamic_truth_table map(1 << nr_in);
    std::transform(map.cbegin(), map.cend(), map.begin(), 
            []( auto w ) { return ~w; } );

    int64_t index = 0;
    dynamic_truth_table tt(nr_in);
    while (index != -1) {
        kitty::create_from_words(tt, &index, &index + 1);
        const auto res = kitty::exact_npn_canonization(
                tt, [&map]( const auto& tt ) { 
                    kitty::clear_bit( map, *tt.cbegin() ); 
                } 
            );
        classes.insert( std::get<0>( res ) );
        index = find_first_one_bit( map );
    }

    printf("[i] enumerated %lu functions into %lu classes\n",
            map.num_bits(), classes.size());

    return classes;
}

void check_npn_equivalence(int nr_in)
{
    auto npn_set = get_npn_classes(nr_in);

    dag<2> g;
    unbounded_dag_generator<sat_solver*> ugen;

    auto synth1 = new_std_synth();
    auto synth2 = new_dag_synth();

    spec spec;
    spec.verbosity = 0;

    chain c1;
    chain c2;

    int i = 0;
    for (auto& npn_tt : npn_set) {
        auto tt = npn_tt;

        // We skip the trivial functions
        if (is_trivial(tt)) {
            continue;
        }
        auto support = min_base_inplace(tt);
        if (support.size() < nr_in) {
            continue;
        }
        expand_inplace(tt, support);

        spec.functions[0] = tt;
        auto res1 = synth1->synthesize(spec, c1);
        assert(res1 == success);
        auto sim_tts1 = chain_simulate(c1, spec);
        auto c1_nr_steps = c1.get_nr_steps();

        ugen.reset(nr_in);
        int min_size = -1;
        while (ugen.next_dag(g)) {
            if (min_size != -1 && g.get_nr_vertices() > min_size) {
                break;
            }
            auto result = synth2->synthesize(spec, g, c2);
            if (result == success) {
                auto c2_nr_steps = c2.get_nr_steps();
                if (min_size == -1) {
                    min_size = c2_nr_steps;
                }
                auto sim_tts2 = chain_simulate(c2, spec);
                assert(c1_nr_steps == c2_nr_steps);
                assert(sim_tts1[0] == sim_tts2[0]);
            }
        }
        const auto dag_found = find_dag(spec, g, nr_in);
        assert(dag_found == success);
        auto result = synth2->synthesize(spec, g, c2);
        assert(result == success);
        auto c2_nr_steps = c2.get_nr_steps();
        auto sim_tts2 = chain_simulate(c2, spec);
        assert(c1_nr_steps == c2_nr_steps);
        assert(sim_tts1[0] == sim_tts2[0]);
    }
}

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

    check_equivalence(2, full_coverage);
    check_equivalence(3, full_coverage);
    
    gen_check_equivalence(2, full_coverage);
    gen_check_equivalence(3, full_coverage);

    if (full_coverage) {
        check_npn_equivalence(4);
    }

    return 0;
}
#endif
int main() { return 0; }