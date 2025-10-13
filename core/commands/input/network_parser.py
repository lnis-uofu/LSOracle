import argparse
import json
import networkx as nx
from collections import defaultdict
"""
Node:
_Cell: hyperedge=False, attrs: cell (type), module
_Net: hyperedge=True, attrs: 
"""

def read_yosys_json(input_file):
    G = nx.Graph()
    with open(input_file) as f:
        y = json.load(f)

        # for _, mod in y.get("modules", {}).items():
        #     for _, cell in mod.get("cells", {}).items():
        #         h = cell.get("attributes", {}).get("hdlname")
        #         print(h)

    for module_name, module in y["modules"].items():
        attributes = module.get("attributes", {})
        hdlname = attributes.get("hdlname")
        ports = module.get("ports", {})
        cells = module.get("cells", {})
        netnames = module.get("netnames", {})

        for cell_name, cell in cells.items():
            type_name = cell.get("type")
            port_directions = cell.get("port_directions", {})
            connections = cell.get("connections", {})

            G.add_node(cell_name, hyperedge=False, cell=type_name, module=hdlname)
            
            for connection, bits in connections.items():
                direction = port_directions.get(connection)
                for bit in bits:
                    bit = f"$_{bit}"
                    if bit not in G.nodes:
                        G.add_node(bit, weight=0, hyperedge=True, cell="$_wire", netnames=set(), module=hdlname)
                    if not G.has_edge(cell_name, bit):
                        G.add_edge(cell_name, bit, hyperedge=True, connection=connection, direction=direction)
                    
                    
        # for _, net in netnames.items():
        #     #hide = net.get("hide_name")
        #     bits = net.get("bits", {})
        #     for idx, bit in enumerate(bits):
        #         bit = f"$_{bit}"
        #         assert(bit in G.nodes)
        #         G.nodes[bit]
        
        for _, port in ports.items():
            direction = port.get("direction")
            bits = port.get("bits", {})
            for idx, bit in enumerate(bits):
                bit = f"$_{bit}"
                G.nodes[bit]["cell"] = f"$_{direction}"
    
    return G




        # modules = y["modules"][top]                  
        # for cell, detail in m["cells"].items():
        #     if detail["type"]
        


#AIG file format
#Tag M I L O A
#M: max variable index used
#I: #input
#O: #output
#L: #latches
#A: #AND gate

# class AIG:
#     def __init__(self):
#         self.M = 0          #max variable index used
#         self.pis = []       #list of primary inputs
#         self.pos = []       #list of primary outputs 
#         self.latches = []   #list of latches
#         self.ands = []      #list of and gate
    
#     def increment_variable(self):
#         self.M += 1

#     def add_and(self, a, b):
#         increment_variable(self)
#         self.ands.append((a, b))
    
#     def add_and_gate(self, a, b):
#         increment_variable(self)
#         self.ands.append((a, b))
    
#     def add_latches(self, a, b):
#         increment_variable(self)
#         self.ands.latches((a, b))
    

        
if __name__ := "__main__":
    read_yosys_json("/mnt/nas/users/cam/LSOracle_mod/LSOracle/benchmarks/ibex/ibex_aigmap.json")
