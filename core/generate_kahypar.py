import sys

in_file, out_file = sys.argv[1:]

with open(in_file, 'r') as incoming:
    with open(out_file, 'w') as outgoing:
        outgoing.write('#pragma once\n')
        outgoing.write('#include "kahypar_config.hpp"\n')
        outgoing.write('#include <string>\n')
        outgoing.write('const std::string KAHYPAR_DEFAULT_CONFIG = "')
        outgoing.write("\\n".join([x.strip() for x in incoming.readlines()]))
        outgoing.write('";')
