import networkx as nx
from graph_extraction import *
import kahypar
import sys
import math
lso_script = """read_aig {circuit}.aig; partitioning {partitions} -f {prefix}.KaHyPar; optimization; write_verilog -m {prefix}.v;
"""

prefix_format = "{circuit}.hpg.part{partitions}.epsilon{epsilon}.seed-1"

hpg_file, output_file = sys.argv[1:]
e = 0.9

print("reading hpg {}".format(hpg_file))
hpg_node_count, hpg_net_count, g = extract_hpg(hpg_file)
#remove_disconnected(g)

# divisions = [math.floor(hpg_node_count/2048)]
# print("Generated partition sizes {}".format(", ".join([str(d) for d in divisions])))

print("generating weights")
nodes = [*g.nodes]
weights = {i: 0 for i in g.nodes}
outputs = find_outputs(g)

for output in outputs:
    if not output in g.nodes:
        print("WARNING missing node {}".format(output))
        continue
    cone = nx.ancestors(g, output)
    for node in cone:
        weights[node] += 1

def extract_list(hpg_path):
    with open(hpg_path) as hpg_file:
        hpg = hpg_file.readlines()
        parsed = [[int(x) for x in n.strip().split(' ')] for n in hpg]
        num_nets, num_nodes = parsed[0]
        return (num_nets, num_nodes, parsed[1:])

num_nets, num_nodes, hpg = extract_list(hpg_file)

hyperedge_indices = [0]
hyperedges = []
for connections in hpg:
    hyperedges.extend(connections)
    hyperedge_indices.append(len(hyperedges))

print("{} nets {} nodes".format(num_nets, num_nodes))
node_weights = []#[1 for _ in range(0, num_nodes)]
max_weight = max(weights.values())
print("max {} min {}".format(max(weights.values()), min(weights.values())))

edge_weights = [1 + math.ceil(10 * weights[i]/max_weight) for i, *_ in hpg]
hist = {i:0 for i in range(1, 12)}
for i in edge_weights:
    hist[i] += 1

print(hist)
with open(output_file, 'w') as f:
    f.write("\n".join([str(i) for i in edge_weights]))

# for partitions in divisions:
#     print("partitioning {}".format(partitions))
#     print(hpg_node_count, hpg_net_count, partitions, len(hyperedge_indices), len(hyperedges), len(edge_weights), len(node_weights))
#     hypergraph = kahypar.Hypergraph(hpg_node_count, hpg_net_count, hyperedge_indices, hyperedges, partitions, edge_weights, node_weights)
#     print("created hypergraph object")
#     context = kahypar.Context()
#     context.loadINIconfiguration("km1_kKaHyPar_sea20.ini")
#     context.setK(partitions)
#     context.setEpsilon(e)
#     print("built context")
#     prefix = prefix_format.format(circuit=circuit_path, epsilon=e, partitions=partitions)
#     print("doing the thing")
#     kahypar.partition(hypergraph, context)
#     # orig_stdout = sys.stdout
#     # with open(prefix + ".part.out", 'w') as part_out: #kahypar dumps to stdout here, redirecting to partition output
#     #     sys.stdout = part_out
#     print("did the thing")
#     with open(prefix + ".KaHyPar.temp", 'w') as f:
#         f.write("\n".join([str(hypergraph.blockID(n)) for n in hypergraph.nodes()]))
#     with open(prefix + ".sh", 'w') as f:
#         f.write("cp {prefix}.KaHyPar.temp {prefix}.KaHyPar\n".format(prefix=prefix))
#     with open(prefix + '.lsoracle', 'w') as lsoracle_file:
#         fixed = ",".join([str(i) for i in range(0, len(groups))])
#         lsoracle_file.write(lso_script.format(prefix=prefix, circuit=circuit_path, fixed=fixed, partitions=partitions))
#     # sys.stdout = orig_stdout
