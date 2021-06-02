#pragma once

#include "../definitions.h"
#include "../util/unused.h"
#include <boost/dynamic_bitset.hpp>

namespace whfc {

	//TODO. when all flow algorithms are implemented. Reassess whether we want to store the flow at the pins or the incidences. and whether we actually want array of structs
	
	class FlowHypergraph {
	public:
		struct Pin {
			Node pin = invalidNode;
			InHeIndex he_inc_iter;
			bool operator==(const Pin& o) const { return o.pin == pin && o.he_inc_iter == he_inc_iter; }
		};

		struct InHe {	//Hyperedge Incidence
			Hyperedge e = invalidHyperedge;
			Flow flow = Flow(0);
			PinIndex pin_iter;
		};

		struct HyperedgeData {
			PinIndex first_out = PinIndex(0);
			Flow flow = Flow(0);
			Flow capacity = Flow(0);
		};

		struct NodeData {
			InHeIndex first_out = InHeIndex(0);
			NodeWeight weight = NodeWeight(0);
		};
		
		using PinRange = mutable_range<std::vector<Pin>>;
		using PinIterator = PinRange::iterator;
		using PinIndexRange = mutable_index_range<PinIndex>;
		using InHeRange = mutable_range<std::vector<InHe>>;
		using InHeIterator = InHeRange::iterator;
		using InHeIndexRange = mutable_index_range<InHeIndex>;

		inline auto nodeIDs() const { return mutable_index_range<Node>(Node(0), Node::fromOtherValueType(numNodes())); }
		inline auto hyperedgeIDs() const { return mutable_index_range<Hyperedge>(Hyperedge(0), Hyperedge::fromOtherValueType(numHyperedges())); }
		inline auto pinIndices() const { return PinIndexRange(PinIndex(0), PinIndex::fromOtherValueType(numPins())); }

		FlowHypergraph() : nodes(1), hyperedges(1) { }
		
		//use in FlowHypergraphBuilder to get rid of any allocations
		FlowHypergraph(size_t maxNumNodes, size_t maxNumHyperedges, size_t maxNumPins) :
				nodes(maxNumNodes + 1), hyperedges(maxNumHyperedges + 1), pins(maxNumPins),
				incident_hyperedges(maxNumPins), pins_sending_flow(maxNumHyperedges), pins_receiving_flow(maxNumHyperedges) { }

		FlowHypergraph(std::vector<NodeWeight>& node_weights, std::vector<HyperedgeWeight>& hyperedge_weights, std::vector<PinIndex>& hyperedge_sizes, std::vector<Node>& _pins) :
				maxHyperedgeCapacity(0),
				nodes(node_weights.size() + 1),
				hyperedges(hyperedge_weights.size() + 1),
				pins(_pins.size()),
				incident_hyperedges(_pins.size()),
				pins_sending_flow(hyperedge_weights.size()),
				pins_receiving_flow(hyperedge_weights.size()),
				total_node_weight(boost::accumulate(node_weights, NodeWeight(0)))
		{
			size_t i = 0;
			for (const Node p : _pins) {
				pins[i++].pin = p;					//copy pins
				nodes[p + 1].first_out++;			//bucket sizes
			}
			
			for (Node u : nodeIDs()) {
				nodes[u + 1].first_out += nodes[u].first_out;			//prefix sum
				nodes[u].weight = node_weights[u];						//copy node weights
			}
			
			for (Hyperedge e : hyperedgeIDs()) {
				hyperedges[e].capacity = hyperedge_weights[e];
				hyperedges[e+1].first_out = hyperedges[e].first_out + hyperedge_sizes[e];		//prefix sum
				for (auto pin_it = beginIndexPins(e); pin_it != endIndexPins(e); pin_it++) {
					Pin& p = pins[pin_it];
					InHeIndex ind_he = nodes[p.pin].first_out++;							//destroy first_out temporarily and reset later
					InHe& inc_he = incident_hyperedges[ind_he];
					inc_he.e = e;
					inc_he.pin_iter = pin_it;				//set iterator for pin -> its position in the pins of the hyperedge
					p.he_inc_iter = ind_he;					//set iterator for incident hyperedge -> its position in incident_hyperedges of the node
				}
				maxHyperedgeCapacity = std::max(maxHyperedgeCapacity, hyperedges[e].capacity);
			}
			
			for (Node u(numNodes()-1); u > 0; u--)
				nodes[u].first_out = nodes[u-1].first_out;	//reset temporarily destroyed first_out
			nodes[0].first_out = InHeIndex(0);
			
			PinIndex x = PinIndex(0);
			for (Hyperedge e : hyperedgeIDs()) {
				pins_sending_flow[e] = PinIndexRange(x,x);	//empty range starting at the first pin of e
				x += pinCount(e);
				pins_receiving_flow[e] = PinIndexRange(x, x);	//empty range starting at one past the last pin of e
			}
		}
		

		bool hasNodeWeights() const { return std::any_of(nodes.begin(), nodes.begin() + numNodes(), [](const NodeData& u) { return u.weight > 1; }); }
		bool hasHyperedgeWeights() const { return std::any_of(hyperedges.begin(), hyperedges.begin() + numHyperedges(), [](const HyperedgeData& e) { return e.capacity > 1; }); }
		inline size_t numNodes() const { return nodes.size() - 1 ; }
		inline size_t numHyperedges() const { return hyperedges.size() - 1; }
		inline size_t numPins() const { return pins.size(); }
		inline PinIndex pinCount(const Hyperedge e) const { return hyperedges[e+1].first_out - hyperedges[e].first_out; }
		inline InHeIndex degree(const Node u) const { return nodes[u+1].first_out - nodes[u].first_out; }
		inline NodeWeight totalNodeWeight() const { return total_node_weight; }
		inline NodeWeight nodeWeight(const Node u) const { return nodes[u].weight; }
		inline NodeWeight& nodeWeight(const Node u) { return nodes[u].weight; }

		inline InHeIndex beginIndexHyperedges(Node u) const { return nodes[u].first_out; }
		inline InHeIndex endIndexHyperedges(Node u) const { return nodes[u+1].first_out; }
		//interface for irange is front(), drop_front(), empty(), size(), begin(), end()
		inline InHeIndexRange incidentHyperedgeIndices(const Node u) const {
			return InHeIndexRange(beginIndexHyperedges(u), endIndexHyperedges(u));
		}
		inline InHe& getInHe(const InHeIndex ind_e) { return incident_hyperedges[ind_e]; }
		inline InHe& getInHe(const Pin& pin) { return getInHe(pin.he_inc_iter); }
		inline const InHe& getInHe(const InHeIndex ind_e) const { return incident_hyperedges[ind_e]; }
		inline const InHe& getInHe(const Pin& pin) const { return getInHe(pin.he_inc_iter); }

		inline PinIndex beginIndexPins(const Hyperedge e) const { return hyperedges[e].first_out; }
		inline PinIndex endIndexPins(const Hyperedge e) const { return hyperedges[e+1].first_out; }
		inline PinIndexRange pinIndices(const Hyperedge e) const { return PinIndexRange(beginIndexPins(e), endIndexPins(e)); }
		inline PinIndexRange pinsSendingFlowIndices(const Hyperedge e) const { return pins_sending_flow[e]; }
		inline Pin& getPin(const PinIndex ind_p) { return pins[ind_p]; }
		inline Pin& getPin(const InHe& inc_p) { return getPin(inc_p.pin_iter); }
		inline const Pin& getPin(const PinIndex ind_p) const { return pins[ind_p]; }
		inline const Pin& getPin(const InHe& inc_p) const { return getPin(inc_p.pin_iter); }


		inline InHeIterator beginHyperedges() { return incident_hyperedges.begin(); }
		inline InHeIterator endHyperedges() { return incident_hyperedges.end(); }
		inline InHeIterator beginHyperedges(const Node u) { return incident_hyperedges.begin() + nodes[u].first_out; }
		inline InHeIterator endHyperedges(const Node u) { return incident_hyperedges.begin() + nodes[u+1].first_out; }
		InHeRange hyperedgesOf(const Node u) { return InHeRange(beginHyperedges(u), endHyperedges(u)); }
		InHeRange hyperedgesInRange(const InHeIndexRange hir) { return InHeRange(beginHyperedges() + hir.begin(), beginHyperedges() + hir.end()); }

		inline PinIterator beginPins() { return pins.begin(); }
		inline PinIterator endPins() { return pins.end(); }
		inline PinIterator beginPins(const Hyperedge e) { return pins.begin() + hyperedges[e].first_out; }
		inline PinIterator endPins(const Hyperedge e) { return pins.begin() + hyperedges[e+1].first_out; }
		PinRange pinsOf(const Hyperedge e) { return PinRange(beginPins(e), endPins(e)); }
		PinRange pinsInRange(const PinIndexRange pir) { return PinRange(beginPins() + pir.begin(), beginPins() + pir.end()); }
		PinRange pinsSendingFlowInto(const Hyperedge e) { return pinsInRange(pins_sending_flow[e]); }
		PinRange pinsReceivingFlowFrom(const Hyperedge e) { return pinsInRange(pins_receiving_flow[e]); }
		
		PinIndexRange pinsNotSendingFlowIndices(const Hyperedge e) const {
			if (forwardView()) {
				return PinIndexRange(pins_sending_flow[e].end(), endIndexPins(e));
			}
			else {
				return PinIndexRange(beginIndexPins(e), pins_sending_flow[e].begin());
			}
		}
		
		PinRange pinsNotSendingFlowInto(const Hyperedge e) {
			return pinsInRange(pinsNotSendingFlowIndices(e));
		}
		
		PinRange pinsWithoutFlow(const Hyperedge e) {
			return pinsInRange(pins_without_flow(e));
		}

		inline bool forwardView() const { return sends_multiplier == 1; }
		void flipViewDirection() { std::swap(pins_sending_flow, pins_receiving_flow); std::swap(sends_multiplier, receives_multiplier); }
		
		
		inline Flow capacity(const Hyperedge e) const { return hyperedges[e].capacity; }
		inline Flow& capacity(const Hyperedge e) { return hyperedges[e].capacity; }
		inline Flow flow(const Hyperedge e) const { return hyperedges[e].flow; }
		inline Flow& flow(const Hyperedge e) { return hyperedges[e].flow; }
		inline Flow residualCapacity(const Hyperedge e) const { return capacity(e) - flow(e); }
		inline bool isSaturated(const Hyperedge e) const { assert(flow(e) <= capacity(e)); return flow(e) == capacity(e); }

		inline Flow flowSent(const Flow f) const { return f * sends_multiplier; }
		//flow sent from u = getPin(inc_u.pin_iter).pin into e = inc_u.e
		inline Flow flowSent(const InHe& inc_u) const { return flowSent(inc_u.flow); }
		inline Flow absoluteFlowSent(const InHe& inc_u) const { return std::max(0, flowSent(inc_u)); }
		inline Flow flowSent(const Pin& pin) const { return flowSent(getInHe(pin)); }
		inline Flow absoluteFlowSent(const Pin& pin) const { return std::max(0, flowSent(pin)); }
		
		//for testing only
		inline Flow flowSent(const Node u) {
			Flow f = 0;
			for (const InHe& inc_u : hyperedgesOf(u))
				f += flowSent(inc_u.flow);
			return f;
		}
		
		inline Flow flowReceived(const Node u) {
			return -flowSent(u);
		}

		inline Flow flowReceived(const Flow f) const { return f * receives_multiplier; }
		//flow that u = getPin(inc_u.pin_iter).pin receives from e = inc_u.e
		inline Flow flowReceived(const InHe& inc_u) const { return flowReceived(inc_u.flow); }
		inline Flow flowReceived(const Pin& pin) const { return flowReceived(getInHe(pin)); }
		inline Flow absoluteFlowReceived(const InHe& inc_u) const { return std::max(0, flowReceived(inc_u)); }
		//inline Flow flowReceived(const Pin& pin) const { return flowReceived(getInHe(pin)); }

		inline Flow residualCapacity(const InHe& inc_u, InHe& inc_v) const {
			return absoluteFlowReceived(inc_u) + absoluteFlowSent(inc_v) + residualCapacity(inc_u.e);
		}
		
		//for testing only
		InHe& findIncidence(const Node u, const Hyperedge e) {
			for (InHe& x : hyperedgesOf(u))
				if (x.e == e)
					return x;
			throw std::out_of_range("e is not in the list of incident hyperedges of u");
		}
		
		//for testing only
		Pin& findPin(const Hyperedge e, const Node v) {
			for (Pin& x : pinsOf(e))
				if (x.pin == v)
					return x;
			throw std::out_of_range("v is not a pin of e");
		}

		void routeFlow(InHe& inc_u, InHe& inc_v, Flow flow_delta) {
			const Hyperedge e = inc_u.e;
			assert(inc_u.e == inc_v.e && "Routing flow but incident hyperedges are not the same");
			assert(flow_delta > 0 && "Routing <= 0 flow.");
			assert(flow_delta <= residualCapacity(inc_u, inc_v) && "Routing more flow than residual capacity");
			assert(flow(e) <= capacity(e) && "Capacity on e already exceeded");
			assert(std::abs(inc_u.flow) <= capacity(e) && "Pin capacity already violated (u)");
			assert(std::abs(inc_v.flow) <= capacity(e) && "Pin capacity already violated (v)");

			const Flow prevFlowU = inc_u.flow;
			const Flow prevFlowV = inc_v.flow;
			
			Flow flow_delta_on_v_eOut_eIn_u = std::min({ absoluteFlowSent(inc_v), absoluteFlowReceived(inc_u), flow_delta });
			inc_u.flow += flowSent(flow_delta_on_v_eOut_eIn_u);
			inc_v.flow += flowReceived(flow_delta_on_v_eOut_eIn_u);
			flow(e) -= flow_delta_on_v_eOut_eIn_u;
			flow_delta -= flow_delta_on_v_eOut_eIn_u;
			
			Flow flow_delta_on_v_eOut_u = std::min(flow_delta, absoluteFlowReceived(inc_u));
			inc_u.flow += flowSent(flow_delta_on_v_eOut_u);
			inc_v.flow += flowReceived(flow_delta_on_v_eOut_u);
			//does not influence flow(e)
			flow_delta -= flow_delta_on_v_eOut_u;
			
			Flow flow_delta_on_v_eIn_u = std::min(flow_delta, absoluteFlowSent(inc_v));
			inc_u.flow += flowSent(flow_delta_on_v_eIn_u);
			inc_v.flow += flowReceived(flow_delta_on_v_eIn_u);
			//does not influence flow(e)
			flow_delta -= flow_delta_on_v_eIn_u;
			
			Flow flow_delta_on_v_eIn_eOut_u = flow_delta;
			assert(flow_delta_on_v_eIn_eOut_u <= residualCapacity(e));
			inc_u.flow += flowSent(flow_delta_on_v_eIn_eOut_u);
			inc_v.flow += flowReceived(flow_delta_on_v_eIn_eOut_u);
			flow(e) += flow_delta_on_v_eIn_eOut_u;

			
			if (flowReceived(prevFlowU) > 0 && flowSent(inc_u.flow) >= 0)	//u previously received flow and now either has none, or sends flow.
				removePinFromFlowPins(inc_u, true);
			if (flowSent(inc_u.flow) > 0 && flowSent(prevFlowU) <= 0) //u now sends flow and did not previously, thus must be inserted into pins_sending_flow
				insertPinIntoFlowPins(inc_u, false);

			if (flowSent(prevFlowV) > 0 && flowReceived(inc_v.flow) >= 0) //v previously sent flow and now either has none, or receives flow.
				removePinFromFlowPins(inc_v, false);
			if (flowReceived(inc_v.flow) > 0 && flowReceived(prevFlowV) <= 0)  //v now receives flow and did not previously, thus must be inserted into pins_receiving_flow
				insertPinIntoFlowPins(inc_v, true);
			
			assert(check_flow_conservation_locally(e));
			assert(sanity_check_pin_ranges(e));
			assert(assert_backpointers_correct(inc_u));
			assert(assert_backpointers_correct(inc_v));
			assert(pin_is_categorized_correctly(inc_u) && "Pin categorized incorrectly");
			assert(pin_is_categorized_correctly(inc_v) && "Pin categorized incorrectly");
		}
		
		Flow maxHyperedgeCapacity = maxFlow;

	protected:
		std::vector<NodeData> nodes;
		std::vector<HyperedgeData> hyperedges;
		std::vector<Pin> pins;
		std::vector<InHe> incident_hyperedges;

		//TODO get rid of the range and just store one index, if this turns out to be cache inefficient later on
		std::vector<PinIndexRange> pins_sending_flow;	//indexed by hyperedge id. gives range of pin ids/iterators sending flow to that hyperedge. grows right if forwardView = true
		std::vector<PinIndexRange> pins_receiving_flow;	//indexed by hyperedge id. gives range of pin ids/iterators receiving flow from that hyperedge. grows left if forwardView = true
		
		NodeWeight total_node_weight = NodeWeight(0);
		int sends_multiplier = 1;						//if forwardView = true, flow entering hyperedge e should be positive and flow exiting e should be negative. reverse, if forwardView = false.
		int receives_multiplier = -1;

		PinIndexRange pins_without_flow(const Hyperedge e) const {
			return forwardView() ? PinIndexRange(pins_sending_flow[e].end(), pins_receiving_flow[e].begin()) : PinIndexRange(pins_receiving_flow[e].end(), pins_sending_flow[e].begin());
		}

		PinIndex removePinFromFlowPins(InHe& inc_u, bool flow_receiving_pins) {
			const Hyperedge e = inc_u.e;
			PinIndex it_u = inc_u.pin_iter;
			PinIndexRange& flow_pins = flow_receiving_pins ? pins_receiving_flow[e] : pins_sending_flow[e];
			assert(!flow_pins.empty());
			assert(flow_pins.contains(it_u));

			PinIndex it_o = (forwardView() == flow_receiving_pins) ? flow_pins.begin() : PinIndex(flow_pins.end() - 1);
			InHe& inc_o = getInHe(getPin(it_o));
			assert(it_o == it_u || (flow_receiving_pins ? flowReceived(inc_o) > 0 : flowSent(inc_o) > 0));	//ensure it_o, taken from flow_pins, actually receives or sends flow, as appropriate
			if (forwardView() == flow_receiving_pins)
				flow_pins.advance_begin();
			else
				flow_pins.retreat_end();
			std::swap(inc_u.pin_iter, inc_o.pin_iter);
			std::swap(pins[it_u], pins[it_o]);
			assert(pins_without_flow(e).contains(it_o));
			return it_o;
		}

		PinIndex insertPinIntoFlowPins(InHe& inc_u, bool flow_receiving_pins) {
			const Hyperedge e = inc_u.e;
			PinIndex it_u = inc_u.pin_iter;
			PinIndexRange& flow_pins = flow_receiving_pins ? pins_receiving_flow[e] : pins_sending_flow[e];
			assert(pins_without_flow(e).contains(it_u));
			PinIndex it_o = (forwardView() == flow_receiving_pins) ? PinIndex(flow_pins.begin() - 1) : flow_pins.end();
			InHe& inc_o = getInHe(getPin(it_o));
			if (forwardView() == flow_receiving_pins)
				flow_pins.retreat_begin();
			else
				flow_pins.advance_end();
			std::swap(inc_u.pin_iter, inc_o.pin_iter);
			std::swap(pins[it_u], pins[it_o]);
			assert(flow_pins.contains(it_o));
			return it_o;
		}
		
		bool check_flow_conservation_locally(const Hyperedge e) {
			Flow f = 0, f_in = 0;
			for (Pin& p : pinsOf(e)) {
				f += flowSent(p);
				f_in += absoluteFlowSent(p);
				assert(std::abs(flowSent(p)) <= capacity(e));
			}
			assert(f == 0);
			assert(f_in == flow(e));
			return true;
		}

		bool assert_backpointers_correct(const InHe& inhe) const {
			const InHe& doubled = getInHe(getPin(inhe));
			unused(inhe, doubled);
			assert(doubled.pin_iter == inhe.pin_iter && "Backpointer Pin Iter inconsistent");
			assert(doubled.e == inhe.e && "Backpointer hyperedge ID inconsistent");
			assert(doubled.flow == inhe.flow && "Backpointer Pin Iter inconsistent");
			return true;
		}

		bool sanity_check_pin_ranges(const Hyperedge e) const {
			//check left / right end of pin ranges agree with first_out
			const PinIndexRange& s = forwardView() ? pins_sending_flow[e] : pins_receiving_flow[e];
			const PinIndexRange& l = !forwardView() ? pins_sending_flow[e] : pins_receiving_flow[e];
			unused(e, s, l);
			assert(hyperedges[e].first_out == s.begin());
			assert(hyperedges[e+1].first_out == l.end());

			assert(s.begin() <= s.end());
			assert(s.end() <= l.begin());
			assert(l.begin() <= l.end());
			return true;
		}

		bool pin_is_categorized_correctly(const InHe& inc_u) {
			const Hyperedge e = inc_u.e;
			const PinIndex it_u = inc_u.pin_iter;
			bool sends = flowSent(inc_u) > 0 && pins_sending_flow[e].contains(it_u);
			bool receives = flowReceived(inc_u.flow) > 0 && pins_receiving_flow[e].contains(it_u);
			bool no_flow = inc_u.flow == 0 && pins_without_flow(e).contains(it_u);
			return 		(sends && !receives && !no_flow)
					|| 	(!sends && receives && !no_flow)
					|| 	(!sends && !receives && no_flow);
		}
		
		static_assert(std::is_trivially_destructible<Pin>::value);
		static_assert(std::is_trivially_destructible<InHe>::value);
		static_assert(std::is_trivially_destructible<HyperedgeData>::value);
		static_assert(std::is_trivially_destructible<NodeData>::value);
		static_assert(std::is_trivially_destructible<PinIndexRange>::value);

	public:

		void printNodes(std::ostream& out) {
			out << "---Nodes---\n";
			for (const Node u : nodeIDs()) {
				out << u << " deg = " << degree(u) << " w= " << nodeWeight(u) << " inc_hes = [";
				for (const InHe e : hyperedgesOf(u))
					out << e.e << " ";
				out << "]" << "\n";
			}
			out << std::flush;
		}

		void printHyperedges(std::ostream& out) {
			out << "---Hyperedges---\n";
			for (const Hyperedge e: hyperedgeIDs()) {
				out << e << " pincount = " << pinCount(e) << " w= " << capacity(e) << " pins (pin,flow) = [";
				for (const Pin& u : pinsOf(e)) {
					assert(pin_is_categorized_correctly(getInHe(u)));
					out << "(" << u.pin << "," << getInHe(u).flow << ") ";
				}
				out << "]" << "\n";
			}
			out << std::flush;
		}

		void printHypergraph(std::ostream& out) {
			printNodes(out);
			printHyperedges(out);
		}

		friend std::ostream& operator<<(std::ostream& out, FlowHypergraph& hg) noexcept {
			hg.printHypergraph(out);
			return out;
		}

	};
}