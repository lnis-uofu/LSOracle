import glob
import subprocess
import os

# CONVERT BLIF TO AIG

path = './blif/*.blif'

files = glob.glob(path)
file_index = 0

for file in files:
	print(file)
	#cmd = ['./abc','read ' + file, 'ps', 'resyn' 'empty', 'quit']
	cmd = ['./abc','-c read ' + file + '; istrash; write ./blif/aig2/aigset' + str(file_index) + '.aig; empty; quit;']
	#cmd = ['./abc','-c read ' + file + '; ps; resyn; resyn2; ps; write ./blif/aig2/aigset' + str(file_index) + '.blif; empty; quit;']
	process = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
	process.wait()
	for line in process.stdout:
		print(line)
	file_index += 1

# CONVERT AIG TO MIG, THEN BACK TO AIG USING CIRKIT

path = './aig/*.aig'

files = glob.glob(path)
file_index = 0	

txtfile = open('./aig/outputdata.txt','w')

for file in files:
	print(file)
	txtfile.write('Data For File: ' + file + '\n')
	#cmd = ['./abc','read ' + file, 'ps', 'resyn' 'empty', 'quit']
	#cmd = ['./abc','-c read ' + file + '; ps; resyn; resyn2; ps; empty; quit;']
	cmd = ['./cirkit','-c read_aiger ' + file + '; ps -a; convert --aig_to_mig; mig_rewrite; convert --mig_to_aig; write_aiger ./aig/output/trainingset' + str(file_index) + '.aig; quit;']
	process = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
	process.wait()
	for line in process.stdout:
		txtfile.write(str(line) + '\n')
		print(line)
	file_index += 1
	txtfile.write('\n\n')

txtfile.close()

# CONVERT AIG BACK TO BLIF

path = './blif/*.blif'

files = glob.glob(path)
file_index = 0

for file in files:
	print(file)
	#cmd = ['./abc','read ' + file, 'ps', 'resyn' 'empty', 'quit']
	cmd = ['./abc','-c read ' + file + '; write ./blif/cirkitblif/trainingset' + str(file_index) + '.blif; empty; quit;']
	#cmd = ['./abc','-c read ' + file + '; ps; resyn; resyn2; ps; write ./blif/aig/aigset' + str(file_index) + '.blif; empty; quit;']
	process = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
	process.wait()
	for line in process.stdout:
		print(line)
	file_index += 1




