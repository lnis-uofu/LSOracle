#include <percy/percy.hpp>

#define MAX_TESTS 512ul

using namespace percy;
using kitty::static_truth_table;

/*
template<int nr_in>
void check_equivalence(bool full_coverage)
{
    printf("Checking synthesis equivalence for %d-input functions\n", nr_in);

    dag<2> g;
    vector<int> perm(nr_in);
    static_truth_table<nr_in> tt;
    chain<2> c1;
    chain<2> c2;
    unbounded_dag_generator<sat_solver*> ugen;
    nonisomorphic_dag_generator<sat_solver*> igen;
    dag_synthesizer<> synth;

    synth_spec<static_truth_table<nr_in> spec(nr_in, 1);

    // don't run too many tests.
    auto max_tests = (1ul << (1ul << nr_in));
    if (!full_coverage) {
        max_tests = std::min(max_tests, MAX_TESTS);
    }
    for (auto i = 1ul; i < max_tests; i++) {
        printf("i = %lu\n", i);
        kitty::create_from_words(tt, &i, &i+1);

        // We skip the trivial functions
        if (is_trivial(tt)) {
            continue;
        }

        ugen.reset(nr_in);
        while (ugen.next_dag(g)) {
            auto result = synth.synthesize(tt, g, c1);
            if (result == success) {
                break;
            }
        }

        igen.reset(nr_in);
        while (igen.next_dag(g)) {
            assert(g.get_nr_vertices() <= c1.nr_steps());
            synth.reset(nr_in, g.get_nr_vertices());
            auto result = synth.perm_synthesize(tt, g, c2, perm);
            if (result == success) {
                break;
            }
        }

        auto c1_nr_steps = c1.nr_steps();
        auto c2_nr_steps = c2.nr_steps();
        auto sim_tts1 = c1.simulate();
        auto sim_tts2 = c2.simulate();
        assert(c1_nr_steps == c2_nr_steps);
        assert(*sim_tts1[0] == *sim_tts2[0]);
    } 
}
*/

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

/*
template<int nr_in>
void check_npn_equivalence()
{
    auto npn_set = get_npn_classes<nr_in>();

    dag<2> g;
    vector<int> perm(nr_in);
    chain<static_truth_table<nr_in>> c1;
    chain<static_truth_table<nr_in>> c2;
    unbounded_dag_generator<sat_solver*> ugen;
    nonisomorphic_dag_generator<sat_solver*> igen;
    dag_synthesizer<static_truth_table<nr_in>,sat_solver*> synth;

    int i = 0;
    for (auto& npn_tt : npn_set) {
        printf("i = %d\n", ++i);
        static_truth_table<nr_in> tt = npn_tt;

        // We skip the trivial functions
        if (is_trivial(tt)) {
            continue;
        }

        spec.functions[0] = &tt;
        
        ugen.reset(nr_in);
        while (ugen.next_dag(g)) {
            synth.reset(nr_in, g.get_nr_vertices());
            auto result = synth.synthesize(spec, g, c1);
            if (result == success) {
                break;
            }
        }

        igen.reset(nr_in);
        while (igen.next_dag(g)) {
            auto result = synth.perm_synthesize(spec, g, c2, perm);
            if (result == success) {
                break;
            }
        }

        auto c1_nr_steps = c1.nr_steps();
        auto c2_nr_steps = c2.nr_steps();
        auto sim_tts1 = c1.simulate();
        auto sim_tts2 = c2.simulate();
        assert(c1_nr_steps == c2_nr_steps);
        assert(sim_tts1[0] == sim_tts2[0]);
    }
}
*/

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

//    check_equivalence<2>(full_coverage);
//    check_equivalence<3>(full_coverage);

    if (full_coverage) {
 //       check_equivalence<5>(full_coverage);
//        check_npn_equivalence<4>();
    }

    return 0;
}

