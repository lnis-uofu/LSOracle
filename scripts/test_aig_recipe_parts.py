# TensorFlow and tf.keras
import argparse
import sys
import glob
import math
# Helper libraries
import numpy as np
import os
import shutil
import subprocess
import matplotlib.pyplot as plt
import re
import time
import timeit
from datetime import datetime

# path to lstools.cpp
home_path = os.getenv('HOME') + '/../../research/ece/lnis/USERS/austin/'
lso_path = home_path + 'test/curr_repo/refactor/LSOracle/build/core/'

# path to abc
abc_path = home_path + 'abc/'

# path to benchmark files
test_path = ''

parser = argparse.ArgumentParser()
parser.add_argument("-b", "--bench", help="Choose directory for benchmark files REQUIRED")
parser.add_argument("-out", "--output_file", help="Choose file to write results to (if null, no results saved)")
args = parser.parse_args()

if args.bench:
	test_path = args.bench
else:
	print('Benchmark directory must be specified')
	exit()

test_path_glob = test_path + '*.aig'
files = glob.glob(test_path_glob)

results_write = 0
results_file_path = ''
if args.output_file:
	results_write = 1
	results_file_path = args.output_file
	results_file = open(results_file_path,'w')

header_1 = "balance"
abc_header_1 = "ABC balance"
header_2 = "refactor"
abc_header_2 = "ABC refactor"
header_3 = "cut_rewriting"
abc_header_3 = "ABC cut_rewriting"

for curr_file in files:

	file = curr_file[len(test_path):]
	file = file[:-4]
	print('\n' + file)
	if results_write:
		results_file.write('\n' + file + '\n')
	os.chdir(lso_path)

	print(header_1)
	if results_write:
		results_file.write(header_1 + '\n')
	cmd = ['./lsoracle','-c', 'read_aig ' + curr_file + '; ps -a; balance; ps -a;']
	process = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
	stdout, stderr = process.communicate()
	string_stdout = str(stdout)

	print string_stdout[string_stdout.find('Final ntk size'):].split('\n')[0]
	if results_write:
		results_file.write(string_stdout[string_stdout.find('Final ntk size'):].split('\n')[0] + '\n')

	os.chdir(abc_path)
	print(abc_header_1)
	if results_write:
		results_file.write(abc_header_1 + '\n')
	cmd = ['./abc','-c', 'read ' + curr_file + '; b; ps;']
	process = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
	stdout, stderr = process.communicate()
	string_stdout = str(stdout)

	print string_stdout.splitlines()[-1]
	if results_write:
		results_file.write(string_stdout.splitlines()[-1] + '\n')

	os.chdir(lso_path)
	print(header_2)
	if results_write:
		results_file.write(header_2 + '\n')
	cmd = ['./lsoracle','-c', 'read_aig ' + curr_file + '; ps -a; refactor; ps -a;']
	process = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
	stdout, stderr = process.communicate()
	string_stdout = str(stdout)
	
	print string_stdout[string_stdout.find('Final ntk size'):].split('\n')[0]
	if results_write:
		results_file.write(string_stdout[string_stdout.find('Final ntk size'):].split('\n')[0] + '\n')

	os.chdir(abc_path)
	print(abc_header_2)
	if results_write:
		results_file.write(abc_header_2 + '\n')
	cmd = ['./abc','-c', 'read ' + curr_file + '; rf; ps;']
	process = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
	stdout, stderr = process.communicate()
	string_stdout = str(stdout)

	print string_stdout.splitlines()[-1]
	if results_write:
		results_file.write(string_stdout.splitlines()[-1] + '\n')

	os.chdir(lso_path)
	print(header_3)
	if results_write:
		results_file.write(header_3 + '\n')
	cmd = ['./lsoracle','-c', 'read_aig ' + curr_file + '; ps -a; cut_rewriting; ps -a;']
	process = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
	stdout, stderr = process.communicate()
	string_stdout = str(stdout)
	
	print string_stdout[string_stdout.find('Final ntk size'):].split('\n')[0]
	if results_write:
		results_file.write(string_stdout[string_stdout.find('Final ntk size'):].split('\n')[0] + '\n')

	os.chdir(abc_path)
	print(abc_header_3)
	if results_write:
		results_file.write(abc_header_3 + '\n')
	cmd = ['./abc','-c', 'read ' + curr_file + '; rw; ps;']
	process = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
	stdout, stderr = process.communicate()
	string_stdout = str(stdout)

	print string_stdout.splitlines()[-1]
	if results_write:
		results_file.write(string_stdout.splitlines()[-1] + '\n')

	

	
