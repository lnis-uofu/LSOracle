# Getting Started with LSOracle

[![Build Status](https://travis-ci.org/LNIS-Projects/LSOracle.svg?branch=master)](https://travis-ci.org/LNIS-Projects/LSOracle)
[![Documentation Status](https://readthedocs.org/projects/lsoracle/badge/?version=master)](https://lsoracle.readthedocs.io/en/master/?badge=master)

## Introduction

The Logic Synthesis oracle is a framework developed on the top of EPFL logic synthesis libraries to unlock efficient logic manipulation by using different logic optimizers.To do so, the flow splits a design into different partitions and selects different optimizers for different logics attributes.
Currently, it supports AIG and MIG based optimization.

[![introductory video for the LSOracle tool](http://img.youtube.com/vi/3fPEUFlHnh8/0.jpg)](http://www.youtube.com/watch?v=3fPEUFlHnh8 "LSOracle overview")


## Installation:

LSOracle can be built on Linux and Mac OS 10.14.  Compilation requires CMake 3.12 or newer and gcc 8 or newer. Additional dependencies for the master branch are Boost Program_options 1.48.0 or newer and readline; dependencies for feature branches may vary.

### Linux
```{r, engine='bash', count_lines}
git clone --recursive https://github.com/LNIS-Projects/LSOracle.git
cd LSOracle
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

The LSOracle full documentation is available [here](https://lsoracle.readthedocs.io/en/master/?badge=master)

## Using LSOracle as a Yosys Plugin

LSOracle is available as a plugin to yosys. To build the plugin, pass the arguments `-D YOSYS_PLUGIN=ON -D YOSYS_INCLUDE_DIR=/path/to/yosys` during cmake configuration, specifying the absolute path to the yosys directory. The plugin shared object will be compile to build/yosys-plugin/oracle.so. The plugin can be copied into the share/plugin directory in the yosys build and used with the flag `-m oracle`, or can be specified by full path `-m LSOracle/build/yosys-plugin/oracle.so`.
