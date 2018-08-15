import glob
import subprocess
import os

workingdir = os.getcwd()

path = './blif/*.blif'

files = glob.glob(path)
file_index = 0

txtfile = open('./blif/training/outputdata.txt','w') 

for file in files:
	print(file)
	txtfile.write('Data For File: ' + file + '\n')
	#cmd = ['./abc','read ' + file, 'ps', 'resyn' 'empty', 'quit']
	#cmd = ['./abc','-c read ' + file + '; ps; resyn; resyn2; ps; empty; quit;']
	cmd = ['./abc','-c read ' + file + '; ps; resyn; resyn2; ps; write ./blif/training/trainingset' + str(file_index) + '.blif; empty; quit;']
	process = subprocess.Popen(cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
	process.wait()
	for line in process.stdout:
		txtfile.write(str(line) + '\n')
		print(line)
	file_index += 1
	txtfile.write('\n\n')

txtfile.close()
