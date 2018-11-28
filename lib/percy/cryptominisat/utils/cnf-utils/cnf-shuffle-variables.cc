/*  cnf-shuffle-variables  Copyright (c) 2011, Vegard Nossum
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
#include <time.h>
}

#include <algorithm>

#include "cnf.hh"

/* TODO:
 * Maybe variable shuffling/renaming should come with an option to output
 * the mapping so that solutions can be mapped back. Maybe that's a job for
 * another tool 'cnf-remap'?
 */

int main(int argc, char *argv[])
{
	/* XXX: This is not actually guaranteed to change the result of
	 * std::random_shuffle, but it works with _MY_ gcc/libc/libstdc++ */
	srand(time(NULL));

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
		cnf::clause::ptr c(new cnf::clause());

		while (1) {
			int literal;
			if (fscanf(fp, "%d", &literal) != 1) {
				fprintf(stderr, "error: expected literal\n");
				exit(EXIT_FAILURE);
			}

			if (!literal)
				break;

			c->add(literal);
		}

		f.add(c);
	}

	/* Shuffle variables */
	std::vector<int> variables(nr_variables + 1);
	for (unsigned int i = 0; i < nr_variables + 1; ++i)
		variables[i] = i;

	std::random_shuffle(++variables.begin(), variables.end());

	for (cnf::clause_vector::iterator cit = f.clauses.begin(),
		cend = f.clauses.end(); cit != cend; ++cit)
	{
		cnf::clause::ptr c(*cit);

		for (cnf::clause::literal_vector::iterator lit = c->literals.begin(),
			lend = c->literals.end(); lit != lend; ++lit)
		{
			int literal = *lit;
			if (literal < 0)
				*lit = -variables[-*lit];
			else
				*lit = variables[*lit];
		}
	}

	printf("p cnf %u %u\n", nr_variables, nr_clauses);
	f.print(stdout);

	fclose(fp);
	return 0;
}
