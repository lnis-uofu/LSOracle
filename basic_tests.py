import argparse
import sys
import glob
import math
import numpy as np
import os
import shutil
import subprocess
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import re
import time
import timeit
from datetime import datetime
import logging

#Set up command line parser
parser = argparse.ArgumentParser(prog='lstools_test', description='Test script for LSOracle')
parser.add_argument('--log_to_file', action='store_true', help='print log information to specified filename in LSOracle directory')
parser.add_argument('--verbose', '-v', action='count',
                    help='output detail.  Default (unset) = warning; -v = info, -vv = debug')
parser.add_argument('--test_directory', '-t',
                    default='./tests/end_to_end',
                    help='Specify path to end-to-end test files. input files should be .aig format')
parser.add_argument('--unit_test_directory', '-u',
                    default='./tests/unit',
                    help='Specify path to unit test files. input files should be .aig format')
parser.add_argument('--training_model', '-m',
                    default='./core/algorithms/classification/deep_learn_model.json',
                    help='if you have a custom tensorflow model for the classifier, specify it here.')
parser.add_argument('--cicd ', action='store_true',
                    help='sets paths, envs, etc, to appropriate values for ci')
parser.add_argument('--kahypar', default='./core/test.ini',
                    help='kahypar config file.')
parser.add_argument('--lsoracle_exe', default='./build/core/lsoracle',
                    help='LSOracle executable.')
parser.add_argument('--abc_exe', default='abc',
                    help='abc executable.')
args = parser.parse_args()

training_file = os.path.abspath(args.training_model)
unit_path = os.path.abspath(args.unit_test_directory)
test_path = os.path.abspath(args.test_directory)

#configure logging
timestamp = datetime.now()
timestamp_format = timestamp.strftime('%Y%m%d%H%M%S')
log_level = 'WARNING'

if args.verbose is None:
    log_level = 'WARNING'
elif args.verbose == 1:
    log_level = 'INFO'
elif args.verbose > 1:
    log_level = 'DEBUG'

if args.log_to_file:
    log_filename = timestamp_format + '_lsoracle_test.log'
    logging.basicConfig(filename=log_filename,format='%(asctime)s:%(levelname)s:%(message)s', level=log_level)
else:
    logging.basicConfig(format='%(asctime)s:%(levelname)s:%(message)s', level=log_level)

#Define our function to call the lstools executable
def optimize(filename, mode, part_num, suffix):
    opt_file = filename + suffix + '.v'
    cmd = [args.lsoracle_exe,'-c', 'read_aig ' + filename + '; partitioning ' + str(part_num) + ' -c ' + args.kahypar + '; ' + mode + ' -o ' + opt_file + ';']
    process = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
    stdout, stderr = process.communicate()
    string_stdout = str(stdout, encoding="utf-8").splitlines()
    logging.debug(str(stdout, encoding="utf-8"))
    if stderr is not None:
        logging.error(str(stderr, encoding="utf-8"))
    logging.debug(string_stdout[-6])
    return [int(s) for s in string_stdout[-6].split() if s.isdigit()]

def compare(filename, suffix):
    new_file = filename + '.v'
    opt_file = filename + suffix + '.v'
    #need to create verilog file to compare to
    cmd = [args.lsoracle_exe, '-c', 'read_aig ' + curr_file + '; write_verilog ' + new_file + ';']
    subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE)

    #use cec to compare the pre and post optimization files
    cmd = [args.abc_exe, '-c', 'cec -n ' + new_file +' '+ opt_file + ';']
    abc_process = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
    abc_stdout, abc_stderr = abc_process.communicate()
    intermediate_string = str(abc_stdout, encoding="utf-8")
    logging.debug(intermediate_string)
    if abc_stderr is not None:
        logging.error(str(abc_stderr, encoding="utf-8"))
    string_abc = intermediate_string.splitlines()
    return string_abc[-1]

#Begin tests
logging.info('LSOracle test suite')
#End to end tests
#Grab my test files
test_path_glob = test_path + '/*.aig'
logging.info('End to end tests:')
logging.info('Test path: %s', test_path)
files = glob.glob(test_path_glob)
logging.debug("List of test files: " + ", ".join(files))
#Actual testing
#we'll have to do some more thinking about what a good end to end test looks like.  For now I'm going to optimize a couple benchmarks
#using aig, mig, mixed, and brute force, and report those.  I'll have a failure message if our method is slower than
#both mig and aig.  It ought to, at least, be between them.
for curr_file in files:
    logging.info(curr_file)

    #report statistics
    cmd = [args.lsoracle_exe,'-c', 'read_aig ' + curr_file + '; ps -a;']
    process = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
    stdout, stderr = process.communicate()
    string_stdout = str(stdout, encoding='utf-8')
    logging.debug(string_stdout)
    if stderr is not None:
        logging.error(str(stderr, encoding='utf-8'))
    #calculate number of nodes
    unoptimized_size = float(string_stdout.splitlines()[1][7:string_stdout.find('\n')])
    num_part = math.ceil(unoptimized_size / 300)
    logging.info('Size (# nodes before optimization): ' + str(unoptimized_size) +' partitions = size/300:  ' + str(num_part))

    #mixed synthesis with classifier
    cmdstr = 'optimization -n ' + training_file
    mixed_size = optimize(curr_file, cmdstr, num_part, '_mixed_out')
    logging.info('ntk size after mixed synthesis: ' + str(mixed_size[0]) + ' depth: ' + str(mixed_size[1]))
    abcout = compare(curr_file, '_mixed_out')
    assert('Networks are equivalent' in abcout)

    #Brute Force
    cmdstr = 'optimization'
    brute_size = optimize(curr_file, cmdstr, num_part, '_brute_out')
    logging.info('ntk size after brute force: ' + str(brute_size[0]) + ' depth: ' + str(brute_size[1]))
    abcout = compare(curr_file, '_brute_out')
    assert('Networks are equivalent' in abcout)

    #AIG only
    cmdstr = 'optimization -a'
    aig_size = optimize(curr_file, cmdstr, num_part, '_aig_out')
    logging.info('ntk size after aig optimization: ' + str(aig_size[0]) + ' depth: ' + str(aig_size[1]))
    abcout = compare(curr_file, '_aig_out')
    assert('Networks are equivalent' in abcout)

    #MIG only
    cmdstr = 'optimization -m'
    mig_size = optimize(curr_file, cmdstr, num_part, '_mig_out')
    logging.info('ntk size after mig optimization: ' + str(mig_size[0]) + ' depth: ' + str(mig_size[1]))
    abcout = compare(curr_file, '_mig_out')
    assert('Networks are equivalent' in abcout)

    assert (mixed_size[0] <= aig_size[0] or mixed_size[0] <= mig_size[0]) or (brute_size[0] <= aig_size[0] or brute_size[0] <= mig_size[0])

#unit tests.  This is a stub.
#Grab my test files
logging.info('\nUnit tests:')
unit_path_glob = unit_path + '/*.aig'
logging.info('Unit tests\n')
logging.info('Test path: %s', unit_path)
files = glob.glob(unit_path_glob)
logging.debug("List of test files: ")
logging.debug(", ".join(files))
