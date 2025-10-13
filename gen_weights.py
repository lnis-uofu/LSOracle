import re
import sys

def extract_nodes(log_file):
    critical_nodes = []
    with open(log_file, 'r') as f:
        found = False
        for line in f:
            if "Startpoint:" in line:
                found = True
            elif found and "slack" in line:
                break
            elif found:
                m = re.search(r'[_a-zA-Z]\w*\/[A-Z]+', line)
                if m:
                    node = m.group(0).split('/')[0]
                    if node not in critical_nodes:
                        critical_nodes.append(node)
    return critical_nodes

def write_weights(nodes, outfile, max_weight=10.0, min_weight=1.0):
    n = len(nodes)
    with open(outfile, 'w') as f:
        for i, node in enumerate(nodes):
            weight = max_weight - i * (max_weight - min_weight) / max(n - 1, 1)
            f.write(f"{node} {round(weight, 2)}\n")

if __name__ == "__main__":
    log_path = sys.argv[1]
    out_path = sys.argv[2]
    nodes = extract_nodes(log_path)
    write_weights(nodes, out_path)