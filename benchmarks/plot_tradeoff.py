import matplotlib.pyplot as plt
import numpy as np
import os
import numpy.lib.recfunctions as rcf

plt.rcParams['font.size'] = '9'
plt.rcParams['font.family'] = 'serif'
plt.style.use('tableau-colorblind10')
colwidth = 21*400./2409
textwidth = 43*400./2409

input_file, prev_file = sys.argv[1:] if len(sys.argv) > 1 else ("everything.tsv", "previous/everything.tsv")

n = np.genfromtxt(input_file, delimiter='\t', names=True, dtype=None, encoding="UTF-8")
if prev_exists:
    p = np.genfromtxt(prev_file, delimiter='\t', names=True, dtype=None, encoding="UTF-8")
else:
    p = np.full_like(d, 0, dtype=d.dtype)
prev_exists = 1 if os.path.isfile(prev_file) else np.nan

d = rcf.join_by("circuit", n, p, jointype='leftouter', r1postfix='', r2postfix='_prev')
np.sort(d, order=["unopt_nodes"])

lso_d, lso_a = d['arrival_prev'], d['area_prev']
farm_d, farm_a = d['arrival'], d['area']
abc_d, abc_a = d['abc_arrival'], d['abc_area']
unopt_d, unopt_a = d['unopt_arrival'], d['unopt_area']

lso_p, lso_n = d['mig_levels_prev'], d['mig_nodes_prev']
farm_p, farm_n = d['mig_levels'], d['mig_nodes']
abc_p, abc_n = d['abc_aig_levels'], d['abc_aig_nodes']
unopt_p, unopt_n = d['unopt_aig_levels'], d['unopt_aig_nodes']

margin = 0.02
quiver_args = {"angles":'xy', "scale_units":'xy', "scale":1., "width":0.005, "headwidth":5, "headlength":8}

def pct_improved(ax, new_d, new_a, old_d, old_a, q_limits, r_limits):
    u = 1 - new_d/old_d
    v = 1 - new_a/old_a

    ax.set( ylabel="area % improved", xlabel="delay % improved")
    ax.axvline(linewidth=0.1, color='black')
    ax.axhline(linewidth=0.1, color='black')
    ax.quiver(np.zeros(u.shape), np.zeros(v.shape), u, v, **quiver_args)
    ax.set_xlim(q_limits)
    ax.set_ylim(r_limits)
    ax.xaxis.set_major_formatter("{x:.0%}")
    ax.yaxis.set_major_formatter("{x:.0%}")
    ax.set_box_aspect(1)

def literal_improved(ax, new_d, new_a, old_d, old_a, q_limits, r_limits):
    s = old_d - new_d
    t = old_a - new_a
    ax.set(ylabel='Area $\mu m^2$', xlabel='Delay $ns$')
    ax.set_box_aspect(1)
    ax.axvline(linewidth=0.1, color='black')
    ax.axhline(linewidth=0.1, color='black')
    ax.quiver(np.zeros(s.shape), np.zeros(t.shape), s, t, **quiver_args)
    ax.set_xlim(q_limits)
    ax.set_ylim(r_limits)

def absolute_position(ax, new_d, new_a, new_label, old_d, old_a, old_label):
    ax.set(ylabel='Area $\mu m^2$', xlabel='Delay $ns$')
    ax.scatter(new_d, new_a, label=new_label)
    ax.scatter(old_d, old_a, label=old_label)
    ax.quiver(old_d, old_a, new_d - old_d, new_a - old_a, **quiver_args)

def absolute_position_triple(ax, new_d, new_a, new_label, old_d, old_a, old_label, orig_d, orig_a, orig_label):
    ax.set(ylabel='Area $\mu m^2$', xlabel='Delay $ns$')
    ax.scatter(new_d, new_a, label=new_label)
    ax.scatter(old_d, old_a, label=old_label)
    ax.scatter(orig_d, orig_a, label=orig_label)
    ax.quiver(old_d, old_a, new_d - old_d, new_a - old_a, **quiver_args)
    ax.quiver(orig_d, orig_a, new_d - orig_d, new_a - orig_a, **quiver_args)
    ax.quiver(orig_d, orig_a, old_d - orig_d, old_a - orig_a, **quiver_args)

def limits_sub(a, b, *ps):
    x = []
    y = []
    for c, d in ps:
        x.append(c - a)
        y.append(d - b)
    q_limits = (min(np.hstack(x))*1.05, max(np.hstack(x))*1.05)
    r_limits = (min(np.hstack(y))*1.05, max(np.hstack(y))*1.05)
    return (q_limits, r_limits)

def limits_pct(a, b, *ps):
    x = []
    y = []
    for c, d in ps:
        x.append(1 - c/a)
        y.append(1 - d/b)
    q_limits = (min(np.hstack(x)) + margin, max(np.hstack(x)) + margin)
    r_limits = (min(np.hstack(y)) + margin, max(np.hstack(y)) + margin)
    return (q_limits, r_limits)

def limits_pct_inv(a, b, *ps):
    x = []
    y = []
    for c, d in ps:
        x.append(1 - a/c)
        y.append(1 - b/d)
    q_limits = (min(np.hstack(x)) + margin, max(np.hstack(x)) + margin)
    r_limits = (min(np.hstack(y)) + margin, max(np.hstack(y)) + margin)
    return (q_limits, r_limits)


######################## Absolute change ########################
fig, (ax, ay) = plt.subplots(1,2)
fig.suptitle("area delay tradeoff")

ax.set(title="Current vs Previous LSOracle")
if prev_exists:
    absolute_position_triple(ax, farm_d, farm_a, "Current", lso_d, lso_a, "Previous", unopt_d, unopt_a, "Unoptimized")

ay.set(title="Current vs Previous LSOracle")
absolute_position_triple(ay, farm_d, farm_a, "Current", abc_d, abc_a, "ABC", unopt_d, unopt_a, "Unoptimized")

fig.set_figheight(textwidth)
fig.set_figwidth(2*textwidth)
fig.tight_layout()
fig.savefig("tradeoff_absolute.pdf", format="pdf")
fig.savefig("tradeoff_absolute.png", format="png")


######################## Literal improvement over unoptimized ########################
fig, (az, ax, aw) = plt.subplots(3,1)

if prev_exists:
    q_limits, r_limits = limits_sub(unopt_d, unopt_a, (abc_d, abc_a), (lso_d, lso_a), (farm_d, farm_a))
else:
    q_limits, r_limits = limits_sub(unopt_d, unopt_a, (abc_d, abc_a), (farm_d, farm_a))

az.set(title="ABC")
literal_improved(az, abc_d, abc_a, unopt_d, unopt_a, q_limits, r_limits)

ax.set(title="Current LSOracle")
literal_improved(az, farm_d, farm_a, unopt_d, unopt_a, q_limits, r_limits)

aw.set(title="Previous LSOracle")
if prev_exists:
    literal_improved(aw, lso_d, lso_a, unopt_d, unopt_a, q_limits, r_limits)

fig.set_figheight(3*colwidth)
fig.set_figwidth(colwidth)
fig.tight_layout()
fig.savefig("tradeoff_unopt_literal.pdf", format="pdf")
fig.savefig("tradeoff_unopt_literal.png", format="png")

######################## Percent improvement over unoptimized ########################
fig, (az, ax, aw) = plt.subplots(3,1)

if prev_exists:
    q_limits, r_limits = limits_sub(unopt_d, unopt_a, (abc_d, abc_a), (lso_d, lso_a), (farm_d, farm_a))
else:
    q_limits, r_limits = limits_sub(unopt_d, unopt_a, (abc_d, abc_a), (farm_d, farm_a))

az.set(title="ABC")
pct_improved(az, abc_d, abc_a, unopt_d, unopt_a, q_limits, r_limits)

ax.set(title="Current LSO")
pct_improved(ax, farm_d, farm_a, unopt_d, unopt_a, q_limits, r_limits)

aw.set(title="Previous LSO")
if prev_exists:
    pct_improved(aw, lso_d, lso_a, unopt_d, unopt_a, q_limits, r_limits)

fig.set_figheight(3*colwidth)
fig.set_figwidth(colwidth)
fig.tight_layout()
fig.savefig("tradeoff_unopt_percent.pdf", format="pdf")
fig.savefig("tradeoff_unopt_percent.png", format="png")

######################## Literal improvement
fig, (az, aw) = plt.subplots(2,1)

if prev_exists:
    q_limits, r_limits = limits_sub(farm_d, farm_a, (abc_d, abc_a), (lso_d, lso_a))
else:
    q_limits, r_limits = limits_sub(farm_d, farm_a, (abc_d, abc_a))

az.set(title="ABC")
literal_improved(az, farm_d, farm_a, abc_d, abc_a, q_limits, r_limits)

aw.set(title="Previous LSOracle")
if prev_exists:
    literal_improved(aw, farm_d, farm_a, lso_d, lso_a, q_limits, r_limits)

fig.set_figheight(2*colwidth)
fig.set_figwidth(colwidth)
fig.tight_layout()
fig.savefig("tradeoff_improved_literal.pdf", format="pdf")
fig.savefig("tradeoff_improved_literal.png", format="png")


######################## Percent improvement ########################
fig, (az, aw) = plt.subplots(2,1)

if prev_exists:
    q_limits, r_limits = limits_pct_inv(farm_d, farm_a, (abc_d, abc_a), (lso_d, lso_a))
else:
    q_limits, r_limits = limits_pct_inv(farm_d, farm_a, (abc_d, abc_a))
az.set(title="ABC")
pct_improved(az, farm_d, farm_a, abc_d, abc_a, q_limits, r_limits)

aw.set(title="previous LSOracle")
if prev_exists:
    pct_improved(aw, farm_d, farm_a, lso_d, lso_a, q_limits, r_limits)

fig.set_figheight(2*colwidth)
fig.set_figwidth(colwidth)
fig.tight_layout()
fig.savefig("tradeoff_improved_percent.pdf", format="pdf")
fig.savefig("tradeoff_improved_percent.png", format="png")

######################## Tech independent percent improvement ########################
fig, (az, aw) = plt.subplots(2,1)

if prev_exists:
    q_limits, r_limits = limits_pct_inv(farm_p, farm_n, (abc_p, abc_n), (lso_p, lso_n))
else:
    q_limits, r_limits = limits_pct_inv(farm_p, farm_n, (abc_p, abc_n))

az.set(title="ABC", ylabel="nodes % improved", xlabel="depth % improved")
pct_improved(az, farm_p, farm_n, abc_p, abc_n, q_limits, r_limits)


aw.set(title="Previous LSOracle", ylabel="nodes % improved", xlabel="depth % improved")
if prev_exists:
    pct_improved(aw, farm_p, farm_n, lso_p, lso_n, q_limits, r_limits)

fig.set_figheight(2*colwidth)
fig.set_figwidth(colwidth)
fig.tight_layout()
fig.savefig("tradeoff_tech_indep_improved_percent.pdf", format="pdf")
fig.savefig("tradeoff_tech_indep_improved_percent.png", format="png")
