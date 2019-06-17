Optimization with Classification
=================================

The main functionality of this command is to use the DNN model to determine the best-fit algorithm for each partition of a network. The general flow of this command is: read network into either AIG (must begin with AIG network), perform k-means hypergraph partitioning, determine the algorithm to use with each using classification, perform optimization on each partition, merge partitions back into one, and write out an optimized network. An example of using this command is shown below.

.. code-block:: c++
	
	//Read AIGER file and store as AIG network
	read_aig <AIGER file>
	//Partition stored AIG 
	partitioning <number of partitions>
	//Perform classification, optimization, merging, and write out to Verilog file
	optimization -c deep_learn_model.json -o <Verilog file to write to>

There are also a couple other options for this command.

.. toctree::

    aig_part
    mig_part
    brute
