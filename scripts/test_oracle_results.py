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

header_1 = "Node x Depth"
header_2 = "Node"
header_3 = "Depth"

for curr_file in files:

	file = curr_file[len(test_path):]
	file = file[:-4]
	print('\n' + file)
	if results_write:
		results_file.write('\n' + file + '\n')
	os.chdir(lso_path)

	cmd = ['./lsoracle','-c', 'read_aig ' + curr_file + '; ps -a;']
	process = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
	stdout, stderr = process.communicate()
	string_stdout = str(stdout)
	print('Original Benchmark')
	print string_stdout[string_stdout.find('AIG nodes:'):].split('\n')[0]
	print string_stdout[string_stdout.find('AIG level'):].split('\n')[0]

	size = float(string_stdout[string_stdout.find('AIG nodes:'):].split('\n')[0][11:])
	num_part = math.ceil(size / 300)

	print str(num_part) + ' partitions'

	os.chdir(lso_path)
	opt_file1 = lso_path + '/' + file + '_product.v'
	print('\n' + header_1)
	if results_write:
		results_file.write('\n' + header_1 + '\n')
	cmd = ['./lsoracle','-c', 'oracle ' + curr_file + ' ' + str(num_part) + ' -b --strategy 0 -o ' + opt_file1 + ';']
	process = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
	stdout, stderr = process.communicate()
	string_stdout = str(stdout)
	for line in string_stdout.split('\n'):
		if "AIGs and " in line or "Final ntk size" in line:
			print line.strip()
	if results_write:
		for line in string_stdout.split('\n'):
			if "AIGs and " in line or "Final ntk size" in line:
				results_file.write( line.strip() + '\n' )

	os.chdir(abc_path)
	cmd = ['./abc','-c', 'cec -n ' + curr_file + ' ' + opt_file1]
	process = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
	stdout, stderr = process.communicate()
	string_stdout = str(stdout)
	print string_stdout[string_stdout.find('Networks '):].split('\n')[0]
	if results_write:
		results_file.write(string_stdout[string_stdout.find('Networks '):].split('\n')[0] + '\n')

	os.chdir(lso_path)
	opt_file2 = lso_path + '/' + file + '_node.v'
	print('\n' + header_2)
	if results_write:
		results_file.write('\n' + header_2 + '\n')
	cmd = ['./lsoracle','-c', 'oracle ' + curr_file + ' ' + str(num_part) + ' -b --strategy 1 -o ' + opt_file2 + ';']
	process = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
	stdout, stderr = process.communicate()
	string_stdout = str(stdout)
	for line in string_stdout.split('\n'):
		if "AIGs and " in line or "Final ntk size" in line:
			print line.strip()
	if results_write:
		for line in string_stdout.split('\n'):
			if "AIGs and " in line or "Final ntk size" in line:
				results_file.write( line.strip() + '\n' )

	os.chdir(abc_path)
	cmd = ['./abc','-c', 'cec -n ' + curr_file + ' ' + opt_file2]
	process = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
	stdout, stderr = process.communicate()
	string_stdout = str(stdout)
	print string_stdout[string_stdout.find('Networks '):].split('\n')[0]
	if results_write:
		results_file.write(string_stdout[string_stdout.find('Networks '):].split('\n')[0] + '\n')

	os.chdir(lso_path)
	opt_file3 = lso_path + '/' + file + '_depth.v'
	print('\n' + header_3)
	if results_write:
		results_file.write('\n' + header_3 + '\n')
	cmd = ['./lsoracle','-c', 'oracle ' + curr_file + ' ' + str(num_part) + ' -b --strategy 2 -o ' + opt_file3 + ';']
	process = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
	stdout, stderr = process.communicate()
	string_stdout = str(stdout)
	for line in string_stdout.split('\n'):
		if "AIGs and " in line or "Final ntk size" in line:
			print line.strip()
	if results_write:
		for line in string_stdout.split('\n'):
			if "AIGs and " in line or "Final ntk size" in line:
				results_file.write( line.strip() + '\n' )

	os.chdir(abc_path)
	cmd = ['./abc','-c', 'cec -n ' + curr_file + ' ' + opt_file3]
	process = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
	stdout, stderr = process.communicate()
	string_stdout = str(stdout)
	print string_stdout[string_stdout.find('Networks '):].split('\n')[0]
	if results_write:
		results_file.write(string_stdout[string_stdout.find('Networks '):].split('\n')[0] + '\n')

	

	
	