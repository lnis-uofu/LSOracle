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
    args = glob.glob("tigfet/*.report.json")
else:
    args = sys.argv[1:]

if "--previous" in args:
    split = args.index("--previous")
    current_files, previous_files = args[:split], sys.argv[split+1:]
else:
    current_files, previous_files = args[:], []

gf12 = "sc9mcpp84_12lp_base_rvt_c14_tt_nominal_max_0p70v_25c"
tigfet = "sclib_tigfet10_hpnw_all_tt_0p70v_25c"

def read_files(files):
    designs = dict()
    for f in files:
        with open(f) as g:
            d = json.load(g)
            design = d["design"]
            tech = d["liberty_file"]
            if "optimization" in d:
                optim = d["optimization"]["name"]
            else:
                optim = "unknown"
            if optim not in ["abc"] or not tech in [gf12, tigfet]:
                continue
            print(f)
            if design not in designs:
                designs[design] = {}
            if tech not in designs[design]:
                designs[design][tech] = {}
            designs[design][tech][optim] = d
    return designs
current = read_files(current_files)
previous = read_files(previous_files)

def e(run):
    power = run["power"]["total"]
    return (run["size"], run["depth"], run["area_estimate"]["total"], run["timing"]["data_arrival"], power["internal"], power["switching"], power["leakage"], power["total"])

base = "abc"
d = []
for design, techs in current.items():
    if not gf12 in techs:
        continue
    if not base in techs[gf12]:
        continue
    ref = e(techs[gf12][base])
    for tech, optims in techs.items():
        if tech != tigfet:
            continue
        for optim, run in optims.items():
            if optim != base:
                continue
            print(design, tech, optim)
            d.append((" ".join([design, optim]), design, tech, optim) + e(run) + ref)

d = np.array(d, dtype=[('label', '<U64'), ('design','<U64'), ('tech', '<U64'), ('optimization', '<U64'), ('nodes', 'i8'), ('level', 'i8'), ('area', 'f8'), ('arrival', 'f8'), ('power_internal', 'f8'), ('power_switching', 'f8'), ('power_leakage', 'f8'), ('power_total', 'f8'), ('ref_nodes', 'i8'), ('ref_level', 'i8'), ('ref_area', 'f8'), ('ref_arrival', 'f8'), ('ref_power_internal', 'f8'), ('ref_power_switching', 'f8'), ('ref_power_leakage', 'f8'), ('ref_power_total', 'f8')])
np.sort(d, order=["label"])
colwidth = 21*400./2409
textwidth = 43*400./2409

width = 0.30

fig, (nodes, depth, ndp, delay, adp, area) = plt.subplots(1,6)

nodes_d = d['nodes'] / d['ref_nodes']
depth_d = d['level'] / d['ref_level']
area_d = d['area'] / d['ref_area']
delay_d = d['arrival'] / d['ref_arrival']
ndp_d = (d['nodes'] * d['level']) / (d['ref_nodes'] * d['ref_level'])
adp_d = (d['area'] * d['arrival']) / (d['ref_arrival'] * d['ref_area'])

fig, ((nodes, depth, ndp), (area, delay, adp))  = plt.subplots(2,3)

for m, t, ax, db in [
        ("nodes", "Nodes", nodes, 1 - nodes_d),
        ("depth", "Depth", depth, 1 - depth_d),
        ("ndp", "Node Depth Product", ndp, 1 - ndp_d),
        ("area", "Area", area, 1 - area_d),
        ("delay", "Delay", delay, 1 - delay_d),
        ("adp", "Area Delay Product", adp, 1 - adp_d),
]:
    ax.margins(x=0.10)
    ax.set(title=t)
    x = np.arange(len(db))
    print(t)
    labels = d['label']
    bars = ax.barh(x+width, db, width, label="test")
    ax.bar_label(bars, [f"{f:.0%}" for f in db], padding=3, fontsize='x-small')
    ax.set_xlabel("% improvement over original circuit")
    ax.set_yticks(x)
    ax.set_yticklabels(labels)

    ax.axvline(color="black")
    ax.xaxis.set_major_formatter("{x:.0%}")
nodes.legend()
fig.set_figheight(20)
fig.set_figwidth(12)

fig.tight_layout()
fig.savefig("test.png")
