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
import json
import glob

plt.rcParams['font.size'] = '9'
plt.style.use('tableau-colorblind10')
plt.rcParams['font.family'] = 'serif'

if len(sys.argv) == 1:
    args = glob.glob("**/*.report.json")
    output = "test.pdf"
elif len(sys.argv) == 2:
    args = glob.glob("**/*.report.json")
    output = sys.argv[1]
else:
    args = sys.argv[2:]
    output = sys.argv[1]

if "--previous" in args:
    split = args.index("--previous")
    current_files, previous_files = args[:split], sys.argv[split+1:]
else:
    current_files, previous_files = args[:], []

def read_files(files):
    designs = dict()
    for f in files:
        with open(f) as g:
            print(f)
            try:
                d = json.load(g)
            except:
                print ("failed", f)
                continue
            design = d["design"]
            tech = d["liberty_file"]
            if "optimization" in d:
                optim = d["optimization"]["name"]
else:
                optim = "unknown"
            if design not in designs:
                designs[design] = {}
            if tech not in designs[design]:
                designs[design][tech] = {}
            designs[design][tech][optim] = d
    return designs
current = read_files(current_files)
previous = read_files(previous_files)

def extract(run):
    power = run["power"]["total"]
    return (run["size"], run["depth"], run["area_estimate"]["total"], run["timing"]["data_arrival"], power["internal"], power["switching"], power["leakage"], power["total"])

data = []
reference = "unoptimized"
for design, techs in current.items():
    for tech, optims in techs.items():
        if reference not in optims:
            print("missing reference for", design, tech)
            continue
        ref = extract(optims[reference])
        for optim, run in optims.items():
            if optim == reference:
                continue
            print(design, tech, optim)
            data.append((" ".join([design, optim]), design, tech, optim) + extract(run) + ref)

data = np.array(data, dtype=[('label', '<U64'),
                             ('design','<U64'),
                             ('tech', '<U64'),
                             ('optimization', '<U64'),
                             ('nodes', 'i8'),
                             ('level', 'i8'),
                             ('area', 'f8'),
                             ('arrival', 'f8'),
                             ('power_internal', 'f8'),
                             ('power_switching', 'f8'),
                             ('power_leakage', 'f8'),
                             ('power_total', 'f8'),
                             ('ref_nodes', 'i8'),
                             ('ref_level', 'i8'),
                             ('ref_area', 'f8'),
                             ('ref_arrival', 'f8'),
                             ('ref_power_internal', 'f8'),
                             ('ref_power_switching', 'f8'),
                             ('ref_power_leakage', 'f8'),
                             ('ref_power_total', 'f8')])

data.sort(order=["design", "tech"])

# Sizes for IEEE latex templates.
colwidth = 21*400./2409
textwidth = 43*400./2409

fig, (nodes, depth, ndp, delay, adp, area) = plt.subplots(1,6)

nodes_d = data['nodes'] / data['ref_nodes']
depth_d = data['level'] / data['ref_level']
area_d = data['area'] / data['ref_area']
delay_d = data['arrival'] / data['ref_arrival']
ndp_d = (data['nodes'] * data['level']) / (data['ref_nodes'] * data['ref_level'])
adp_d = (data['area'] * data['arrival']) / (data['ref_arrival'] * data['ref_area'])

fig, ((nodes, depth, ndp), (area, delay, adp))  = plt.subplots(2,3)

width = 0.9
for graph, title, ax, db, label in [
        ("nodes", "Nodes", nodes, nodes_d, True),
        ("depth", "Depth", depth, depth_d, False),
        ("ndp", "Node Depth Product", ndp, ndp_d, False),
        ("area", "Area", area, area_d, True),
        ("delay", "Delay", delay, delay_d, False),
        ("adp", "Area Delay Product", adp, adp_d, False),
]:
    print(title)
    ax.margins(x=0.10)
    ax.set(title=title)
    designs = np.unique(data['design']).tolist()
    y = np.arange(len(db),0,-1) + len(designs) - np.array([designs.index(design) for design in data['design']])
    colormap = plt.rcParams['axes.prop_cycle'].by_key()['color']
    colors = [colormap[designs.index(design) % len(colormap)] for design in data['design']]
    labels = data['label']
    bars = ax.barh(y, db, width, color=colors)
    ax.bar_label(bars, [f"{f:.2f}x" for f in db], padding=3, fontsize='x-small')
    if label:
    ax.set_xlabel("% improvement over original circuit")
        ax.set_yticks(y)
    ax.set_yticklabels(labels)
    else:
        ax.set_yticks([])
        ax.set_yticklabels([])
    ax.axvline(1, color="black")
    ax.xaxis.set_major_formatter("{x:.1f}")
fig.set_figheight(20)
fig.set_figwidth(12)

fig.tight_layout()
fig.savefig(output)
