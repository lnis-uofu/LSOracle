Partitions with only AIG Optimization
======================================

This option skips classification and optimizes all partitions using just AIG optimization.

.. code-block:: c++
	
	//Read AIGER file and store as AIG network
	read_aig <AIGER file>
	//Partition stored AIG 
	partitioning <number of partitions>
	//-a flag denotes using AIG optimization
	optimization -a -o <Verilog file to write to>