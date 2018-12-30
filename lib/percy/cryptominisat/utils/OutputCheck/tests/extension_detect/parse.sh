#!/usr/bin/env bash
# RUN: sed 's/^#[ ]*CHECK.\+$//g' %s | %OutputCheck %s

# CHECK: hello
echo "hello"
