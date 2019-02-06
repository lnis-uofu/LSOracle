#include <cstdio>
#include <percy/percy.hpp>
#include <chrono>

#define MAX_TESTS 256

using namespace percy;
using kitty::dynamic_truth_table;


int main()
{
    {
        const auto dags = pd3_generate_filtered(1, 3);

        // Synthesize MAJ-3
        mig mig;
        spec spec;
        bsat_wrapper solver;
        maj_encoder encoder(solver);
        kitty::dynamic_truth_table tt(3);
        kitty::create_majority(tt);
        spec[0] = tt;

        auto start = std::chrono::steady_clock::now();
        auto res = maj_synthesize(spec, mig, solver, encoder);
        const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now() - start
            ).count();
        assert(res == success);
        assert(mig.satisfies_spec(spec));
        assert(mig.get_nr_steps() == 1);

        start = std::chrono::steady_clock::now();
        res = maj_cegar_synthesize(spec, mig, solver, encoder);
        const auto elapsed_cegar = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now() - start
            ).count();
        assert(res == success);
        assert(mig.satisfies_spec(spec));
        assert(mig.get_nr_steps() == 1);

        start = std::chrono::steady_clock::now();
        res = maj_fence_synthesize(spec, mig, solver, encoder);
        const auto elapsed_fence = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now() - start
            ).count();
        assert(res == success);
        assert(mig.satisfies_spec(spec));
        assert(mig.get_nr_steps() == 1);

        start = std::chrono::steady_clock::now();
        res = maj_fence_cegar_synthesize(spec, mig, solver, encoder);
        const auto elapsed_fence_cegar = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now() - start
            ).count();
        assert(res == success);
        assert(mig.satisfies_spec(spec));
        assert(mig.get_nr_steps() == 1);
        
        start = std::chrono::steady_clock::now();
        res = maj_pd_synthesize(spec, mig, dags, solver, encoder);
        const auto elapsed_pd = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now() - start
            ).count();
        assert(res == success);
        assert(mig.satisfies_spec(spec));
        assert(mig.get_nr_steps() == 1);

        printf("MAJ-3 time elapsed (MAJ): %ldus\n", elapsed);
        printf("MAJ-3 time elapsed (MAJ CEGAR): %ldus\n", elapsed_cegar);
        printf("MAJ-3 time elapsed (MAJ FENCE): %ldus\n", elapsed_fence);
        printf("MAJ-3 time elapsed (MAJ FENCE CEGAR): %ldus\n", elapsed_fence_cegar);
        printf("MAJ-3 time elapsed (MAJ PD): %ldus\n", elapsed_pd);
    }
    {
        const auto dags = pd3_generate_filtered(4, 5);

        // Exact synthesis of MAJ-5
        mig mig;
        spec spec;
        bsat_wrapper solver;
        maj_encoder encoder(solver);
        kitty::dynamic_truth_table tt(5);
        kitty::create_majority(tt);
        spec[0] = tt;

        auto start = std::chrono::steady_clock::now();
        auto res = maj_synthesize(spec, mig, solver, encoder);
        const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now() - start
            ).count();
        assert(res == success);
        assert(mig.satisfies_spec(spec));
        assert(mig.get_nr_steps() == 4);

        start = std::chrono::steady_clock::now();
        res = maj_cegar_synthesize(spec, mig, solver, encoder);
        const auto elapsed_cegar = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now() - start
            ).count();
        assert(res == success);
        assert(mig.satisfies_spec(spec));
        assert(mig.get_nr_steps() == 4);

        start = std::chrono::steady_clock::now();
        res = maj_fence_synthesize(spec, mig, solver, encoder);
        const auto fence_elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now() - start
            ).count();
        assert(res == success);
        assert(mig.satisfies_spec(spec));
        assert(mig.get_nr_steps() == 4);

        start = std::chrono::steady_clock::now();
        res = maj_fence_cegar_synthesize(spec, mig, solver, encoder);
        const auto fence_cegar_elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now() - start
            ).count();
        assert(res == success);
        assert(mig.satisfies_spec(spec));
        assert(mig.get_nr_steps() == 4);

        start = std::chrono::steady_clock::now();
        res = parallel_maj_synthesize(spec, mig);
        const auto parr_elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now() - start
            ).count();
        assert(res == success);
        assert(mig.satisfies_spec(spec));
        assert(mig.get_nr_steps() == 4);

        spec.verbosity = 0;
        start = std::chrono::steady_clock::now();
        res = maj_pd_synthesize(spec, mig, dags, solver, encoder);
        const auto pd_elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now() - start
            ).count();
        assert(res == success);
        assert(mig.satisfies_spec(spec));
        assert(mig.get_nr_steps() == 4);

        printf("MAJ-5 time elapsed (MAJ): %ldus\n", elapsed);
        printf("MAJ-5 time elapsed (MAJ CEGAR): %ldus\n", elapsed_cegar);
        printf("MAJ-5 time elapsed (MAJ FENCE): %ldus\n", fence_elapsed);
        printf("MAJ-5 time elapsed (MAJ FENCE CEGAR): %ldus\n", fence_cegar_elapsed);
        printf("MAJ-5 time elapsed (MAJ PARR): %ldus\n", parr_elapsed);
        printf("MAJ-5 time elapsed (MAJ PD): %ldus\n", pd_elapsed);
    }

    {
        const auto dags = pd3_generate_filtered(7, 7);

        // Exact synthesis of MAJ-7
        mig mig;
        spec spec;
        bmcg_wrapper solver;
        maj_encoder encoder(solver);
        kitty::dynamic_truth_table tt(7);
        kitty::create_majority(tt);
        spec[0] = tt;
        spec.initial_steps = 7;

        auto start = std::chrono::steady_clock::now();
        auto res = parallel_maj_synthesize(spec, mig);
        const auto parr_elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now() - start
            ).count();
        assert(res == success);
        assert(mig.satisfies_spec(spec));
        assert(mig.get_nr_steps() == 7);

        start = std::chrono::steady_clock::now();
        res = parallel_nocegar_maj_synthesize(spec, mig);
        const auto parr_nocegar_elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now() - start
            ).count();
        assert(res == success);
        assert(mig.satisfies_spec(spec));
        assert(mig.get_nr_steps() == 7);

        printf("MAJ-7 time elapsed (MAJ PARR): %ldus\n", parr_elapsed);
        printf("MAJ-7 time elapsed (MAJ PARR NOCEGAR): %ldus\n", parr_nocegar_elapsed);

        /*
        start = std::chrono::steady_clock::now();
        res = maj_synthesize(spec, mig, solver, encoder);
        const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now() - start
            ).count();
        assert(res == success);
        assert(mig.satisfies_spec(spec));
        assert(mig.get_nr_steps() == 7);

        
        printf("MAJ-7 time elapsed (MAJ): %ldus\n", elapsed);
        */


        spec.add_alonce_clauses = false;
        spec.add_colex_clauses = false;
        spec.add_noreapply_clauses = false;
        spec.add_symvar_clauses = false;
        start = std::chrono::steady_clock::now();
        res = maj_cegar_synthesize(spec, mig, solver, encoder);
        const auto elapsed_cegar = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now() - start
            ).count();
        assert(res == success);
        assert(mig.satisfies_spec(spec));
        assert(mig.get_nr_steps() == 7);
        printf("MAJ-7 time elapsed (MAJ CEGAR): %ldus\n", elapsed_cegar);

        /*
        start = std::chrono::steady_clock::now();
        res = maj_pd_synthesize(spec, mig, dags, solver, encoder);
        const auto pd_elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now() - start
            ).count();
        assert(res == success);
        assert(mig.satisfies_spec(spec));
        assert(mig.get_nr_steps() == 7);
        mig.to_expression(std::cout, true);
        
        printf("MAJ-7 time elapsed (MAJ PD): %ldus\n", pd_elapsed);

        
        start = std::chrono::steady_clock::now();
        res = maj_fence_synthesize(spec, mig, solver, encoder);
        const auto fence_elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now() - start
            ).count();
        assert(res == success);
        assert(mig.satisfies_spec(spec));
        assert(mig.get_nr_steps() == 7);
        printf("MAJ-7 time elapsed (MAJ FENCE): %ldus\n", fence_elapsed);

        start = std::chrono::steady_clock::now();
        res = maj_fence_cegar_synthesize(spec, mig, solver, encoder);
        const auto fence_cegar_elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now() - start
            ).count();
        assert(res == success);
        assert(mig.satisfies_spec(spec));
        assert(mig.get_nr_steps() == 7);
        printf("MAJ-7 time elapsed (MAJ FENCE CEGAR): %ldus\n", fence_cegar_elapsed);
        */
    }
    
    return 0;
}

