/*  cnf-cat  Copyright (c) 2011, Vegard Nossum
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

#include "cnf.hh"

int main(int argc, char *argv[])
{
	if (argc <= 1) {
		fprintf(stderr, "Usage: %s [OPTION]... [FILE] ...\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	cnf f;
	unsigned int max_nr_variables = 0;

	for (int i = 1; i < argc; ++i) {
		FILE *fp = fopen(argv[i], "r");
		/* XXX: Real error checking */
		assert(fp);

		unsigned int nr_variables;
		unsigned int nr_clauses;
		if (fscanf(fp, "p cnf %u %u\n", &nr_variables, &nr_clauses) != 2) {
			fprintf(stderr, "error: %s: expected: p cnf <variables> <clauses>\n", argv[i]);
			exit(EXIT_FAILURE);
		}

		if (nr_variables > max_nr_variables)
			max_nr_variables = nr_variables;

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
	}

	printf("p cnf %u %lu\n", max_nr_variables, f.clauses.size());
	f.print(stdout);

	return 0;
}
