Detailed Command Usage
====================================

- aigscript

  Performs homogeneous AIG optimization using interleaved rewriting, refactoring, and balancing, similar to ABC's resyn2. AIG network must be stored before use.
  
  
- balance

  Performs network balancing.  Uses AIG network by default.  Use -m flag to balance stored MIG network.
  
  
- cut_e

  Performs cut enumeration on a stored AIG network
  
  
- cut_rewriting

  Performs cut rewriting on a stored network (AIG default.  MIG use -m option).
  Default cut size is 4; use -c INT to set an alternative cut size.
  
  
- depth

  Displays depth of current network. AIG default; use -m for MIG, -x for XAG.
  
  
- depthr

  Performs depth oriented MIG rewriting.  MIG only.
  
  
- interleaving

  Performs interleaved MIG rewriting and refactoring.
  
  
- migscript

  Performs homogeneous MIG optimization using interleaved rewriting, refactoring, and balancing, similar to ABC's resyn2.  MIG network must be stored before use.
  
  
- optimization

  Performs Mixed Synthesis on network after partitioning.  Network must be loaded and separately partitioned before use.
    * "-n" to specify neural network model
    * "-o" to specify verilog output
    * "-s INT" to specify classification strategy. 0 = Balanced, 1 = Area optimized, 2 = Delay optimized
    * "-a" to perform only AIG optimization
    * "-m" to perform only MIG optimization
    * "-c" to combine adjacent partitions of the same type
    * "--skip-feedthrough" to not include feedthrough nets when writing output
  
  
- oracle

  All in one command to partition stored AIG network and perform mixed synthesis, as with "optimization" command.  Uses all flags in optimization command.
    * "--partition INT" to manually specify the partition count instead of using the automatic selection.
  
  
- rwscript

  XAG cut rewriting
  
  
- generate_truth_tables

  Generates truth tables for every logic cone in every partition.  AIG default; use "-m" for MIG
  
  
- read_aig

  Reads an AIGER file (binary, not ASCII) and stores the resulting network.  It is stored as an AIG by default.
    * "-m" store resulting network as MIG
    * "-x" store resulting network as XAG
  
  
- read_bench

  Reads a bench file and stores the resulting network as a KLUT network.
    * "-a" store resulting network as AIG
    * "-m" store resulting network as MIG
    * "-x" store resulting network as XAG
  
  
- read_blif
  
  Reads a bench file and stores the resulting network as a KLUT network.
    * "-a" store resulting network as AIG
    * "-m" store resulting network as MIG
    * "-x" store resulting network as XAG
    
    
- read_verilog
  
  Reads a simple, structural verilog file and stores the resulting network as AIG.  If you need to read real world verilog files, use the Yosys plugin, or use Yosys to create an AIG or BLIF of the network.
    * "-m" store resulting network as MIG


- lut_map
  
  Converts the stored network to an LUT network.  Reads from the stored AIG network by default.
    * "-m" read from MIG network instead
    * "-K INT" LUT size for mapping (default 6)
    * "-C INT" Max number of priority cuts (default 6)
    * "-o FILENAME" Write LUT mapping to bench file
  
  
- ps

  Print network statistics for:
    * "-a" the AIG network
    * "-m" the MIG network
    * "-x" the XAG network
    * "-k" the KLUT network
    * "--all" all stored networks
  
  
- disjoint_clouds
  
  Writes status of all disjoint combinational clouds in the stored AIG.
  

- get_all_partitions
  
  Exports every partition to a verilog file.  Must provide the directory to write to as a positional argument.
    * "-m" export partitions in the MIG network
  
  
- print_karnaugh
  
  Print all partitioned truth tables as karnaugh maps
  
  
- show_ntk
  
  Display details of the stored network.  AIG default.
    * "-m" show MIG network
  
  
- techmap
  
  Experimental ASIC mapper.  Not ready for general use; please contact developers if you would like a detailed usage guide.
  
  
- write_bench
  
  Write KLUT network to bench format
  
  
- write_blif
  
  Write KLUT network to blif format
  
  
- write_dot
  
  Writes AIG network to dot format for visualization.  Do not use except with very small networks.
  * "-m" to write MIG network
  
  
- write_hypergraph
  
  Output stored network in hypergraph format used by hMETIS, gMETIS, etc.  For use with an external partitioning tool. Takes filename to write to as a positional argument.
    * "-m" write stored MIG network (default AIG)
  
  
- write_verilog
  
  Writes stored AIG network into very simple structural verilog.  For robust verilog support, use Yosys plugin.
    * "-m" write MIG network
    * "-x" write XAG network
    * "--skip-feedthrough" exclude feedthrough nets in resulting verilog file
  
  
- crit_path_stats
  
  Determines the number of AND and MAJ3 nodes along the critical path in an MIG network.
  
  
- get_cones
  
  Displays size and depth of all logic cones in an AIG network
  
  
- ntk_stats
  
  Writes number of AND2 and MAJ3 nodes in stored MIG network
  
  
- partitioning
  
  Partition the AIG network.  Number of partitions is a positional argument.
    * "-m" partition MIG network
    * "-c" path to config file for KaHyPar
    * "-f" path to external partition file, if using an external partitioner.
  
  
- partition_detail
  
  Display all nodes in each partition.
    * "-m" use stored MIG
    * "-n" use internal net names (default is yes for AIG)
