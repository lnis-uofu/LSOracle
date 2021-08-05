# LSOracle: A learning based Oracle for Logic Synthesis
# MIT License
# Copyright 2019 Laboratory for Nano Integrated Systems (LNIS)
#
# Permission is hereby granted, free of charge, to any person
# obtaining a copy of this software and associated documentation
# files (the "Software"), to deal in the Software without
# restriction, including without limitation the rights to use,
# copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following
# conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
# OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
# HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.
import sys
import numpy as np

header_string = "circuit    inputs    outputs    unoptimized_nodes    unoptimized_level    lsoracle_aigs    lsoracle_migs    lsoracle_nodes	lsoracle_level	lsoracle_runtime	inputs_1	outputs_1	lsoracle_critical_maj	lsoracle_critical_and	lsoracle_maj_nodes_internal	lsoracle_and_nodes_internal	lsoracle_internal_power	lsoracle_switching_power	lsoracle_leakage_power	lsoracle_total_power	lsoracle_percent_power	lsoracle_arrival	lsoracle_worst_slack	lsoracle_area	abc_nodes	abc_level	abc_internal_power	abc_switching_power	abc_leakage_power	abc_total_power	abc_percent_power	abc_arrival	abc_worst_slack	abc_area	unoptimized_internal_power	unoptimized_switching_power	unoptimized_leakage_power	unoptimized_total_power	unoptimized_percent_power	unoptimized_arrival	unoptimized_worst_slack	unoptimized_area	migscript_nodes	migscript_level	migscript_runtime	migscript_critical_maj	migscript_critical_and	migscript_maj_nodes_internal	migscript_and_nodes_internal	migscript_internal_power	migscript_switching_power	migscript_leakage_power	migscript_total_power	migscript_percent_power	migscript_arrival	migscript_worst_slack	migscript_area	aigscript_nodes	aigscript_level	aigscript_runtime	aigscript_internal_power	aigscript_switching_power	aigscript_leakage_power	aigscript_total_power	aigscript_percent_power	aigscript_arrival	aigscript_worst_slack	aigscript_area"
files = sys.argv[1:]
print(files)
everything = np.zeros(shape=(0, 68))
for f in files:
       print(f)
       line = np.genfromtxt(f, delimiter=None, names=None, dtype=None, encoding="UTF-8", usecols=1)
       everything = np.vstack((everything, line))
       print(everything)
#d = np.hstack([np.genfromtxt(f, delimiter=None, names=None, dtype=None, encoding="UTF-8").T for f in files])
#print(d)
np.savetxt("everything.tsv", everything, delimiter='\t', comments="", header=header_string, fmt="%s", encoding='utf-8')
