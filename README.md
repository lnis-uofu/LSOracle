# Getting Started with LSOracle

[![Build Status](https://travis-ci.org/LNIS-Projects/LSOracle.svg?branch=master)](https://travis-ci.org/LNIS-Projects/LSOracle)
[![Documentation Status](https://readthedocs.org/projects/lsoracle/badge/?version=master)](https://lsoracle.readthedocs.io/en/master/?badge=master)

## Introduction

The Logic Synthesis oracle is a framework developed on the top of EPFL logic synthesis libraries to unlock efficient logic manipulation by using different logic optimizers.To do so, the flow splits a design into different partitions and selects different optimizers for different logics attributes.
Currently, it supports AIG and MIG based optimization. 


## Installation: 

LSOracle can be built on Linux and Mac OS 10.14.  Compilation requires CMake 3.12 or newer and gcc 8 or newer. Additional dependencies for the master branch are Boost Program_options 1.48.0 or newer and readline; dependencies for feature branches may vary.

### Linux
```{r, engine='bash', count_lines}
git clone https://github.com/LNIS-Projects/LSOracle.git
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=RELEASE 
make 
```

### Mac OS
Mac installation is slightly more involved than on Linux because gcc and Boost are typically not yet installed, with the gcc command symlinked to apple clang, which is unsupported.  We recommend using [Homebrew](https://brew.sh). First install gcc, then install Boost, using gcc to build boost@1.60, as gcc installation is broken on boost@latest as of December 2019.
```{r, engine='bash', count_lines}
brew install gcc
brew install --cc=gcc-9 boost@1.60

git clone https://github.com/LNIS-Projects/LSOracle.git
mkdir build
cd build
cmake .. -DCMAKE_CXX_COMPILER=/usr/local/bin/g++-9 -DCMAKE_CC_COMPILER=/usr/local/bin/gcc-9 -DCMAKE_BUILD_TYPE=Release
make 
```

## Usage

```{r, engine='bash', count_lines}
./lstools                         # run the tool
read my_file.blif                 # read a file in either blif or AIG format (determined by file extension)
oracle                            # partitions network and optimizes each partition with either AIG or MIG optimization recipes
write_verilog my_output.v         # outputs the optimized network in Verilog format
```

## Full Documentation 

The LSOracle full documentation is available [here](https://lsoracle.readthedocs.io/en/master/?badge=master)

