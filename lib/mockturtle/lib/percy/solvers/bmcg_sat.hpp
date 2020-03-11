#pragma once

#include "solver_wrapper.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wpedantic"
#include <abc/AbcGlucose.h>
#pragma GCC diagnostic pop

namespace percy
{
    class bmcg_wrapper : public solver_wrapper
    {
    private:
        abc::bmcg_sat_solver * solver = NULL;

    public:
        bmcg_wrapper()
        {
            solver = abc::bmcg_sat_solver_start();
        }

        ~bmcg_wrapper()
        {
            abc::bmcg_sat_solver_stop(solver);
            solver = NULL;
        }

        void restart()
        {
            abc::bmcg_sat_solver_reset(solver);
        }

        void set_nr_vars(int nr_vars)
        {
            abc::bmcg_sat_solver_set_nvars(solver, nr_vars);
        }

        int nr_vars()
        {
            // Not currently supported
            return -1;
        }

        int nr_clauses()
        {
            return abc::bmcg_sat_solver_clausenum(solver);
        }

        int nr_conflicts()
        {
            return abc::bmcg_sat_solver_conflictnum(solver);
        }

        int add_clause(abc::lit* begin, abc::lit* end)
        {
            return abc::bmcg_sat_solver_addclause(solver, begin, end - begin);
        }

        void add_var()
        {
            abc::bmcg_sat_solver_addvar(solver);
        }

        int var_value(int var)
        {
            return abc::bmcg_sat_solver_read_cex_varvalue(solver, var);
        }

        synth_result solve(int cl)
        {
            abc::bmcg_sat_solver_set_conflict_budget(solver, cl);
            auto res = abc::bmcg_sat_solver_solve(solver, 0, 0);
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
            abc::bmcg_sat_solver_set_conflict_budget(solver, cl);
            auto res = abc::bmcg_sat_solver_solve(solver, begin, end - begin);
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
