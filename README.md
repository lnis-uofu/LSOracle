# Getting Started with LSOracle

[![Build Status](https://travis-ci.org/LNIS-Projects/LSOracle.svg?branch=master)](https://travis-ci.org/LNIS-Projects/LSOracle)
[![Documentation Status](https://readthedocs.org/projects/lsoracle/badge/?version=latest)](https://lsoracle.readthedocs.io/en/latest/?badge=latest)

## Introduction

The Logic Synthesis oracle is a framework developed on the top of EPFL logic synthesis libraries to unlock efficient logic manipulation by using different logic optimizers.To do so, the flow splits a design into different partitions and selects different optimizers for different logics attributes.
Currently, it supports AIG and MIG based optimization. 
To make possible the use of sequential MIGs, sequential support has been added on this Boolean network. 

## How to compile: 

To compile, it is needed support to CMake 3.9 and C++ 17. For the tests, we have used cmake 3.11.2 and gcc/g++ 8.2
The current version has been tested on Red Hat 7.5 and MacOS Mojave 10.14.  

```{r, engine='bash', count_lines}
git clone https://github.com/LNIS-Projects/LSOracle.git
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=RELEASE 
make 
```

## MIG Sequential Support Example: 

To run sequential MIG, please do as follows:

```{r, engine='bash', count_lines}
./lstools                                                #run the tool
get_mig                                               #read a sequential AIG into a sequential AIG. Specify the file path when the command asks for. 
migscript                                             #interleaves area and depth optimization in the same way as the ABC resyn2 optimization command. 
write_verilog                                       #outputs the optimized network in Verilog format, that can be used to technology mapping or functional verification. 
```

## Full Documentation 

The LSOracle full documentation is available [here](https://lsoracle.readthedocs.io/en/latest/?badge=latest)

