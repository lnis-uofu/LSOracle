# Getting Started with LSOraclte

[![Build Status](https://travis-ci.org/LNIS-Projects/LSOracle.svg?branch=master)](https://travis-ci.org/LNIS-Projects/LSOracle)


## Introduction

The Logic Synthesis oracle is a framework developed on the top of EPFL logic synthesis libraries to unlock efficient logic manipulation by using different logic optimizers. To do so, the flow splits a design into different partitions and selects different optimizers for different logics attributes. Currently, it supports AIG and MIG based optimization. To make possible the use of sequential MIGs, sequential support has been added on this Boolean network. 

## How to compile: 

To compile, it is needed support to CMake 3.9 and C++ 17. For the tests, we have used cmake 3.11.2 and gcc/g++ 8.2
The current version has been tested on Red Hat 7.5 and MacOS Yosemite 10.10.  

1. git clone https://github.com/LNIS-Projects/LSOracle.git
2. mkdir build
3. cd build
4. cmake .. 
5. make 

## MIG Sequential Support Example: 

To run sequential MIG, please do as follows:

* ./lstools                                                //run the tool
* get_mig                                               //read a sequential AIG into a sequential AIG. Specify the file path when the command asks for. 
* migscript                                             //interleaves area and depth optimization in the same way as the ABC resyn2 optimization command. 
* write_verilog                                       //outputs the optimized network in Verilog format, that can be used to technology mapping or functional verification. 


