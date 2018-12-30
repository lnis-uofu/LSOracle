#include "solvertypes.h"
#include "MersenneTwister.h"
#include <vector>
#include <iostream>
#include <time.h>
#include <string.h>

using std::endl;
using std::cout;
using std::vector;
using CMSat::Lit;

static MTRand mtrand;
static uint64_t numVars;

int main(int argc, char** argv)
{
    int seed = time(NULL);
    if (argc >= 2) {
        char* end = NULL;
        seed = strtol(argv[1], &end, 10);
        if (end != argv[1] + strlen(argv[1])) {
            cout << "ERROR Seed value you gave was not an integer!" << endl;
            exit(-1);
        }
        cout << "c using set value " << seed << " as seed" << endl;
    } else {
        cout << "c using time(NULL) as seed" << endl;
    }
    mtrand.seed(seed);
    numVars = mtrand.randInt(5ULL*1000ULL*1000ULL) + 1ULL*1000ULL*1000ULL;
    if (mtrand.randInt(10) == 0) {
        numVars += mtrand.randInt(8ULL*1000ULL*1000ULL);
    }

    size_t numLargeClauses = mtrand.randInt(150000);
    size_t numSmallClauses = 100ULL*10000ULL + mtrand.randInt(600ULL*1000ULL);
    cout << "c small clauses: " << numSmallClauses << endl;
    cout << "c large clauses: " << numLargeClauses << endl;
    cout
    << "p cnf " << numVars + 1
    << " " << numLargeClauses + numSmallClauses
    << endl;

    //Create MANY larege clauses
    for(size_t i = 0, end = numLargeClauses
        ; i < end
        ; i++
    ) {
        size_t size = mtrand.randInt(250);
        size += 200;
        for(size_t i2 = 0; i2 < size; i2++) {
            cout
            << Lit(mtrand.randInt(numVars), mtrand.randInt(1))
            << " ";
        }
        cout << "0" << endl;
    }

    vector<char> alreadyHit;
    alreadyHit.resize((numVars+1)*2, 0);

    //Lots of long binary chains
    size_t numSmall = 0;
    Lit lit = Lit(mtrand.randInt(numVars), mtrand.randInt(1));
    size_t i = 0;
    while(numSmall < numSmallClauses) {
        i++;
        //Pick a fresh one that hasn't been hit yet
        do {
            lit = Lit(
                mtrand.randInt(numVars)
                , mtrand.randInt(1)
            );
        } while(alreadyHit[lit.toInt()] == 1);
        assert(alreadyHit[lit.toInt()] == 0);
        alreadyHit[lit.toInt()] = 1;

        //And chain to it
        size_t size = mtrand.rand(i*10);
        for(size_t i2 = 0; i2 < size; i2++) {

            //Chain linearly
            Lit lit2;
            for(size_t i3 = 0, i3size = mtrand.randInt(500) + 40
                ; i3 < i3size
                ; i3++
            ) {
                do {
                    lit2 = Lit(
                        mtrand.randInt(numVars)
                        , mtrand.randInt(1)
                    );
                } while(alreadyHit[lit2.toInt()] == 1);
                assert(alreadyHit[lit2.toInt()] == 0);
                alreadyHit[lit2.toInt()] = 1;

                //print binary
                cout
                << ~lit
                << " "
                << lit2
                << " 0" << endl;
                numSmall++;

                //Exit if the number of small clauses is the right amount
                if (numSmall == numSmallClauses)
                    return 0;
            }

            //Chain next one
            lit = lit2;
        }
    }
}
