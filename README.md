# Using LSOracle as a Plugin for Yosys

## Introduction
Yosys is a framework for RTL synthesis tools. This plugin allows you to utilize features of the LSOracle tool within Yosys with the "lsoracle" command. This command is run in a similar way to the "abc" command

## Instructions
In order to use this plugin, you first need to build both LSOracle and Yosys. Instructions for this can be found in their respective repositories. You will then clone and make this branch making sure that you update the directory of Yosys in the Makefile to wherever you build Yosys. The result of this Makefile will be oracle.so. This can be used as a plugin to Yosys by using the command "./yosys -m oracle.so" or by using the plugin command within Yosys itself.


