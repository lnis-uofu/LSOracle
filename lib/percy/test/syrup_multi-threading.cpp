#include <cstdio>
#include <percy/percy.hpp>

#define MAX_TESTS 256

using namespace percy;

template<int nr_in>
void check_std_equivalence()
{
    spec spec;
    chain c2;
    using truth_table = kitty::static_truth_table<nr_in>;
    kitty::dynamic_truth_table map(truth_table::NumBits);
    std::transform(map.cbegin(), map.cend(), map.begin(), [](auto word) { return ~word; });
    std::unordered_set<truth_table, kitty::hash<truth_table>> classes;
    int64_t index = 0;
    truth_table tt;

    while (index != -1) {
        kitty::create_from_words(tt, &index, &index + 1);
        const auto res = kitty::exact_npn_canonization(tt, [&map](const auto& tt) { kitty::clear_bit(map, *tt.cbegin()); });
        classes.insert(std::get<0>(res));
        index = find_first_one_bit(map);
    } 

    glucose_wrapper solver2;
    ssv_encoder encoder2(solver2);

    std::vector<int> nrs_threads = { 2 };

    int ctr = 0;
    int64_t total_elapsed = 0;
    for (const auto& tt : classes) {
        spec[0] = tt;


        auto start = std::chrono::steady_clock::now();
        const auto res2 = synthesize(spec, c2, solver2, encoder2);
        const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - start
                ).count();
        assert(res2 == success);
        assert(c2.satisfies_spec(spec));

        total_elapsed += elapsed;

        printf("%d\r", ++ctr);
        fflush(stdout);
    }
    printf("\"true time\"=%zums\n", total_elapsed);
    printf("\n");

    for (auto nr_threads : nrs_threads) {
        int ctr = 0;
        int64_t total_elapsed = 0;
        solver2.set_nr_threads(nr_threads);
        for (const auto& tt : classes) {
            spec[0] = tt;
            

            auto start = std::chrono::steady_clock::now();
            const auto res2 = synthesize(spec, c2, solver2, encoder2);
            const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - start
                    ).count();
            assert(res2 == success);
            assert(c2.satisfies_spec(spec));

            total_elapsed += elapsed;

            printf("%d\r", ++ctr);
            fflush(stdout);
        }
        printf("\n");
        printf("nthreads=%d, elapsed time=%zd\n", nr_threads, total_elapsed);
    }
}

int main()
{
#ifndef TRAVIS_BUILD
#ifdef USE_SYRUP
    check_std_equivalence<3>();
    check_std_equivalence<4>();
#endif
#endif
    return 0;
}

