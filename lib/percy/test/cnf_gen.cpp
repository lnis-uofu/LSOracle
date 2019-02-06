#include <cstdio>
#include <percy/percy.hpp>
#if !defined(_WIN32) && !defined(_WIN64)
#ifdef USE_GLUCOSE
#include <glucose/utils/ParseUtils.h>
#else
#include <syrup/core/Dimacs.h>
#include <syrup/utils/ParseUtils.h>
#endif
#endif

using namespace percy;

/// Test the generation of DIMACS output from encoded exact synthesis instances.
int
main(void)
{
    spec spec;

    kitty::dynamic_truth_table tt(4);
    kitty::create_from_hex_string(tt, "cafe");

    printf("Generating CNF for f = cafe\n");

    spec[0] = tt;
    chain c;

    // Synthesize it to see what the minimum number of steps is.
    auto status = synthesize(spec, c);
    assert(status == success);
    auto min_nr_steps = c.get_nr_steps();
    printf("min_nr_steps=%d\n", min_nr_steps);
    
    // Generate cnf formulas up to the minimum nr of steps and
    // make sure that all but the last are UNSAT.
    cnf_formula cnf;
    ssv_encoder encoder(cnf);

    for (int i = 1; i <= min_nr_steps; i++) {
        const auto filename = std::string("cnf_") + std::to_string(i) + std::string(".cnf");

        auto fhandle = fopen(filename.c_str(), "w");
        if (fhandle == NULL) {
            fprintf(stderr, "Error: unable to open CNF output file\n");
            return 1;
        }
        spec.nr_steps = i;
        spec.preprocess();

        cnf.clear();
        encoder.encode(spec);
        cnf.to_dimacs(fhandle);
        fclose(fhandle);

        printf("generated DIMACS for %d steps\n", spec.nr_steps);
        
#if !defined(_WIN32) && !defined(_WIN64)
        Glucose::SimpSolver s;
        s.parsing = 1;
        s.use_simplification = 1;
        gzFile in = gzopen(filename.c_str(), "rb");
        Glucose::parse_DIMACS(in, s);
        s.parsing = 0;
        s.eliminate(true);
        if (!s.okay()){
            printf("Solved by simplification\n");
            printf("UNSATISFIABLE\n");
        } else {
            Glucose::vec<Glucose::Lit> dummy;
            auto ret = Glucose::toInt(s.solveLimited(dummy));
            if (ret == 1) {
                printf("UNSATISFIABLE\n");
            } else if (ret == 0) {
                printf("SATISFIABLE\n");
            } else {
                printf("TIMEOUT\n");
            }
        }
        gzclose(in);
#endif
    }
    
    // Test majority synthesis
    printf("Generating CNF for f = MAJ-3\n");
    kitty::dynamic_truth_table maj3(3);
    kitty::create_majority(maj3);
    spec.fanin = 3;
    spec.set_primitive(MAJ);
    spec[0] = maj3;
    status = synthesize(spec, c);
    assert(status == success);
    min_nr_steps = c.get_nr_steps();
    printf("min_nr_steps=%d\n", min_nr_steps);
    
    for (int i = 1; i <= min_nr_steps; i++) {
        const auto filename = std::string("cnf_") + std::to_string(i) + std::string(".cnf");

        auto fhandle = fopen(filename.c_str(), "w");
        if (fhandle == NULL) {
            fprintf(stderr, "Error: unable to open CNF output file\n");
            return 1;
        }
        spec.nr_steps = i;
        spec.preprocess();

        cnf.clear();
        encoder.encode(spec);
        cnf.to_dimacs(fhandle);
        fclose(fhandle);

        printf("generated DIMACS for %d steps\n", spec.nr_steps);
        
#if !defined(_WIN32) && !defined(_WIN64)
        Glucose::SimpSolver s;
        s.parsing = 1;
        s.use_simplification = 1;
        gzFile in = gzopen(filename.c_str(), "rb");
        Glucose::parse_DIMACS(in, s);
        s.parsing = 0;
        s.eliminate(true);
        if (!s.okay()){
            printf("Solved by simplification\n");
            printf("UNSATISFIABLE\n");
        } else {
            Glucose::vec<Glucose::Lit> dummy;
            auto ret = Glucose::toInt(s.solveLimited(dummy));
            if (ret == 1) {
                printf("UNSATISFIABLE\n");
            } else if (ret == 0) {
                printf("SATISFIABLE\n");
            } else {
                printf("TIMEOUT\n");
            }
        }
        gzclose(in);
#endif
    }

    printf("Generating CNF for f = MAJ-5\n");
    kitty::dynamic_truth_table maj5(5);
    kitty::create_majority(maj5);
    spec.fanin = 3;
    spec.set_primitive(MAJ);
    spec[0] = maj5;
    status = synthesize(spec, c);
    assert(status == success);
    min_nr_steps = c.get_nr_steps();
    printf("min_nr_steps=%d\n", min_nr_steps);
    
    for (int i = 1; i <= min_nr_steps; i++) {
        const auto filename = std::string("cnf_") + std::to_string(i) + std::string(".cnf");

        auto fhandle = fopen(filename.c_str(), "w");
        if (fhandle == NULL) {
            fprintf(stderr, "Error: unable to open CNF output file\n");
            return 1;
        }
        spec.nr_steps = i;
        spec.preprocess();

        cnf.clear();
        encoder.encode(spec);
        cnf.to_dimacs(fhandle);
        fclose(fhandle);

        printf("generated DIMACS for %d steps\n", spec.nr_steps);
        
#if !defined(_WIN32) && !defined(_WIN64)
        Glucose::SimpSolver s;
        s.parsing = 1;
        s.use_simplification = 1;
        gzFile in = gzopen(filename.c_str(), "rb");
        Glucose::parse_DIMACS(in, s);
        s.parsing = 0;
        s.eliminate(true);
        if (!s.okay()){
            printf("Solved by simplification\n");
            printf("UNSATISFIABLE\n");
        } else {
            Glucose::vec<Glucose::Lit> dummy;
            auto ret = Glucose::toInt(s.solveLimited(dummy));
            if (ret == 1) {
                printf("UNSATISFIABLE\n");
            } else if (ret == 0) {
                printf("SATISFIABLE\n");
            } else {
                printf("TIMEOUT\n");
            }
        }
        gzclose(in);
#endif
    }

    return 0;
}

