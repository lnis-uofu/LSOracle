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
home_path = os.getenv('HOME')
lstools_path = home_path + '/LSOracle/build/core'
abc_path = home_path + '/abc'
training_file = home_path + '/LSOracle/cnn_model.json'
timestamp = datetime.now()
timestamp_format = timestamp.strftime('%Y%m%d%H%M%S')
print ("LSOracle test suite     ")
print(timestamp)
print('\nHome path: ' + home_path + '\n')

#End to end tests
test_path = lstools_path + '/../../tests/end_to_end'
test_path_glob = test_path + '/*.aig'
print('End to end tests\n')
print('Test path: ' + test_path + '\n')
files = glob.glob(test_path_glob)
print(files)

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
    process = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
    stdout, stderr = process.communicate()
    string_stdout = str(stdout)
    #number of nodes
    unoptimized_size = float(string_stdout[7:string_stdout.find('\n')])
    num_part = math.ceil(unoptimized_size / 300)
    print(num_part)
    results_file.write('Size (# nodes before optimization): ' + str(unoptimized_size) +' partitions = size/300:  ' + str(num_part) + '\n')
    #mixed synthesis with classifier
    opt_file = curr_file + '_mixed_out.v'
    cmd = ['./lstools','-c', 'read_aig ' + curr_file + '; mixed -c ' + training_file + ' -p ' + str(num_part) + ' -o ' + opt_file + ';']
    results_file.write('mixed synthesis with classifier\n')
    results_file.write(str(cmd))
    results_file.write('\n')
    process = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
    stdout, stderr = process.communicate()
    string_stdout = str(stdout)
    cnn_size = int(string_stdout[string_stdout.find('new ntk size = '):string_stdout.find('Finished optimization') - 1])
    print("new ntk size: " + str(cnn_size) + '\n')
    results_file.write("new ntk size: " + str(cnn_size) + '\n')

    #Brute Force
    opt_file = curr_file + '_brute_out.v'
    cmd = ['./lstools','-c', 'read_aig ' + curr_file + '; mixed -b -p ' + str(num_part) + ' -o ' + opt_file + ';']
    results_file.write('brute force mixed synthesis\n')
    results_file.write(str(cmd))
    results_file.write('\n')
    process = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
    stdout, stderr = process.communicate()
    string_stdout = str(stdout)
    print('brute force mixed synthesis done.  See file for detailed output.\n')
    results_file.write(string_stdout)
    results_file.write('\n ')
   
    #AIG only
    opt_file = curr_file + '_AIG_out.v'
    cmd = ['./lstools','-c', 'read_aig ' + curr_file + '; aig_partition -p ' + str(num_part) + ' -o ' + opt_file + ';']
    results_file.write('AIG only\n')
    results_file.write(str(cmd))
    results_file.write('\n')
    process = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
    stdout, stderr = process.communicate()
    string_stdout = str(stdout)
    print('AIG done.  See file for detailed output.\n')
    results_file.write(string_stdout)
    results_file.write('\n ')
    print ("AIG done")
    #MIG only
    opt_file = curr_file + '_MIG_out.v'
    cmd = ['./lstools','-c', 'read_aig ' + curr_file + '; mig_partition -p ' + str(num_part) + ' -o ' + opt_file + ';']
    results_file.write('MIG only\n')
    results_file.write(str(cmd))
    results_file.write('\n')
    process = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
    stdout, stderr = process.communicate()
    string_stdout = str(stdout)
    print('MIG done.  See file for detailed output.\n')
    results_file.write(string_stdout)
    results_file.write('\n ')
   
    #a unit test suite can go here.  Will likely move it to the top once it's written
