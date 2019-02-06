#include <percy/percy.hpp>

using namespace percy;

int
main(void)
{
    partial_dag3_generator gen;
    gen.gen_type(GEN_TUPLES);

    for (int i = 1; i <= 8; i++) {
        uint64_t expected_dags = 1;
        for (int j = 1; j < i; j++) {
            expected_dags *= (1 + j + (j >= 2 ? binomial_coeff(j, 2) : 0) + (j >= 3 ? binomial_coeff(j, 3) : 0));
        }
        //printf("Expecting %lu dags with %d vertices\n", expected_dags, i);
        gen.reset(i);
        auto nr_dags = gen.count_dags();
        printf("Found %lu dags with %d vertices\n", nr_dags, i);
        assert(nr_dags == expected_dags);
    }

    gen.gen_type(GEN_CONNECTED);
    for (int i = 1; i <= 8; i++) {
        gen.reset(i);
        auto nr_dags = gen.count_dags();
        printf("Found %lu connected dags with %d vertices\n", nr_dags, i);
    }

    gen.gen_type(GEN_COLEX);
    for (int i = 1; i <= 8; i++) {
        gen.reset(i);
        auto nr_dags = gen.count_dags();
        printf("Found %lu colex dags with %d vertices\n", nr_dags, i);
    }

    gen.gen_type(GEN_NOREAPPLY);
    //gen.verbosity(1);
    for (int i = 1; i <= 10; i++) {
        gen.reset(i);
        auto nr_dags = gen.count_dags();
        printf("Found %lu noreapply dags with %d vertices\n", nr_dags, i);
    }

    return 0;
}
