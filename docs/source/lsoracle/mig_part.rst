Partitions with only MIG Optimization
=====================================

This option skips classification and optimizes all partitions using just MIG optimization. Note that the AIGER file must still be stored as an AIG network. Conversion for MIG optimization will occur after partitioning.

.. code-block:: c++
	
	//Read AIGER file and store as AIG network
	read_aig <AIGER file>
	//Partition stored AIG 
	partitioning <number of partitions>
	//-m flag denotes using MIG optimization
	optimization -m -o <Verilog file to write to>