/*  cnf-utils  Copyright (c) 2011, Vegard Nossum
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CNF_HH
#define CNF_HH

#include <boost/shared_ptr.hpp>

#include <vector>

class cnf {
public:
	typedef int literal;

	class clause {
	public:
		typedef boost::shared_ptr<clause> ptr;
		typedef std::vector<literal> literal_vector;

		literal_vector literals;
        bool satisfied;

		clause() :
            satisfied(false)
		{
		}

		bool add(literal x)
		{
            if (std::find(literals.begin(), literals.end(), x) == literals.end()) {
                literals.push_back(x);
                return true;
            } else
                return false;
		}
	};

	typedef std::vector<clause::ptr> clause_vector;

	clause_vector clauses;

	cnf()
	{
	}

	void add(clause::ptr c)
	{
        assert(!c->satisfied);
		clauses.push_back(c);
	}

	void print(FILE *fp);
};

inline std::ostream& operator<<(std::ostream& os, const cnf::clause c)
{
    for(size_t i = 0; i < c.literals.size(); i++) {
        os << c.literals[i] << " ";
    }
    os  << "0 satisfied: " << c.satisfied;
    return os;
}

inline void cnf::print(FILE *fp)
{
    std::vector<char> litsInside;
    long unsigned nr_variables = 0;
    long unsigned nr_clauses = 0;
    std::vector<char> print_clause;
    size_t at_cl = 0;
    for (clause_vector::iterator cit = clauses.begin(),
        cend = clauses.end(); cit != cend
        ; ++cit, ++at_cl)
    {
        print_clause.resize(at_cl+1, 1);
        if ((*cit)->satisfied) {
            print_clause[at_cl]= 0;
            continue;
        }

        litsInside.clear();
        bool isSAT = false;
        for (clause::literal_vector::iterator lit = (*cit)->literals.begin(),
            lend = (*cit)->literals.end(); lit != lend; ++lit)
        {
            nr_variables = std::max<long unsigned >(nr_variables, abs(*lit));

            unsigned at = std::abs(*lit)*2;
            unsigned atother = at;
            if (*lit > 0) {
                at += 1;
            } else {
                atother += 1;
            }

            unsigned mymax = std::max(at,atother);
            if (litsInside.size() < mymax)
                litsInside.resize(mymax+1, 0);

            if (litsInside[atother]) {
                isSAT = true;
                print_clause[at_cl] = 0;
            }
            litsInside[at] = 1;
        }
        if (!isSAT) {
            nr_clauses++;
        } else {
            //std::cout << "Satisfied clause: " << *(*cit) << std::endl;
        }
    }
    printf("p cnf %lu %lu\n", nr_variables, nr_clauses);

    at_cl = 0;
    for (clause_vector::iterator cit = clauses.begin(),
        cend = clauses.end(); cit != cend
        ; ++cit, ++at_cl)
    {
        if (print_clause[at_cl]) {
            for (clause::literal_vector::iterator lit = (*cit)->literals.begin(),
                lend = (*cit)->literals.end(); lit != lend; ++lit)
            {
                fprintf(fp, "%d ", *lit);
            }

            fprintf(fp, "0\n");
        }
    }
}

#endif
