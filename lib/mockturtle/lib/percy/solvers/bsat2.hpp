#pragma once

#include "solver_wrapper.hpp"

namespace percy
{
    class bsat_wrapper : public solver_wrapper
    {
    private:
        abc::sat_solver * solver = NULL;

    public:
        bsat_wrapper()
        {
            solver = abc::sat_solver_new();
        }

        ~bsat_wrapper()
        {
            abc::sat_solver_delete(solver);
            solver = NULL;
        }

        void restart()
        {
            abc::sat_solver_restart(solver);
        }

        void set_nr_vars(int nr_vars)
        {
            abc::sat_solver_setnvars(solver, nr_vars);
        }

        int nr_vars()
        {
            return abc::sat_solver_nvars(solver);
        }

        int nr_clauses()
        {
            return abc::sat_solver_nclauses(solver);
        }

        int nr_conflicts()
        {
            return abc::sat_solver_nconflicts(solver);
        }

        int add_clause(abc::lit* begin, abc::lit* end)
        {
            return abc::sat_solver_addclause(solver, begin, end);
        }

        void add_var()
        {
            abc::sat_solver_addvar(solver);
        }

        int var_value(int var)
        {
            return abc::sat_solver_var_value(solver, var);
        }

        synth_result solve(int cl)
        {
            auto res = abc::sat_solver_solve(solver, 0, 0, cl, 0, 0, 0);
            if (res == 1) {
                return success;
            } else if (res == -1) {
                return failure;
            } else {
                return timeout;
            }
        }

        synth_result solve(abc::lit* begin, abc::lit* end, int cl)
        {
            auto res = abc::sat_solver_solve(solver, begin, end, cl, 0, 0, 0);
            if (res == 1) {
                return success;
            } else if (res == -1) {
                return failure;
            } else {
                return timeout;
            }
        }

    };
}
