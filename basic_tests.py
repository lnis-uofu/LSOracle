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
parser.add_argument('--verbose', '-v', action='count', help='output detail.  Default (unset) = warning; -v = info, -vv = debug')
parser.add_argument('--test_directory', '-t', help='If you have a custom set of test files, specify path here.  Default LSOracle/tests. The directory you specify should have 2 subdirectories: end_to_end and unit_tests, and the input files should be .aig format')
parser.add_argument('--training_model', '-m', default='/LSOracle/cnn_model.json', help='if you have a custom tensorflow model for the classifier, specify it here.')
args = parser.parse_args()

#saving paths for future use
home_path = os.getenv('HOME')
lstools_path = home_path + '/LSOracle/build/core'
abc_path = home_path + '/abc'
training_file = home_path + args.training_model

#configure logging
timestamp = datetime.now()
timestamp_format = timestamp.strftime('%Y%m%d%H%M%S')
log_level = 'WARNING'
if args.verbose == 1:
    log_level = 'INFO'
if args.verbose > 1:
    log_level = 'DEBUG'
if args.log_to_file:
    log_filename = timestamp_format + '_lsoracle_test.log'
    logging.basicConfig(filename=log_filename,format='%(asctime)s:%(levelname)s:%(message)s', level=log_level)
else:
    logging.basicConfig(format='%(asctime)s:%(levelname)s:%(message)s', level=log_level)

#Define our function to call the lstools executable
def optimize(filename, mode, part_num, suffix):
    opt_file = filename + suffix + '.v'
    cmd = ['./lstools','-c', 'read_aig ' + filename + '; partitioning ' + str(part_num) + '; ' + mode + '-p'  + str(part_num) + ' -o ' + opt_file + ';']
    process = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
    stdout, stderr = process.communicate()
    string_stdout = str(stdout)
    string_stderr = str(stderr)
    if string_stderr:
        logging.warning(string_stderr)
    cnn_size = int(string_stdout[string_stdout.find('new ntk size = '):string_stdout.find('Finished optimization') - 1])
    return cnn_size

#Begin tests
print('LSOracle test suite ' + str(timestamp))
print(home_path)
logging.debug('Home path: %s', home_path)
#End to end tests
#Grab my test files
test_path = lstools_path + '/../../tests/end_to_end'
test_path_glob = test_path + '/*.aig'
logging.info('End to end tests\n')
logging.info('Test path: %s', test_path)
print(test_path)
files = glob.glob(test_path_glob)
logging.debug("List of test files: ")
logging.debug(files)
print(files)
#Actual testing
#we'll have to do some more thinking about what a good end to end test looks like.  For now I'm going to optimize a couple benchmarks
#using aig, mig, mixed, and brute force, and report those.  I'll have a failure message if our method is slower than 
#both mig and aig.  It ought to, at least, be between them.
for curr_file in files:
    print(curr_file + '\n')
    os.chdir(lstools_path)
   
    #report statistics
    cmd = ['./lstools','-c', 'read_aig ' + curr_file + '; ps -a;']
    process = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
    stdout, stderr = process.communicate()
    string_stdout = str(stdout)
    #calculate number of nodes
    unoptimized_size = float(string_stdout[7:string_stdout.find('\n')])
    num_part = math.ceil(unoptimized_size / 300)
    print('Size (# nodes before optimization): ' + str(unoptimized_size) +' partitions = size/300:  ' + str(num_part) + '\n')
   
    #mixed synthesis with classifier
    cmdstr = 'optimization -c ' + training_file
    mixed_size = optimize(curr_file, cmdstr, num_part, '_mixed_out')
    print('ntk size after mixed synthesis: ' + str(mixed_size) + '\n')

    #Brute Force
    cmdstr = 'optimization -b'
    brute_size = optimize(curr_file, cmdstr, num_part, '_brute_out')
    print('ntk size after brute force: ' + str(brute_size) + '\n')
    
    #AIG only
    cmdstr = 'optimization -a'
    aig_size = optimize(curr_file, cmdstr, num_part, '_aig_out')
    print('ntk size after aig optimization: ' + str(aig_size) + '\n')
    
    #MIG only
    cmdstr = 'optimization -m'
    mig_size = optimize(curr_file, cmdstr, num_part, '_mig_out')
    print('ntk size after mig optimization: ' + str(mig_size) + '\n')
    
    assert (mixed_size <= aig_size or mixed_size <= mig_size) or (brute_size <= aig_size or brute_size <= mig_size)