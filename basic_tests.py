#End to end and unit test script
#forked from Max's testing_results.py script
#Scott Temple 20MAY2019
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

#These may need to be modified based on individual installation
#making this more robust is on the to-do list once this is functional
home_path = os.getenv('HOME')
lstools_path = home_path + '/LSOracle/build/core'
abc_path = home_path + '/abc'
timestamp = datetime.now()
timestamp_format = timestamp.strftime('%Y%m%d%H%M%S')
print ('LSOracle test suite     ')
print(timestamp)
print('\nHome path: ' + home_path + '\n')

#End to end tests
test_path = lstools_path + '/../../test_files/end_to_end'
test_path_glob = test_path + '/*.aig'
print('End to end tests\n')
print('Test path: ' + test_path + '\n')
files = glob.glob(test_path_glob)
print('\n')

results_file_path = test_path + '/' + timestamp_format + '_basic_tests.txt'
results_file = open(results_file_path,'w')
#we'll have to do some more thinking about what a good end to end test looks like.  For now I'm going to optimize a couple benchmarks
#using aig, mig, mixed, and brute force, and report those.  I'll have a failure message if our method is slower than 
#both mig and aig.  It ought to, at least, be between them.
for curr_file in files:
    print(curr_file + '\n')
    results_file.write(curr_file + '\n')
    os.chdir(lstools_path)

    #report statistics
    cmd = ['./lstools','-c', 'read_aig ' + curr_file + '; ps -a;']
    process = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE, shell=True)
	stdout, stderr = process.communicate()
    string_stdout = str(stdout)
    size = float(string_stdout[7:string_stdout.find('\n')])
	num_part = math.ceil(size / 300)
	print('size: ' + size +' partitions = size/300:  ' + num_part + '\n')
    results_file.write('size: ' + size +' partitions = size/300:  ' + num_part + '\n')
    #mixed synthesis with classifier
    opt_file = curr_file + '_mixed_out.v'
    #this assumes that cnn_model.json is in the lstools_path root directory
    cmd = ['./lstools','-c', 'read_aig ' + curr_file + '; mixed -c cnn_model.json -p ' + str(num_part) + ' -o ' + opt_file + ';']
    results_file.write('mixed synthesis with classifier\n')
    results_file.write(cmd)
    results_file.write('\n')
    process = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE, shell=True)
	stdout, stderr = process.communicate()
    string_stdout = str(stdout)
    print('mixed synthesis done.  See file for detailed output.\n')
    results_file.write(string_stdout)
    results_file.write('\n ')
    #I need a way to grab the results for each of these so I can compare each method alert when it fails

    #Brute Force
    opt_file = curr_file + '_brute_out.v'
    cmd = ['./lstools','-c', 'read_aig ' + curr_file + '; mixed -b -p ' + str(num_part) + ' -o ' + opt_file + ';']
    results_file.write('brute force mixed synthesis\n')
    results_file.write(cmd)
    results_file.write('\n')
    process = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE, shell=True)
	stdout, stderr = process.communicate()
    string_stdout = str(stdout)
    print('brute force mixed synthesis done.  See file for detailed output.\n')
    results_file.write(string_stdout)
    results_file.write('\n ')
   
    #AIG only
    opt_file = curr_file + '_AIG_out.v'
    #shouldn't be aig_partition.  Not sure what it is right now
    cmd = ['./lstools','-c', 'read_aig ' + curr_file + '; aig_partition -p ' + str(num_part) + ' -o ' + opt_file + ';']
    results_file.write('AIG only\n')
    results_file.write(cmd)
    results_file.write('\n')
    process = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE, shell=True)
	stdout, stderr = process.communicate()
    string_stdout = str(stdout)
    print('AIG done.  See file for detailed output.\n')
    results_file.write(string_stdout)
    results_file.write('\n ')
   
    #MIG only
    opt_file = curr_file + '_MIG_out.v'
    cmd = ['./lstools','-c', 'read_aig ' + curr_file + '; mig_partition -p ' + str(num_part) + ' -o ' + opt_file + ';']
    results_file.write('MIG only\n')
    results_file.write(cmd)
    results_file.write('\n')
    process = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE, shell=True)
	stdout, stderr = process.communicate()
    string_stdout = str(stdout)
    print('MIG done.  See file for detailed output.\n')
    results_file.write(string_stdout)
    results_file.write('\n ')
   
#a unit test suite can go here.  Will likely move it to the top once it's written
#
#test_path = lstools_path + '/../../test_files/unit'
#test_path_glob = test_path + '/*.aig'
#print('Unit tests\n')
#print('Test path: ' + test_path + '\n')
#files = glob.glob(test_path_glob)
#results_file.write('\n\nBegin unit tests\n')

