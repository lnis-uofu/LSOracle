High Effort Classification
===========================

Instead of using the DNN model, a high effort method is used to determine what algorithm to use for each partition. After partitioning, each partition is optimized using both AIG and MIG methods and the one that reduces the area and delay product is the method to be used. 

Note: This metric is not the same used to train the DNN model, and the model was also trained on cones rather than full partitions.

.. code-block:: c++
	
	//Read AIGER file and store as AIG network
	read_aig <AIGER file>
	//Partition stored AIG 
	partitioning <number of partitions>
	//-b flag denotes using high effort classification
	optimization -b -o <Verilog file to write to>
