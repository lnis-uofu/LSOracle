/*  cnf-stat  Copyright (c) 2011, Vegard Nossum
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

extern "C" {
#include <stdio.h>
#include <stdlib.h>
}

#include <map>

#include "cnf.hh"

int main(int argc, char *argv[])
{
	FILE *fp;
	if (argc > 1) {
		fp = fopen(argv[1], "r");
		/* XXX: Real error checking */
		assert(fp);
	} else {
		fp = stdin;
	}

	unsigned int nr_variables;
	unsigned int nr_clauses;
	if (fscanf(fp, "p cnf %u %u\n", &nr_variables, &nr_clauses) != 2) {
		fprintf(stderr, "error: expected: p cnf <variables> <clauses>\n");
		exit(EXIT_FAILURE);
	}

	cnf f;
	for (unsigned int i = 0; i < nr_clauses; ++i) {
		cnf::clause::ptr clause(new cnf::clause());

		while (1) {
			int literal;
			if (fscanf(fp, "%d", &literal) != 1) {
				fprintf(stderr, "error: expected literal\n");
				exit(EXIT_FAILURE);
			}

			if (!literal)
				break;

			clause->add(literal);
		}

		f.add(clause);
	}

	fclose(fp);

	unsigned int nr_literals = 0;
	unsigned int nr_negative_literals = 0;
	unsigned int nr_positive_literals = 0;
	for (cnf::clause_vector::iterator it = f.clauses.begin(),
		end = f.clauses.end(); it != end; ++it)
	{
		cnf::clause::ptr clause(*it);

		nr_literals += clause->literals.size();

		for (cnf::clause::literal_vector::iterator lit = clause->literals.begin(),
			lend = clause->literals.end(); lit != lend; ++lit)
		{
			cnf::literal literal(*lit);

			if (literal < 0)
				++nr_negative_literals;
			else
				++nr_positive_literals;
		}
	}

	printf("Literals: %u (%u negative, %u positive)\n",
		nr_literals, nr_negative_literals, nr_positive_literals);

	typedef std::map<unsigned int, unsigned int> hist_map;
	hist_map clause_length_hist;

	for (cnf::clause_vector::iterator it = f.clauses.begin(),
		end = f.clauses.end(); it != end; ++it)
	{
		cnf::clause::ptr clause(*it);

		++clause_length_hist[clause->literals.size()];
	}

	printf("Clause length histogram:\n");
	for (hist_map::iterator it = clause_length_hist.begin(),
		end = clause_length_hist.end(); it != end; ++it)
	{
		printf("%2u: %u\n", it->first, it->second);
	}

	return 0;
}
