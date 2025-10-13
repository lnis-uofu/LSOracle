import networkx as nx
import json

def read_yosys_json(input_file, top):
    G = nx.Graph()
    with open(input_file) as f:
        y = json.load(f)

    m = y["modules"][top]

    for cell, detail in m["cells"].items():
        if detail["type"] == "$scopeinfo":
            continue
        G.add_node(cell, hyperedge=False, cell=detail["type"],
                   parameters=detail["parameters"], attributes=detail["attributes"])
        for port, bits in detail["connections"].items():
            if "port_directions" in detail:
                direction = detail["port_directions"][port]
            else:
                direction = "unknown"
            for bit in bits:
                bit = f"$_{bit}"
                if bit not in G.nodes:
                    # If this is an input/output/inout, it will get marked from netnames section
                    G.add_node(bit, weight=0, hyperedge=True, cell="$_wire", netnames=set())
                assert((cell, bit) not in G.edges) # this would for a multigraph
                G.add_edge(cell, bit, hyperedge=True, port=port, direction=direction)
    for net, details in m["netnames"].items():
        # Multiple named nets will be attached to the same bit, last one wins.
        for index, bit in enumerate(details["bits"]):
            bit = f"$_{bit}"
            assert(bit in G.nodes)
            if not details.get("hide_name", 0):
                G.nodes[bit]["netnames"].add((net, details.get("index", 0)))
                # there is a possible offset, ignoring
            G.nodes[bit]["attributes"] = details["attributes"]
            if "type" in details["attributes"]:
                G.nodes[bit]["type"] = details["attributes"]["type"]
            # If the wire type is input/output/inout, mark here
            if net in m["ports"]:
                direction = m["ports"][net]["direction"]
                G.nodes[bit]["cell"] = f"$_{direction}"
    return G

def is_net(G, node):
    return G.nodes[node]["cell"] in {"$_output", "$_inout", "$_wire", "$_input"}

def is_port(G, node):
    return G.nodes[node]["cell"] in {"$_output", "$_inout", "$_input"}

def is_input_port(G, node):
    return G.nodes[node]["cell"] == "$_input"

def is_inout_port(G, node):
    return G.nodes[node]["cell"] == "$_inout"
def is_output_port(G, node):
    return G.nodes[node]["cell"] == "$_output"

def is_wire(G, node):
    return G.nodes[node]["cell"] == "$_wire"

def collapse_edges(G: nx.Graph):
    rm = []
    for node in G.nodes:
        if G.nodes[node]["hyperedge"] and G.degree(node) == 2:
            f, s = G.adj[node]
            G.add_edge(f, s, hyperedge=False)
            rm.append(node)
    for node in rm:
        G.remove_node(node)
    return G

