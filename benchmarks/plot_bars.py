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
import numpy as np
import matplotlib as mpl
import matplotlib.pyplot as plt
import matplotlib.ticker as mt
import sys
import os.path
import numpy.lib.recfunctions as rcf

plt.rcParams['font.size'] = '9'
plt.style.use('tableau-colorblind10')
plt.rcParams['font.family'] = 'serif'

input_file, prev_file, output_file = sys.argv[1:] if len(sys.argv) > 1 else ['everything.tsv', 'previous/everything.tsv', 'everything.png']

n = np.genfromtxt(input_file, delimiter=None, names=True, dtype=None, encoding="UTF-8")

prev_exists = os.path.isfile(prev_file)
if prev_exists:
    p = np.genfromtxt(prev_file, delimiter=None , names=True, dtype=n.dtype, encoding="UTF-8")
else:
    p = np.asarray([], dtype=n.dtype)

d = rcf.join_by("circuit", n, p, jointype='leftouter', r1postfix='', r2postfix='_prev')
np.sort(d, order=["unoptimized_nodes"])

colwidth = 21*400./2409
textwidth = 43*400./2409

width = 0.10

fig, (nodes, depth, ndp, delay, adp, area) = plt.subplots(1,6)

runs = [("lsoracle", ""), ("abc", ""), ("aigscript", ""), ("migscript", ""), ("lsoracle", "_prev")]
graph_cols = ['nodes', 'level', 'area', 'arrival']

nodes_d, depth_d, area_d, delay_d = np.array([[
    d[run + '_' + col + suff]
    for run, suff in runs] for col in graph_cols])

ndp_u = d['unoptimized_nodes'] * d['unoptimized_level']
ndp_d = nodes_d * depth_d / ndp_u
adp_u = d['unoptimized_arrival'] * d['unoptimized_area']
adp_d = area_d * delay_d / adp_u

fig, ((nodes, depth, ndp), (area, delay, adp))  = plt.subplots(2,3)

for m, t, ax, db in [
        ("nodes", "Nodes", nodes, nodes_d/d['unoptimized_nodes']),
        ("depth", "Depth", depth, depth_d/d['unoptimized_level']),
        ("ndp", "Node Depth Product", ndp, ndp_d),
        ("area", "Area", area, area_d/d['unoptimized_area']),
        ("delay", "Delay", delay, delay_d/d['unoptimized_arrival']),
        ("adp", "Area Delay Product", adp, adp_d),
]:
    da = np.transpose(1 - db) * [1,1,1,1, 1 if prev_exists else 0]
    print(da)
    ax.margins(x=0.10)
    ax.set(title=t)
    x = np.arange(len(da)+1)

    labels = np.append(d["circuit"], "average")

    def bar(off, i, label):
        u = np.append(da[:,i], np.average(da[:,i]))
        bars = ax.barh(x+off*width, u, width, label=label)
        ax.bar_label(bars, [f"{f:.0%}" for f in u], padding=3, fontsize='x-small')

    bar(2, 0, "lsoracle")
    bar(1, 1, "aigscript")
    bar(0, 2, "migscript")
    bar(-1, 3, "abc")
    bar(-2, 4, "previous lsoracle")

    ax.set_xlabel("% improvement over original circuit")
    ax.set_yticks(x)
    ax.set_yticklabels(labels)

    ax.axvline(color="black")
    ax.xaxis.set_major_formatter("{x:.0%}")
nodes.legend()
fig.set_figheight(20)
fig.set_figwidth(12)

fig.tight_layout()
fig.savefig(output_file)
