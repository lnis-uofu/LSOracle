# LSOracle ERI Demo July 7/17/2019

## Introduction

For this demo we show the capabilities of the LSOracle tool on a benchmark from the OpenPiton Design Benchmark (OPDB) suite. Optimization of this file requires only 3 commands: read_aig, partitioning, and optimization.

```{r, engine='bash', count_lines}
./lstools                                                #run the tool
read_aig chip_bridge.aig                                 #read in the OpenPiton benchmark as an AIG network 
partitioning 415                                         #perform k-way hypergraph partitioning, any number of partitions can be used but 415 were used in the demo since this partition number creates partitions with about 300 nodes in all partitions
optimization -b -o chip_bridge_out.v                     #perform mixed synthesis optimization and write out the optimized Verilog file
```
