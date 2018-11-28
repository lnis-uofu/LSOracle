/*  cnf-sort-clauses  Copyright (c) 2011, Vegard Nossum
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

static bool clauses_cmp(const cnf::clause::ptr &a, const cnf::clause::ptr &b)
{
	cnf::clause::literal_vector::iterator ait = a->literals.begin();
	cnf::clause::literal_vector::iterator aend = a->literals.end();
	cnf::clause::literal_vector::iterator bit = b->literals.begin();
	cnf::clause::literal_vector::iterator bend = b->literals.end();

	while (ait != aend && bit != bend) {
		cnf::literal a = *ait;
		cnf::literal b = *bit;

		if (a < b)
			return true;
		if (a > b)
			return false;

		++ait;
		++bit;
	}

	if (ait == aend)
		return true;
	if (bit == bend)
		return false;

	return false;
}

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

	std::sort(f.clauses.begin(), f.clauses.end(), clauses_cmp);

	printf("p cnf %u %u\n", nr_variables, nr_clauses);
	f.print(stdout);

	fclose(fp);
	return 0;
}
