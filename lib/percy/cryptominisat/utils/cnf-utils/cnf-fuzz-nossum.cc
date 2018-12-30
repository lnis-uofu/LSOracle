/*****************************************************************************
cnf fuzzer -- Copyright (c) 2011 Vegard Nossum

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "MersenneTwister.h"
#include <vector>
#include <cassert>
using std::vector;

class clause {
public:
    bool is_xor;
    vector<int> lits;

    clause(bool _is_xor = false) :
        is_xor(_is_xor)
    {
    }
};

static MTRand rnd;

static unsigned int randNorm(double mean, double variance)
{
    double r = rnd.randNorm(mean, variance);
    if (r < 0)
        return 0;
    return r;
}

vector<clause> get_gate(MTRand& rnd, const unsigned nr_variables)
{
    vector<clause> ret;
    size_t gatesize = rnd.randInt(30)+1;

    //Fill literals
    vector<int> lits(gatesize);
    for(size_t i = 0; i < gatesize; i++) {
        bool OK = false;
        while(!OK) {
            int thisLit = 1 + rnd.randInt(nr_variables-1);
            OK = true;
            for(size_t i2 = 0; i2 < i; i2++) {
                if (abs(lits[i]) == thisLit) {
                    OK = false;
                    break;
                }
            }
            int multiplier = 1 - 2*rnd.randInt(1);
            lits[i] = thisLit * multiplier;
        }
    }

    //The long clause
    clause cl;
    cl.lits.push_back(-1*lits[0]);
    for(size_t i = 1; i < gatesize; i++) {
        cl.lits.push_back(lits[i]);
    }
    ret.push_back(cl);

    //The many short clauses
    for(size_t i = 1; i < gatesize; i++) {
        clause cl;
        cl.lits.push_back(lits[0]);
        cl.lits.push_back(-1*lits[i]);
        ret.push_back(cl);
    }

    return ret;
}

static vector<clause> clauses;
bool need_regular_clauses = true;

int main(int, char **)
{
    unsigned int nr_variables = randNorm(8000, 300);
        nr_variables++;
    assert(nr_variables > 0);
    unsigned num_final_clauses = 0;

    unsigned int nr_constraints = 2 + rnd.randInt(4);

    vector<unsigned int> offsets;
    vector<char> polarities;
    for (unsigned int i = 0; i < nr_constraints; ++i) {
        //Only every 15th should be an XOR
        bool is_xor = (rnd.randInt(3) == 1);

        unsigned size;
        if (rnd.randInt(10) == 1) {
            size = 1 + randNorm(10, 2);
        } else {
            size = 1 + randNorm(3, 2);
        }
        offsets.resize(size);
        polarities.resize(size);

        //Set offsets and polarities
        for (unsigned int j = 0; j < size; ++j) {
            offsets[j] = rnd.randInt();
            polarities[j] = rnd.randInt(1);
        }
        unsigned int stride = 1 + rnd.randInt(3);

        for (unsigned int j = 0; j < nr_variables; j += stride) {
            clause c(is_xor);

            //Vary the size of the clause
            if (j % 3 == 1) {
                size -= (int)rnd.randInt(2) - 1;
                if (size < 1)
                    size = 3;
            }

            //If it's an XOR, don't make it so large
            if (c.is_xor)
                size = std::min<unsigned>(size, 6);

            for (unsigned int k = 0; k < size; ++k) {
                int lit = (polarities[k] ? -1 : 1) * (1 + ((j + offsets[k]) % nr_variables));
                c.lits.push_back(lit);
            }

            clauses.push_back(c);
            if (!c.is_xor || !need_regular_clauses) {
                num_final_clauses++;
            } else {
                num_final_clauses += 1UL << (c.lits.size()-1);
            }
        }
    }

    //Add a some completely random clauses
    size_t simpl_rand_constrs = rnd.randInt(5000);
    for (unsigned int i = 0; i < simpl_rand_constrs; ++i) {
        size_t size = 1+rnd.randNorm(10,2);
        clause cl;
        for(size_t i2 = 0; i2 < size; i2++) {
            cl.lits.push_back( (1-(2*rnd.randInt(1))) * (1+rnd.randInt(nr_variables-1)) );
        }
        clauses.push_back(cl);
        num_final_clauses++;
    }

    //Add a some random gates
    size_t other_constraints = randNorm(200, 20);
    for(size_t i = 0; i < other_constraints; i++) {
        vector<clause> add_cls = get_gate(rnd, nr_variables);
        for(size_t i2 = 0; i2 < add_cls.size(); i2++) {
            clauses.push_back(add_cls[i2]);
            num_final_clauses++;
        }
    }

    //Add a couple of unitary clauses
    for(size_t i = 0; i < 10; i++) {
        clause cl;
        cl.lits.push_back( (1-(rnd.randInt(1)*2)) * (1+rnd.randInt(nr_variables-2)) );
        clauses.push_back(cl);
        num_final_clauses++;
    }

    printf("p cnf %u %lu\n", nr_variables, (long unsigned)num_final_clauses);
    for (unsigned int i = 0; i < clauses.size(); ++i) {
        const clause &c = clauses[i];

        //If XOR, handle differently
        if (c.is_xor && need_regular_clauses) {
            for (unsigned int k = 0; k < (1U << c.lits.size()); ++k) {
                if (__builtin_parity(k))
                    continue;

                for (unsigned int l = 0; l < c.lits.size(); ++l) {
                    printf("%d ", ((k & (1 << l)) ? -1 : 1) * c.lits[l]);
                }
                printf("0\n");
            }
        } else {
            //If XOR, but doesn't need regular clause conversion, print X
            if (c.is_xor) {
                printf("x ");
            }

            //Print the literals
            for (unsigned int k = 0; k < c.lits.size(); ++k) {
                printf("%d ", c.lits[k]);
            }
            printf("0\n");
        }
    }

    return 0;
}
