#pragma once

#include "../datastructure/queue.h"
#include "../datastructure/flow_hypergraph.h"
#include "cutter_state.h"
#include "../datastructure/stack.h"


namespace whfc {

template<typename FlowAlgorithm>
class GrowAssimilated {
public:
	using Pin = FlowHypergraph::Pin;
	
	using ScanList = typename FlowAlgorithm::ScanList;
	using ReachableNodes = typename FlowAlgorithm::ReachableNodes;
	using ReachableHyperedges = typename FlowAlgorithm::ReachableHyperedges;

	
	static void grow(CutterState<FlowAlgorithm>& cs, ScanList& nodes_to_scan, const bool reach_and_settle = false) {
		unused(reach_and_settle);
		
		ReachableNodes& n = cs.n;
		ReachableHyperedges& h = cs.h;
		nodes_to_scan.clear();
		FlowHypergraph& hg = cs.hg;

		for (auto& ps : cs.sourcePiercingNodes) {
			const Node s = ps.node;
			nodes_to_scan.push(s);
			assert(n.isSource(s));
			assert(!n.isTarget(s));
			assert(!n.isTargetReachable(s));
		}

		while (!nodes_to_scan.empty()) {
			const Node u = nodes_to_scan.pop();
			for (const auto& he_inc : hg.hyperedgesOf(u)) {
				const Hyperedge e = he_inc.e;

				if (!h.areAllPinsSources(e)) {
					const bool scanAllPins = !hg.isSaturated(e) || hg.flowReceived(he_inc) > 0;
					if (scanAllPins) {
						assert(h.areAllPinsSourceReachable(e) || reach_and_settle);
						cs.settleAllPins(e);
						
						if (FlowAlgorithm::grow_reachable_marks_flow_sending_pins_when_marking_all_pins) {
							assert(h.areFlowSendingPinsSourceReachable(e) || reach_and_settle);
							if (!h.areFlowSendingPinsSources(e))
								cs.settleFlowSendingPins(e);
						}
					}
					else {
						if (cs.shouldBeAddedToCut(e))
							cs.addToCut(e);
						if (h.areFlowSendingPinsSources(e))
							continue;
						
						assert(h.areFlowSendingPinsSourceReachable(e) || reach_and_settle || (!FlowAlgorithm::same_traversal_as_grow_assimilated && h.areAllPinsSourceReachable(e)));
#ifndef NDEBUG
						if (!FlowAlgorithm::same_traversal_as_grow_assimilated && h.areAllPinsSourceReachable(e) && !h.areFlowSendingPinsSourceReachable(e)) {
							h.reachFlowSendingPins(e);
						}
#endif
						cs.settleFlowSendingPins(e);
					}

					for (const Pin& pv : scanAllPins ? hg.pinsOf(e) : hg.pinsSendingFlowInto(e)) {
						const Node v = pv.pin;
						assert(!n.isTargetReachable(v));
						assert(reach_and_settle || n.isSourceReachable(v) || cs.isIsolated(v));
						if (!n.isSource(v) && !cs.isIsolated(v)) {
							cs.settleNode(v);
							nodes_to_scan.push(v);
						}
					}
				}
			}
		}
	}


};

}