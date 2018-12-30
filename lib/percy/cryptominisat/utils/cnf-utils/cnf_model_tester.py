#!/usr/bin/env python3

"""
    Model tester
    ~~~~~~~~~~~~

    Does the given model satisfy the given CNF?
    If no, show failing clause.

    Two files are required. One file containing a CNF in DIMACS format::

        p cnf 8 5
        1 2 3 4 0
        5 4 2 1 0
        2 3 6 0
        6 -7 8 0
        1 2 5 8 0

    Secondly, a model file is given defining a model::

        -1
        -2
        3
        4
        -5
        -8

    Invoking this script with both files as parameter,
    it will return line 6 of the CNF file, because the model contradicts
    with this clause.

    Note. This script does not use the "p"-line in the DIMACS file.

    (C) 2013, Public Domain, Lukas Prokop
"""

import re
import sys
import collections
import argparse

# Note. You can also supply the model here
model = {
    #1: False, 2: False, 3: True, 4: True, 5: False, ...
}


# global stats
lines = 0
clauses = 0
all_vars = set()


def read_model(fp, missing_var=False):
    """Read a model from the given model file.

    :param fp:          open file pointer (read perm) to modelfile
    :param missing_var: how to handle missing variables?
    :type missing_var:  either False (are False) or None (KeyError)
    """
    global lines, clauses, all_vars

    if missing_var is False:
        model = collections.defaultdict(lambda: False)
    elif missing_var is None:
        model = {}

    for line in fp.readlines():
        m = re.match('^-?(\d+)', line)

        lines += 1
        if not m:
            continue

        var = int(m.group(1))
        all_vars.add(var)
        if line.startswith('-'):
            model[var] = False
        else:
            model[var] = True

    return model


def test_clauses(fp, model):
    """Test clauses of CNF from output file with given model.

    :param fp:       open file pointer (read perm) to DIMACS file
    :param model:    the model {var: bool} to use
    """
    global lines, clauses, all_vars

    for line in fp.readlines():
        # parse line
        m = re.match('^(((-?\d+) )*(-?\d+))( 0)?\s*$', line)

        lines += 1
        if not m:
            # not a line defining a clause
            continue
        else:
            clauses += 1

        lits = m.group(1).split(' ')
        lits.pop()

        clause = []
        variables = {}
        for lit in lits:
            try:
                if lit.startswith('-'):
                    var = int(lit[1:])
                    val = not model[var]
                else:
                    var = int(lit)
                    val = model[var]
            except KeyError:
                msg = 'Variable {} is not defined in model'.format(var)
                raise KeyError(msg)

            all_vars.add(var)
            variables[var] = model[var]
            clause.append(val)

        if not any(clause):
            print('I have found a contradiction.')
            print('  Line:', line[:-1])
            print('  Variables:', variables)
            print('This line does not contain any literal which becomes true.')
            print()
            return False

    print('{} lines read. {} clauses tested. {} variables found.'
        .format(lines, clauses, len(all_vars)))
    return True


def main():
    """Main routine"""
    global model

    parser = argparse.ArgumentParser(
        description='Evaluate CNF with given model.'
    )

    parser.add_argument('cnffile', type=argparse.FileType('r'),
        help='DIMACS file providing a CNF')
    if not model:
        parser.add_argument('modelfile', type=argparse.FileType('r'),
            help='file containing model (one literal per line)')
    parser.add_argument('-f', '--false', dest='missing_var', default=None,
        action='store_false',
        help='var is missing in modelfile means var is 0'
    )
    args = parser.parse_args()

    if not model:
        model = read_model(args.modelfile, args.missing_var)

    if not model:
        raise ValueError('Given model is empty.')

    if test_clauses(args.cnffile, model):
        print('Model is valid. Satisfied!')
    else:
        print('Unsatisfied with given model.')
        sys.exit(1)


if __name__ == '__main__':
    main()
