# Getting Started with LSOracle

[![Build Status](https://travis-ci.org/LNIS-Projects/LSOracle.svg?branch=master)](https://travis-ci.org/LNIS-Projects/LSOracle)
[![Documentation Status](https://readthedocs.org/projects/lsoracle/badge/?version=master)](https://lsoracle.readthedocs.io/en/master/?badge=master)

## Introduction

The Logic Synthesis oracle is a framework developed on the top of EPFL logic synthesis libraries to unlock efficient logic manipulation by using different logic optimizers.To do so, the flow splits a design into different partitions and selects different optimizers for different logics attributes.
Currently, it supports AIG, MIG, XAG, and XMG based optimization.

[![introductory video for the LSOracle tool](http://img.youtube.com/vi/3fPEUFlHnh8/0.jpg)](http://www.youtube.com/watch?v=3fPEUFlHnh8 "LSOracle overview")


## Installation:

LSOracle can be built on Linux and Mac OS.  Compilation requires CMake 3.12 or newer and gcc 9 or newer. Additional dependencies for the master branch are Boost Program_options 1.48.0 or newer and readline; dependencies for feature branches may vary.

### Linux
```{r, engine='bash', count_lines}
git clone --recursive https://github.com/LNIS-Projects/LSOracle.git
cd LSOracle
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=RELEASE -DYOSYS_PLUGIN=ON -DYOSYS_INCLUDE_DIR=/path/to/yosys
make
```

### Mac OS
Mac installation is similar to Linux. We recommend using [Homebrew](https://brew.sh) to install any missing dependencies.
```{r, engine='bash', count_lines}
git clone https://github.com/LNIS-Projects/LSOracle.git
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DYOSYS_PLUGIN=ON -DYOSYS_INCLUDE_DIR=/path/to/yosys
gmake
```
### Docker
LSOracle is available as a docker image on [Dockerhub](https://hub.docker.com/u/lnis).

## Usage

[![a video tutorial for LSOracle](http://img.youtube.com/vi/W8SO2rTiLUY/0.jpg)](http://www.youtube.com/watch?v=W8SO2rTiLUY "LSOracle Tutorial")

```{r, engine='bash', count_lines}
build/core/lsoracle               # run the tool
read my_file.blif                 # read a file in either blif or AIG format (determined by file extension)
oracle                            # partitions network and optimizes each partition with either AIG or MIG optimization recipes
write_verilog my_output.v         # outputs the optimized network in Verilog format
```
## Help

```{r, engine='bash', count_lines}
help				  # Gives a list of commands
[command] -h			  # Gives more information about the command.
```
## Full Documentation

The LSOracle documentation is available [here](https://lsoracle.readthedocs.io/en/master/?badge=master). An update is underway. In the interim, a usage guide is available [here](https://docs.google.com/document/d/14cd4-4V3kSqmgdqLF5ahuFzrQ7hKPcMZu0X6No0VqmE/edit?usp=sharing), it is oriented toward users targeting FPGAs, but much of the information is applicable to all users.

## Using LSOracle as a Yosys Plugin

LSOracle includes a plugin for Yosys. The plugin is built in build/yosys-plugin/oracle.so and may be copied into the share/plugin directory in the yosys build and used with the flag `-m oracle`, or can be specified by full path `-m LSOracle/build/yosys-plugin/oracle.so` when calling Yosys.

## Publications and Related Repositories
This [repository](https://github.com/wlneto/pathAnalyzer) contains a ML-based tool to predict post-PnR critical maps using pre-mapping features. Using this tool, we were able to improve ADP by up to 15.53% and PDP by up to 18.56% compared to the generic reference flow. For a detailed explanation of this work, please refer to the paper "Read your Circuit: Leveraging Word Embedding to Guide Logic Optimization", published in ASP-DAC 2021. This work is to be integrated with LSOracle when we implement robust TCL and DB infrastructures.

This [repository](https://github.com/lnis-uofu/ML-Mapper) contains a modified version of ABC, as well as the Python code used in the paper "SLAP: A Supervised Learning Approach for Priority Cuts Technology Mapping", published in DAC 2021. In that paper, we used a ML model to filter cuts for ASIC technology mapping and obtained a 10% improvement in delay over the default ABC heuristic for cut pruning. As we needed to modify ABC to interface with our Python structure, we placed it in its own repository. We plan to apply the same approach to the EPFL native standard cell mapper at a later date. In the interim, it could be implemented in Yosys with minimal effort.
