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

for curr_file in files:

	file = curr_file[len(test_path):]
	file = file[:-4]
	print('\n\n' + file)
	if results_write:
		results_file.write('\n\n' + file + '\n')
	
	os.chdir(abc_path)
	print('b')
	if results_write:
		results_file.write('b' + '\n')

	cmd = ['./abc','-c', 'read ' + curr_file + '; b; ps;']
	process = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
	stdout, stderr = process.communicate()
	string_stdout = str(stdout)
	print string_stdout.splitlines()[-1]
	if results_write:
		results_file.write(string_stdout.splitlines()[-1] + '\n')

	print('rw')
	if results_write:
		results_file.write('rw' + '\n')
	cmd = ['./abc','-c', 'read ' + curr_file + '; b; rw; ps;']
	process = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
	stdout, stderr = process.communicate()
	string_stdout = str(stdout)
	print string_stdout.splitlines()[-1]
	if results_write:
		results_file.write(string_stdout.splitlines()[-1] + '\n')

	print('rf')
	if results_write:
		results_file.write('rf' + '\n')
	cmd = ['./abc','-c', 'read ' + curr_file + '; b; rw; rf; ps;']
	process = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
	stdout, stderr = process.communicate()
	string_stdout = str(stdout)
	print string_stdout.splitlines()[-1]
	if results_write:
		results_file.write(string_stdout.splitlines()[-1] + '\n')

	print('b')
	if results_write:
		results_file.write('b' + '\n')
	cmd = ['./abc','-c', 'read ' + curr_file + '; b; rw; rf; b; ps;']
	process = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
	stdout, stderr = process.communicate()
	string_stdout = str(stdout)
	print string_stdout.splitlines()[-1]
	if results_write:
		results_file.write(string_stdout.splitlines()[-1] + '\n')

	print('rw')
	if results_write:
		results_file.write('rw' + '\n')
	cmd = ['./abc','-c', 'read ' + curr_file + '; b; rw; rf; b; rw; ps;']
	process = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
	stdout, stderr = process.communicate()
	string_stdout = str(stdout)
	print string_stdout.splitlines()[-1]
	if results_write:
		results_file.write(string_stdout.splitlines()[-1] + '\n')

	print('rwz')
	if results_write:
		results_file.write('rwz' + '\n')
	cmd = ['./abc','-c', 'read ' + curr_file + '; b; rw; rf; b; rw; rwz; ps;']
	process = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
	stdout, stderr = process.communicate()
	string_stdout = str(stdout)
	print string_stdout.splitlines()[-1]
	if results_write:
		results_file.write(string_stdout.splitlines()[-1] + '\n')

	print('b')
	if results_write:
		results_file.write('b' + '\n')
	cmd = ['./abc','-c', 'read ' + curr_file + '; b; rw; rf; b; rw; rwz; b; ps;']
	process = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
	stdout, stderr = process.communicate()
	string_stdout = str(stdout)
	print string_stdout.splitlines()[-1]
	if results_write:
		results_file.write(string_stdout.splitlines()[-1] + '\n')

	print('rfz')
	if results_write:
		results_file.write('rfz' + '\n')
	cmd = ['./abc','-c', 'read ' + curr_file + '; b; rw; rf; b; rw; rwz; b; rfz; ps;']
	process = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
	stdout, stderr = process.communicate()
	string_stdout = str(stdout)
	print string_stdout.splitlines()[-1]
	if results_write:
		results_file.write(string_stdout.splitlines()[-1] + '\n')

	print('rwz')
	if results_write:
		results_file.write('rwz' + '\n')
	cmd = ['./abc','-c', 'read ' + curr_file + '; b; rw; rf; b; rw; rwz; b; rfz; rwz; ps;']
	process = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
	stdout, stderr = process.communicate()
	string_stdout = str(stdout)
	print string_stdout.splitlines()[-1]
	if results_write:
		results_file.write(string_stdout.splitlines()[-1] + '\n')

	print('b')
	if results_write:
		results_file.write('b' + '\n')
	cmd = ['./abc','-c', 'read ' + curr_file + '; b; rw; rf; b; rw; rwz; b; rfz; rwz; b; ps;']
	process = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
	stdout, stderr = process.communicate()
	string_stdout = str(stdout)
	print string_stdout.splitlines()[-1]
	if results_write:
		results_file.write(string_stdout.splitlines()[-1] + '\n')

	

	
