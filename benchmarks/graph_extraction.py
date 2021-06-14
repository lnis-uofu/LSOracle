import math
import networkx as nx
from collections import deque
def extract(hpg_path, outputs):
    with open(hpg_path) as hpg_file:
        next(hpg_file) # first line contains size
        hpg = hpg_file.readlines()
        G = nx.DiGraph()
        for n in hpg:
            node, *connections = [int(x) for x in n.strip().split(' ')]
            G.add_node(node)
            if node in outputs: # there's something special about output nodes, they reference back to their parent.
                continue
            for c in connections:
                # Nodes are read in topological order, this will generate
                # the correct direction even though the hpg file is undirected
                G.add_edge(node, c, weight=1)
        return G

def write_fixed(filename, size, parts):
    with open(filename, 'w') as output:
        fixed = [-1 for _ in range(0, size)]
        for idx, nodes in enumerate(parts):
            for node in nodes:
                fixed[node-1] = idx
        output.write("\n".join([str(i) for i in fixed]))
    print("wrote {} partitions".format(len(parts)))
    print("partition sizes {}".format(", ".join([str(len(p)) for p in parts])))
    max_part = max([len(p) for p in parts])
    print("max partition size {}".format(max_part))

def write_fixed_single(filename, size, parts):
    with open(filename, 'w') as output:
        fixed = [-1 for _ in range(0, size)]
        for nodes in parts:
            for node in nodes:
                fixed[node-1] = 0
        output.write("\n".join([str(i) for i in fixed]))
    print("wrote 1 partitions")
    max_part = sum([len(p) for p in parts])
    print("max partition size {}".format(max_part))

def fix_longest_path(gr, num):
    size = len(gr.nodes)
    g = gr.copy()
    paths = []
    for i in range(0, num):
        lp = nx.dag_longest_path(g)
        paths.append(lp)
        g.remove_nodes_from(lp)
        print(len(lp), len(g.nodes), len(g.edges))
    return paths

def split_clouds(g, outputs):
    u = g.to_undirected()
    clouds = []
    for i in outputs:
        if i in u:
            t = nx.bfs_tree(u, i)
            clouds.append(t)
            for n in [*t.nodes]:
                u.remove_node(n)
    return clouds

def fix_cones(gr, outputs, num):
    g = gr.copy()
    cones = []
    for i in range(0, num):
        lp = nx.dag_longest_path(g)
        output = lp[-1]
        if not output in outputs:
            print("WARN not in outputs {}".format(output))
        cone = nx.ancestors(g, output)
        cones.append(cone)
        g.remove_nodes_from(cone)
        print(len(lp), len(cone), len(g.nodes), len(g.edges))
    return cones

def fix_cones_thresh(gr, outputs, thresh):
    g = gr.copy()
    cones = []
    while nx.dag_longest_path_length(g) >= thresh:
        lp = nx.dag_longest_path(g)
        output = lp[-1]
        if not output in outputs:
            print("WARN not in outputs {}".format(output))
        cone = nx.ancestors(g, output)
        cones.append([*cone])
        g.remove_nodes_from(cone)
        print(len(lp), len(cone), len(g.nodes), len(g.edges))
    return cones

def group_parts(parts, size):
    grouped = []
    group = 0
    staging = []
    for part in parts:
        if (group + len(part)) > size:
            if group != 0:
                grouped.append(staging)
            group = len(part)
            staging = [*part]
            if len(part) > size:
                print("WARN cone {} is {} greater than requested size.".format(group, group - size))
        else:
            group += len(part)
            staging.extend(part)
    grouped.append(staging)
    return grouped

def extract_hpg(hpg_file):
    with open(hpg_file) as hpg:
        g = nx.DiGraph()
        number_nets, number_nodes = [int(i) for i in hpg.readline().split(' ')]
        nets = [[int(i) for i in line.strip().split(' ')] for line in hpg.readlines()]
        assert(number_nets == len(nets))
        all_node_refs = set()
        for net in nets:
            for node in net:
                all_node_refs.add(node)
        nodes = sorted(list(all_node_refs))
        for node in range(1, number_nodes + 1):
            g.add_node(node)
        cycles = []
        for head, *tail in nets:
            for child in tail:
                if child in g and head in g[child]:
                    if head < child:
                        print("warn, got a non-topological output cycle {} {}".format(head, child))
                    cycles.append((head, child))
                else:
                    g.add_edge(head, child)
        disconnected = number_nodes - len(nodes)
        if cycles:
            print("warn {} cycles found".format(len(cycles)))
        if disconnected > 0:
            print("warn {} non-connected nodes".format(disconnected))
        return (number_nodes, number_nets, g)

def mark_depth(graph, inputs):
    h = graph.copy()
    nodes = deque([(0, i) for i in inputs])
    while nodes:
        depth, node = nodes.popleft()
        graph.nodes[node]["depth"] = depth
        children = [*h.successors(node)]
        h.remove_node(node)
        for child in children:
            if len([*h.predecessors(child)]) == 0:
                nodes.append((depth+1, child))

def mark_partition(graph, partitions):
    for i, p in enumerate(partitions):
        if i in graph.nodes:
            graph.nodes[i]["partition"] = p

def read_partitions(part_file):
    with open(part_file) as f:
        return [int(l.strip()) for l in f.readlines()]

def find_outputs(g):
    return {n for n in g.nodes if len([*g.successors(n)]) == 0}

def find_inputs(g):
    return {n for n in g.nodes if len([*g.predecessors(n)]) == 0}

def remove_disconnected(g):
    disconnected = [n for n in g.nodes if len([*g.successors(n)]) == 0 and len([*g.predecessors(n)])]
    for d in disconnected:
        g.remove_node(d)

def calc_divisions(size):
    return sorted([i for i in {math.floor(size/nodes) for nodes in [*range(256, min(2**13, math.floor(size/4)), 128)]} if i > 1])
