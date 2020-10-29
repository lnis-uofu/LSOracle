## LSOracle

LSOracle is an MIT licensed package for mixed logic synthesis, using multiple graph representations of a logic circuit simultaneously to achieve higher performance than is possible with a homogeneous approach.  It is available as a standalone tool and as a plugin for [Yosys](http://www.clifford.at/yosys/), a popular open-source synthesis tool.

### Getting Started

# Stand Alone
The main LSOracle repository is available [here](https://github.com/LNIS-Projects/LSOracle), and is where most development is done.  As a standalone tool, LSOracle has many options for customizing the synthesis performed, including homogeneous AIG, MIG, and XAG synthesis, and an adjustable mixed synthesis framework which can prioritize area or delay, merge adjacent partitions, use external partitioners, etc. It is intented primarily for researchers, as it lacks a verilog frontend, and the many options can be confusing for users who may just want to get a design implemented.
# Yosys Plugin
Most users will prefer to use LSOracle with the Yosys plugin, available [here](https://github.com/LNIS-Projects/LSOracle-Plugin).  Using LSOracle within Yosys adds a robust verilog frontend, and expands techmapping capabilities, allowing use of [abc's](https://github.com/berkeley-abc/abc) boolean matching capabilites for ASIC synthesis and more sophisticated FPGA techniques including mapping to carry chains, DSPs, etc, rather than only to LUTs.  It is also completely automatic.  The user simply calls the LSOracle pass and then continues using Yosys as if they had just called the "abc" command.

