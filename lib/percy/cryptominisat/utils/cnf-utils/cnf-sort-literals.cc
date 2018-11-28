/*  cnf-sort-literals  Copyright (c) 2011, Vegard Nossum
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

#include <algorithm>

#include "cnf.hh"

int main(int argc, char *argv[])
{
	if (argc < 1) {
		fprintf(stderr, "Usage: %s [OPTION]... [FILE] ...\n", argv[0]);
		exit(EXIT_FAILURE);
	}

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

	for (cnf::clause_vector::iterator it = f.clauses.begin(),
		end = f.clauses.end(); it != end; ++it)
	{
		cnf::clause::ptr c(*it);
		std::sort(c->literals.begin(), c->literals.end());
	}

	printf("p cnf %u %u\n", nr_variables, nr_clauses);
	f.print(stdout);

	fclose(fp);
	return 0;
}
