sha1-sat -- SAT instance generator for SHA-1
============================================


# Compiling

To compile, you first need to make sure you have the Boost libraries
installed. When you do, simply run make.sh:

```
bash make.sh
```

# Running

Before running, please make sure that the espresso binary is in your PATH.
espresso is used to minimise the truth tables for the pseudo-boolean
constraints used to encode the adders. You can obtain espresso from
<ftp://ftp.cs.man.ac.uk/pub/amulet/balsa/other-software/espresso-ab-1.0.tar.gz>.

```
wget ftp://ftp.cs.man.ac.uk/pub/amulet/balsa/other-software/espresso-ab-1.0.tar.gz
tar xzvf espresso-ab-1.0.tar.gz
cd espresso-ab-1.0
./configure
make
cd ..
export PATH="$PWD/espresso-ab-1.0/src:$PATH"
```

To generate a CNF instance encoding a preimage attack on the full SHA-1
algorithm, run:

```
./main --cnf --rounds=80 --hash-bits=160 > instance.cnf
```

To look at the possible options, run:

```
./main --help
```

The program can also generate OPB instances (pseudo-boolean constraints) if
you specify --opb instead of --cnf.


# Verifying solutions

To verify that the solution output by the solver is actually correct, run:

```
perl verify-preimage instance.cnf solution | ./verify-preimage
```

Here, 'solution' is the file output e.g. by minisat or the 'v'-line for
other popular solvers like CryptoMiniSAT or PrecoSAT. The program returns
an error code of 0 if and only if the solution is correct.


# About

I developed this program as part of my master thesis. Please e-mail comments,
suggestions, etc. to me at <vegard.nossum@gmail.com>.

If you use the program in your research, please make a note of this in your
acknowledgements and let me know about your paper/thesis/etc.!
