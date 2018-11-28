#!/usr/bin/env python
# -*- coding: utf-8 -*-

print("NOTE: this should import sage in some way")

def gen_cnf(n):
    A = random_matrix(GF(2),n,n+1)
    B = BooleanPolynomialRing(n,'x')
    v = Matrix(B, n+1, 1, B.gens() + (1,))
    l = (A*v).list()
    a2 = ANFSatSolver(B)
    s = a2.cnf(l)

    fname = "matrix%02d.cnf" % n
    print("written matrix CNF to file %s" % fname)
    open(fname).write(s)


gen_cnf(1)
