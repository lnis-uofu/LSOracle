#include <percy/percy.hpp>
#include <cstdio>
#include <fstream>
#include <ctime>
#include <thread>

using namespace percy;

void
runtime_comparison(int nr_vars, int nr_vertices)
{
    printf("Comparing sequential and parallel enumeration\n");
    rec_dag_generator rec_gen;
    rec_gen.reset(nr_vars, nr_vertices);
    auto seq_start = std::chrono::high_resolution_clock::now();
    auto seq_nsols = rec_gen.count_dags();
    auto seq_stop = std::chrono::high_resolution_clock::now();

    auto num_cpus = std::thread::hardware_concurrency();
    auto par_start = std::chrono::high_resolution_clock::now();
    auto par_nsols = parallel_dag_count(nr_vars, nr_vertices, num_cpus);
    auto par_stop = std::chrono::high_resolution_clock::now();

    printf("Enumeration time elapsed: %fms (SEQ)\n", 
            std::chrono::duration<double,std::milli>(
                seq_stop-seq_start).count());
    printf("Enumeration time elapsed: %fms (PAR)\n", 
            std::chrono::duration<double,std::milli>(
                par_stop-par_start).count());
    assert(seq_nsols == par_nsols);
    printf("\n");

    printf("Comparing sequential and parallel generation\n");
    rec_gen;
    rec_gen.reset(nr_vars, nr_vertices);
    size_t nr_seq_dags = 0;
    {
        seq_start = std::chrono::high_resolution_clock::now();
        auto seq_dags = rec_gen.gen_dags();
        seq_stop = std::chrono::high_resolution_clock::now();
        nr_seq_dags = seq_dags.size();
    }

    size_t nr_par_dags = 0;
    {
        par_start = std::chrono::high_resolution_clock::now();
        auto par_dags = parallel_dag_gen(nr_vars, nr_vertices, num_cpus);
        par_stop = std::chrono::high_resolution_clock::now();
        nr_par_dags = par_dags.size();
    }

    printf("Generation time elapsed: %fms (SEQ)\n", 
            std::chrono::duration<double,std::milli>(
                seq_stop-seq_start).count());
    printf("Generation time elapsed: %fms (PAR)\n", 
            std::chrono::duration<double,std::milli>(
                par_stop-par_start).count());
    assert(nr_seq_dags == nr_par_dags);
    printf("\n");
}

int main(void)
{
// TODO: fix Travis errors.
#ifndef TRAVIS_BUILD
    runtime_comparison(3, 8);
    runtime_comparison(3, 9);
    
    auto num_cpus = std::thread::hardware_concurrency();
    if (num_cpus >= 10) {
        runtime_comparison(5, 6);
        runtime_comparison(5, 7);
        runtime_comparison(5, 8);
    }
#endif

    return 0;
}

