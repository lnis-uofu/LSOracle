#include <percy/percy.hpp>
#include <cassert>
#include <cstdio>
#include <fstream>

using namespace percy;
using kitty::dynamic_truth_table;

int main(void)
{
    {
        bmcg_wrapper solver;
        ditt_maj_encoder encoder(solver);
        percy::spec spec;
        dynamic_truth_table maj_tt(7);
        kitty::create_majority(maj_tt);
        spec[0] = maj_tt;
        chain c;
        spec.nr_steps = 7;
        spec.preprocess();

        const auto start = std::chrono::steady_clock::now();

        encoder.cegar_encode(spec);
        auto iMint = 0;
        for (int i = 0; iMint != -1; i++) {
            if (!encoder.add_cnf(spec, iMint)) {
                //printf("The problem has no solution\n");
                break;
            }
            printf( "Iter %3d : ", i);
            printf( "Var =%5d  ", encoder.get_nr_vars());
            printf( "Cla =%6d  ", solver.nr_clauses());
            printf( "Conf =%9d\n", solver.nr_conflicts());
            const auto status = solver.solve(0);
            if (status == failure) {
                //printf("The problem has no solution\n");
                break;
            }
            iMint = encoder.simulate(spec);
        }
        if (iMint == -1) {
            //printf("found solution!\n");
            encoder.extract_chain(spec, c);
            assert(c.simulate()[0] == maj_tt);
            assert(c.is_mag());
            assert(c.get_nr_steps() == 7);

            const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now() - start
            ).count();
            c.print_mag();
            printf("Time elapsed: %ldus (STD CEGAR)\n", elapsed);
        }
    }
    
    {
        spec spec;
        kitty::dynamic_truth_table tt(7);
        kitty::create_majority(tt);
        spec[0] = tt;
        spec.preprocess();
        bsat_wrapper solver;
        ditt_maj_encoder encoder(solver);
        fence f;
        f.reset(7, 4);
        f[0] = 2;
        f[1] = 2;
        f[2] = 2;
        f[3] = 1;
        spec.nr_steps = f.nr_nodes();
        
        const auto start = std::chrono::steady_clock::now();

        encoder.cegar_encode(spec, f);
        auto iMint = 0;
        for (int i = 0; iMint != -1; i++) {
            if (!encoder.add_cnf(spec, iMint)) {
                //printf("The problem has no solution\n");
                break;
            }
            //printf( "Iter %3d : ", i);
            //printf( "Var =%5d  ", encoder.get_nr_vars());
            //printf( "Cla =%6d  ", solver.nr_clauses());
            //printf( "Conf =%9d\n", solver.nr_conflicts());
            const auto status = solver.solve(0);
            if (status == failure) {
                //printf("The problem has no solution\n");
                break;
            }
            iMint = encoder.simulate(spec);
        }
        if (iMint == -1) {
            //printf("Solution found!\n");
            const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now() - start
            ).count();

            chain c;
            encoder.extract_chain(spec, c);
            assert(c.simulate()[0] == tt);
            assert(c.is_mag());
            assert(c.get_nr_steps() == 7);

            c.print_mag();
            printf("Time elapsed: %ldus (SAT FENCE)\n", elapsed);
        }
    }

    {
        const auto start = std::chrono::steady_clock::now();
        chain c;

        ditt_maj_synthesize(7, c);

        const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now() - start).count();
        printf("Time elapsed: %ldus (STD CEGAR)\n", elapsed);
    }

    {
        chain c;
        kitty::dynamic_truth_table maj_tt(7);
        kitty::create_majority(maj_tt);

        const auto start = std::chrono::steady_clock::now();

        pf_ditt_maj_synthesize(7, c);

        const auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now() - start
            ).count();

        assert(c.simulate()[0] == maj_tt);
        assert(c.is_mag());
        assert(c.get_nr_steps() == 7);
        c.print_mag();
        printf("Time elapsed: %ldus (PAR FENCE)\n", elapsed);
    }

    return 0;
}

