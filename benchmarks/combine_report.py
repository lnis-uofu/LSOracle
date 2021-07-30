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

dtype=[('circuit', '<U128'),
       ('inputs', '<i8'),
       ('outputs', '<i8'),
       ('unoptimized_nodes', '<i8'),
       ('unoptimized_level', '<i8'),
       ('lsoracle_partitions', '<i8'),
       ('lsoracle_aigs', '<i8'),
       ('lsoracle_migs', '<i8'),
       ('lsoracle_nodes', '<i8'),
       ('lsoracle_level', '<i8'),
       ('lsoracle_runtime', '<i8'),
       ('inputs_1', '<i8'),
       ('outputs_1', '<i8'),
       ('lsoracle_critical_maj', '<i8'),
       ('lsoracle_critical_and', '<i8'),
       ('lsoracle_maj_nodes_internal', '<i8'),
       ('lsoracle_and_nodes_internal', '<i8'),
       ('lsoracle_internal_power', '<f8'),
       ('lsoracle_switching_power', '<f8'),
       ('lsoracle_leakage_power', '<f8'),
       ('lsoracle_total_power', '<f8'),
       ('lsoracle_percent_power', '<f8'),
       ('lsoracle_arrival', '<f8'),
       ('lsoracle_worst_slack', '<f8'),
       ('lsoracle_area', '<f8'),
       ('abc_nodes', '<i8'),
       ('abc_level', '<i8'),
       ('abc_internal_power', '<f8'),
       ('abc_switching_power', '<f8'),
       ('abc_leakage_power', '<f8'),
       ('abc_total_power', '<f8'),
       ('abc_percent_power', '<f8'),
       ('abc_arrival', '<f8'),
       ('abc_worst_slack', '<f8'),
       ('abc_area', '<f8'),
       ('unoptimized_internal_power', '<f8'),
       ('unoptimized_switching_power', '<f8'),
       ('unoptimized_leakage_power', '<f8'),
       ('unoptimized_total_power', '<f8'),
       ('unoptimized_percent_power', '<f8'),
       ('unoptimized_arrival', '<f8'),
       ('unoptimized_worst_slack', '<f8'),
       ('unoptimized_area', '<f8'),
       ('migscript_nodes', '<i8'),
       ('migscript_level', '<i8'),
       ('migscript_runtime', '<i8'),
       ('migscript_critical_maj', '<i8'),
       ('migscript_critical_and', '<i8'),
       ('migscript_maj_nodes_internal', '<i8'),
       ('migscript_and_nodes_internal', '<i8'),
       ('migscript_internal_power', '<f8'),
       ('migscript_switching_power', '<f8'),
       ('migscript_leakage_power', '<f8'),
       ('migscript_total_power', '<f8'),
       ('migscript_percent_power', '<f8'),
       ('migscript_arrival', '<f8'),
       ('migscript_worst_slack', '<f8'),
       ('migscript_area', '<f8'),
       ('aigscript_nodes', '<i8'),
       ('aigscript_level', '<i8'),
       ('aigscript_runtime', '<i8'),
       ('aigscript_internal_power', '<f8'),
       ('aigscript_switching_power', '<f8'),
       ('aigscript_leakage_power', '<f8'),
       ('aigscript_total_power', '<f8'),
       ('aigscript_percent_power', '<f8'),
       ('aigscript_arrival', '<f8'),
       ('aigscript_worst_slack', '<f8'),
       ('aigscript_area', '<f8')
       ]

outfile, *files = sys.argv[1:]
d = np.hstack([np.genfromtxt(f, delimiter='\t', names=True, dtype=dtype, encoding="UTF-8") for f in files])
np.savetxt("everything.tsv", d, delimiter='\t', comments="", header='\t'.join(d.dtype.names), fmt="%s", encoding='utf-8')
