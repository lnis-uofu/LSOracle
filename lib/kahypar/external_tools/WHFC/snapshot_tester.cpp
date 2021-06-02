#include <iostream>
#include "datastructure/flow_hypergraph.h"
#include "algorithm/hyperflowcutter.h"
#include "algorithm/ford_fulkerson.h"
#include "io/hmetis_io.h"
#include "io/whfc_io.h"
#include "util/random.h"
#include "datastructure/flow_hypergraph_builder.h"
#include "algorithm/dinic.h"


namespace whfc {
	void runSnapshotTester(const std::string& filename) {
		
		using FlowAlgorithm = Dinic;
		//using FlowAlgorithm = ScalingDinic;
		
		WHFC_IO::WHFCInformation info = WHFC_IO::readAdditionalInformation(filename);
		Node s = info.s;
		Node t = info.t;
		std::cout << s << " " << t << " " << info.maxBlockWeight[0] << " " << info.maxBlockWeight[1] << " " << info.upperFlowBound<< std::endl;
		
		FlowHypergraphBuilder hg = HMetisIO::readFlowHypergraphWithBuilder(filename);
		if (s >= hg.numNodes() || t >= hg.numNodes())
			throw std::runtime_error("s or t not within node id range");
		
		int seed = 42;
		HyperFlowCutter<FlowAlgorithm> hfc(hg, seed);
		hfc.upperFlowBound = info.upperFlowBound;
		for (int i = 0; i < 2; ++i)
			hfc.cs.setMaxBlockWeight(i, info.maxBlockWeight[i]);
		
		WHFC_IO::readRandomGeneratorState(filename, hfc.cs.rng);
		
		hfc.timer.start();
		hfc.runUntilBalancedOrFlowBoundExceeded(s, t);
		hfc.timer.stop();
		hfc.timer.report(std::cout);
		hfc.timer.clear();
		
		std::cout << "Reread" << std::endl;
		HMetisIO::readFlowHypergraphWithBuilder(hg, filename);
		WHFC_IO::readRandomGeneratorState(filename, hfc.cs.rng);

		std::cout << "Reset" << std::endl;
		hfc.reset();
		hfc.upperFlowBound = info.upperFlowBound;
		std::cout << "Run again" << std::endl;
		
		hfc.timer.start();
		hfc.runUntilBalancedOrFlowBoundExceeded(s, t);
		hfc.timer.stop();
		hfc.timer.report(std::cout);
		hfc.timer.clear();
		
		
	}
}

int main(int argc, const char* argv[]) {
	if (argc < 2 || argc > 3)
		throw std::runtime_error("Usage: ./WHFC hypergraphfile");
	std::string hgfile = argv[1];
	whfc::runSnapshotTester(hgfile);
	return 0;
}