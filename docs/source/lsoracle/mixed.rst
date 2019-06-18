Two-Step Mixed Synthesis
=========================

This method also uses classification, but it is slightly different. This method also partitions an AIG network and performs classification, but it only optimizes partitions that have been classified to use AIG optimization. The resulting network is then partitioned and re-classified, and now MIG classified partitions are optimized. Similar to the above command, the flag -b can be used instead of -c deep_learn_model.json to use a brute force method for classification.

.. code-block:: c++
	
	read_aig <AIGER file>
	mixed_2step -p 5 -c deep_learn_model.json -o <Verilog file to write to>
