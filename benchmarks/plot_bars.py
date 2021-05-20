import numpy as np
import matplotlib as mpl
import matplotlib.pyplot as plt
import matplotlib.ticker as mt
import sys

plt.rcParams['font.size'] = '9'
plt.style.use('tableau-colorblind10')
plt.rcParams['font.family'] = 'serif'
input_file = sys.argv[1]
output_file = sys.argv[2]

d = np.genfromtxt(input_file, delimiter='\t', names=True, dtype=None, encoding="UTF-8", filling_values=0, missing_values="")

colwidth = 21*400./2409
textwidth = 43*400./2409

# unopt = d[['circuit','unoptimized_nodes_percent','unoptimized_depth_percent','unoptimized_nodesdepth_percent','unoptimized_power_percent','unoptimized_delay_percent','unoptimized_area_percent','unoptimized_adp_percent']]
# abc = d[['circuit','abc_nodes_percent','abc_depth_percent','abc_nodesdepth_percent','abc_power_percent','abc_delay_percent','abc_area_percent','abc_adp_percent']]
# lsoracle = d[['circuit','lsoracle_nodes_percent','lsoracle_depth_percent','lsoracle_nodesdepth_percent','lsoracle_power_percent','lsoracle_delay_percent','lsoracle_area_percent','lsoracle_adp_percent']]

width = 0.30

fig, (nodes, depth, ndp, delay, adp, area) = plt.subplots(1,6)
prev_exists = d['prev_nodes']/d['prev_nodes']

nodes_d = np.dstack((1 - d['nodes'] / d['unopt_nodes'],
                     1 - d['abc_nodes'] / d['unopt_nodes'],
                     prev_exists * (1 - d['prev_nodes'] / d['unopt_nodes'])))[0]
depth_d = np.dstack((1 - d['depth'] / d['unopt_depth'],
                     1 - d['abc_depth'] / d['unopt_depth'],
                     prev_exists * (1 - d['prev_depth'] / d['unopt_depth'])))[0]

ndp_u = d['unopt_nodes'] * d['unopt_depth']
ndp_d = np.dstack((1 - d['nodes'] * d['depth'] / ndp_u,
                   1 - d['abc_nodes'] * d['abc_depth'] / ndp_u,
                   prev_exists * (1 - d['prev_nodes'] * d['prev_depth'] /ndp_u)))[0]
area_d = np.dstack((1 - d['area'] / d['unopt_area'],
                    1 - d['abc_area']/d['unopt_area'],
                    prev_exists * (1 - d['prev_area']/d['unopt_area'])))[0]
adp_u = d['unopt_arrival'] * d['unopt_area']
adp_d = np.dstack((1 - d['area'] * d['arrival'] / adp_u,
                   1 - d['abc_area'] * d['abc_arrival'] / adp_u,
                   prev_exists * (1 - d['prev_area'] * d['prev_arrival'] / adp_u)))[0]

delay_d = np.dstack((1 - d['arrival'] / d['unopt_arrival'],
                     1 - d['abc_arrival'] / d['unopt_arrival'],
                     prev_exists * (1 - d['prev_arrival'] / d['unopt_arrival'])))[0]

fig, ((nodes, depth, ndp), (area, delay, adp))  = plt.subplots(2,3)

for m, t, ax, da in [
        ("nodes", "Nodes", nodes, nodes_d),
        ("depth", "Depth", depth, depth_d),
        ("ndp", "Node Depth Product", ndp, ndp_d),
        ("area", "Area", area, area_d),
        ("delay", "Delay", delay, delay_d),
        ("adp", "Area Delay Product", adp, adp_d),
]:
    ax.margins(x=0.10)
    ax.set(title=t)
    print(np.flip(da))
    x = np.arange(len(da)+1)
    u = np.append(da[:,0], np.average(da[:,0]))
    a = np.append(da[:,1], np.average(da[:,1]))
    l = np.append(da[:,2], np.average(da[:,2]))

    labels = np.append(d["circuit"], "average")

    farm = ax.barh(x+width, u, width, label="Latest LSOracle")
    lsoracle = ax.barh(x, l, width, label="Previous LSOracle")
    abc = ax.barh(x-width, a, width, label="ABC")

    ax.set_xlabel("% improvement over original circuit")
    ax.set_yticks(x)
    ax.set_yticklabels(labels)

    ax.bar_label(farm, [f"{f:.0%}" for f in u], padding=3, fontsize='x-small')
    ax.bar_label(abc, [f"{f:.0%}" for f in a], padding=3, fontsize='x-small')
    ax.bar_label(lsoracle, [f"{f:.0%}" for f in l], padding=3, fontsize='x-small')

    ax.axvline(color="black")
    ax.xaxis.set_major_formatter("{x:.0%}")
ax.legend()
fig.set_figheight(8)
fig.set_figwidth(12)

fig.tight_layout()
fig.savefig(output_file)
