#include <cstdio>
#include <random>
#include <percy/percy.hpp>
#include <benchmark/benchmark.h>
#include <kitty/kitty.hpp>
#include <unordered_set>

extern "C"
{
#include <base/abc/abc.h>
void Abc_Start();
void Abc_Stop();
}

#define NR_IN 4
#define MAX_TESTS 15

using namespace percy;
using truth_table = kitty::static_truth_table<NR_IN>;
using npn_set = std::unordered_set<truth_table, kitty::hash<truth_table>>;

/*******************************************************************************
    Perform various microbenchmarks of synthesizer functionality.
*******************************************************************************/

class simple_synth_fixture : public benchmark::Fixture
{
    public:
        synth_spec<static_truth_table<NR_IN>> spec;
        chain<static_truth_table<NR_IN>> c;

        void SetUp(const benchmark::State &state)
        {
            spec.nr_out = 1;
            spec.nr_in = state.range(0);
            spec.verbosity = 0;
        }

};

BENCHMARK_DEFINE_F(simple_synth_fixture, chain_test)(benchmark::State& state)
{
    static_truth_table<NR_IN> tt;
    spec.functions[0] = &tt;
                    
    auto synth = new_synth<static_truth_table<NR_IN>,sat_solver*>(SIMPLE);

    for (auto _ : state) {
        auto word = state.range(1);
        kitty::create_from_words(tt, &word, &word+1);
        synth->synthesize(spec, c);
    }
};

class symmetric_synth_fixture : public benchmark::Fixture
{
    public:
        synth_spec<static_truth_table<NR_IN>> spec;
        chain<static_truth_table<NR_IN>> c;

        void SetUp(const benchmark::State &state)
        {
            spec.nr_out = 1;
            spec.nr_in = state.range(0);
            spec.verbosity = 0;
        }
};

BENCHMARK_DEFINE_F(symmetric_synth_fixture, chain_test)(benchmark::State& state)
{
    static_truth_table<NR_IN> tt;
    spec.functions[0] = &tt;
            
    auto synth = new_synth<static_truth_table<NR_IN>,sat_solver*>(SYMMETRIC);

    for (auto _ : state) {
        auto word = state.range(1);
        kitty::create_from_words(tt, &word, &word+1);
        synth->synthesize(spec, c);
    }
};

BENCHMARK_DEFINE_F(symmetric_synth_fixture, cegar_test)(benchmark::State& state)
{
    static_truth_table<NR_IN> tt;
    spec.functions[0] = &tt;
            
    auto synth = new_synth<static_truth_table<NR_IN>,sat_solver*>(SYMMETRIC);

    for (auto _ : state) {
        auto word = state.range(1);
        kitty::create_from_words(tt, &word, &word+1);
        synth->cegar_synthesize(spec, c);
    }
};

npn_set
get_npn_classes(void)
{
    npn_set classes;
    kitty::dynamic_truth_table map( truth_table::NumBits );
    std::transform( map.cbegin(), map.cend(), map.begin(), []( auto w ) { return ~w; } );

    int64_t index = 0;
    truth_table tt;
    while (index != -1) {
        kitty::create_from_words(tt, &index, &index + 1);
        const auto res = kitty::exact_npn_canonization( tt, [&map]( const auto& tt ) { kitty::clear_bit( map, *tt.cbegin() ); } );
        classes.insert( std::get<0>( res ) );
        index = find_first_one_bit( map );
    }

    printf("[i] enumerated %lu functions into %lu classes\n",
            map.num_bits(), classes.size());

    return classes;
}

static void custom_arguments(benchmark::internal::Benchmark* b)
{
    auto classes = get_npn_classes();

    static_truth_table<NR_IN> tt;
    auto counter = 0;
    for (const auto& c : classes) {
        tt = c;
        int word = 0;
        int nrwords = 0;
        for (auto it = tt.begin(); it != tt.end(); it++) {
            word = *it;
            nrwords++;
        }
        assert(nrwords == 1);
        b->Args({NR_IN, word});
        if (++counter >= MAX_TESTS) {
            break;
        }
    }
}

BENCHMARK_REGISTER_F(simple_synth_fixture, chain_test)->Apply(custom_arguments);
BENCHMARK_REGISTER_F(symmetric_synth_fixture, cegar_test)->Apply(custom_arguments);
BENCHMARK_MAIN();

