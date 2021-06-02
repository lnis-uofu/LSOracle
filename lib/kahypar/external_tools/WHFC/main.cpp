#include <iostream>

#include "util/range.h"
#include "datastructure/flow_hypergraph.h"
#include "algorithm/hyperflowcutter.h"
#include "algorithm/grow_assimilated.h"
#include "algorithm/ford_fulkerson.h"
#include "io/hmetis_io.h"

#include "util/random.h"
#include "datastructure/flow_hypergraph_builder.h"
#include "algorithm/dinic.h"


namespace whfc {
	void run(const std::string& filename, Node s, Node t) {
		FlowHypergraph hg = HMetisIO::readFlowHypergraph(filename);
		NodeWeight mbw(hg.totalNodeWeight() / 2);

		if (s >= hg.numNodes() || t >= hg.numNodes())
			throw std::runtime_error("s or t not within node id range");
		
		int seed = 42;
		HyperFlowCutter<Dinic> hfc(hg, seed);
		hfc.cs.setMaxBlockWeight(0, mbw);
		hfc.cs.setMaxBlockWeight(1, mbw + (hg.totalNodeWeight() % NodeWeight(2)));
		
		hfc.runUntilBalancedOrFlowBoundExceeded(s, t);
		hfc.timer.report(std::cout);
	}
}

int main(int argc, const char* argv[]) {
	if (argc != 4)
		throw std::runtime_error("Usage: ./WHFC hypergraphfile s t");
	std::string hgfile = argv[1];
	whfc::Node s(static_cast<unsigned int>(std::stoul(argv[2])));
	whfc::Node t(static_cast<unsigned int>(std::stoul(argv[3])));
	whfc::run(hgfile, s, t);
 	return 0;
}