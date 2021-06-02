#pragma once

#include "../io/hmetis_io.h"
#include "../logger.h"
#include "../algorithm/ford_fulkerson.h"
#include "../algorithm/dinic.h"

namespace whfc {
namespace Test {
	
	class FlowHypergraphTests {
	private:
		using InHe = FlowHypergraph::InHe;
		using Pin = FlowHypergraph::Pin;
		
		template<typename T>
		bool compareRanges(std::vector<T> actual, std::vector<T>& expected) {
			std::sort(actual.begin(), actual.end());
			std::sort(expected.begin(), expected.end());
			return actual == expected;
		}
		
	public:
		static constexpr bool log = true;
		void routeFlowTests() {
			std::string hgfile = "../test_hypergraphs/twocenters.hgr";
			//FlowHypergraph hg = HMetisIO::readFlowHypergraph(hgfile);
			FlowHypergraphBuilder hg = HMetisIO::readFlowHypergraphWithBuilder(hgfile);
			
			Node u(1), v(11);
			Hyperedge e(3);
			InHe& inc_u = hg.findIncidence(u,e);
			InHe& inc_v = hg.findIncidence(v,e);
			Pin& pu = hg.findPin(e, u);
			Pin& pv = hg.findPin(e, v);
			
			assert(pu.pin == u);
			assert(pv.pin == v);
			assert(hg.getPin(inc_u) == pu);
			assert(hg.getPin(inc_v) == pv);
			
			hg.routeFlow(inc_u, inc_v, 1);
			
			assert(hg.flow(e) == 1);
			assert(hg.flowSent(u) == 1);
			assert(hg.flowReceived(v) == 1);
			assert(hg.flowSent(inc_u) == 1);
			assert(hg.flowReceived(inc_v) == 1);
			assert(hg.flowSent(hg.findPin(e, u)) == 1);
			assert(hg.flowReceived(hg.findPin(e, v)) == 1);
			
			Node w(9);
			Hyperedge e2(2);
			InHe& inc_w_e2 = hg.findIncidence(w, e2);
			InHe& inc_u_e2 = hg.findIncidence(u, e2);
			hg.routeFlow(inc_u_e2, inc_w_e2, 1);
			
			assert(hg.flow(e2) == 1);
			assert(hg.flowSent(inc_u_e2) == 1);
			assert(hg.flowReceived(inc_w_e2) == 1);
			assert(hg.flowSent(u) == 2);
			assert(hg.flowReceived(w) == 1);
			
			hg.routeFlow(inc_v, inc_u, 1);
			assert(hg.flow(e) == 0);
			assert(hg.flowSent(inc_u) == 0);
			assert(hg.flowReceived(inc_v) == 0);
		}
		
		template<typename FlowAlgo>
		bool tryFlowAlgo(std::string file, Flow expected_flow, Node s, Node t) {
			FlowHypergraph hg = HMetisIO::readFlowHypergraph(file);
			TimeReporter dummy_timer;
			CutterState<FlowAlgo> cs(hg, dummy_timer);
			cs.setMaxBlockWeight(0, hg.nodeWeight(s));
			cs.setMaxBlockWeight(1, hg.nodeWeight(t));
			cs.initialize(s,t);
			FlowAlgo flow(hg);
			flow.exhaustFlow(cs);
			Flow f = cs.flowValue;
			assert(f == expected_flow);
			return f == expected_flow;
		}
		
		void flowAlgoTest(std::string file, Flow expected_flow, Node s, Node t) {
			//assert(tryFlowAlgo<BasicFordFulkerson>(file, expected_flow, s, t));
			//assert(tryFlowAlgo<ScalingFordFulkerson>(file, expected_flow, s, t));
			//assert(tryFlowAlgo<BasicEdmondsKarp >(file, expected_flow, s, t));
			//assert(tryFlowAlgo<ScalingEdmondsKarp>(file, expected_flow, s, t));
			//assert(tryFlowAlgo<BasicDepthFirstFordFulkerson>(file, expected_flow, s, t));
			//assert(tryFlowAlgo<ScalingDepthFirstFordFulkerson>(file, expected_flow, s, t));
			assert(tryFlowAlgo<Dinic>(file, expected_flow, s, t));
		}
		
		void run() {
			routeFlowTests();
			flowAlgoTest("../test_hypergraphs/testhg.hgr", Flow(1), Node(14), Node(10));
			flowAlgoTest("../test_hypergraphs/twocenters.hgr", Flow(2), Node(0), Node(2));
			flowAlgoTest("../test_hypergraphs/twocenters.hgr", Flow(2), Node(0), Node(3));
			flowAlgoTest("../test_hypergraphs/push_back.hgr", Flow(6), Node(0), Node(7));
			
		}
	};
}
}