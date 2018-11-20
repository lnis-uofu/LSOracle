/** Complete all-combination generator by Vegard Nossum
 * All bugs are due to fiddling around by Mate Soos
 */
#include <cstdio>
#include <stdint.h>
#include <cassert>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

int main(int argc, char *argv[])
{
    if (argc != 2) {
        cerr << "ERROR: You must give the clause length as parameter" << endl;
        exit(EXIT_FAILURE);
    }

    const long int nr_vars = strtol(argv[1], NULL, 10);

    if ((errno == ERANGE && (nr_vars == LONG_MAX || nr_vars == LONG_MIN))
        || (errno != 0 && nr_vars == 0)
    ) {
        perror("strtol");
        exit(EXIT_FAILURE);
    }

    if (nr_vars > 62) {
        cerr << "Too large value given: " << nr_vars << endl;
        exit(EXIT_FAILURE);
    }

    const unsigned long num = (1UL << nr_vars);
    cout << "p cnf " << nr_vars << " " << num << endl;
    //cout << "c argv[0]: " << argv[0] << endl;
    //cout << "c argv[1]: " << argv[1] << endl;

    unsigned long x = 0;
    do {
        for (int i = 0; i < nr_vars; ++i)
            cout << ((x >> i) & 1 ? "-" : "") << 1 + i << " ";

        cout << "0" << endl;
        ++x;
    } while (x != num);

    return 0;
}
