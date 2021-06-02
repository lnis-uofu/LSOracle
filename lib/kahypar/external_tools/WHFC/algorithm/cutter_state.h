#pragma once

#include "../datastructure/queue.h"
#include "../definitions.h"
#include "../datastructure/border.h"
#include "../datastructure/node_border.h"
#include "../datastructure/flow_hypergraph.h"
#include "../datastructure/isolated_nodes.h"
#include "../datastructure/bitset_reachable_sets.h"
#include "../util/math.h"
#include "../util/random.h"


// TODO factor out verification code, maybe even balance checking?

namespace whfc {
	
	struct SimulatedNodeAssignment {
		bool assignUnclaimedToSource = true;
		bool assignTrackedIsolatedWeightToSource = true;
		NodeWeight trackedIsolatedWeight = invalidWeight;
		double imbalanceSourceBlock = std::numeric_limits<double>::max(), imbalanceTargetBlock = std::numeric_limits<double>::max();
		size_t numberOfTrackedMoves = 0;
		int direction = 0;
		
		double imbalance() const {
			return std::max(imbalanceSourceBlock, imbalanceTargetBlock);
		}
		
		bool isPerfectlyBalanced() const {
			return std::abs(imbalanceSourceBlock - imbalanceTargetBlock) < 1e-9;
		}
	};
	
	struct Move {
		enum class Type : uint8_t { SettleNode, SettleAllPins, SettleFlowSendingPins };
		Node node;
		Hyperedge hyperedge;
		int direction;
		Type t;
		Move(Node node, int dir, Type t) : node(node), hyperedge(invalidHyperedge), direction(dir), t(t) {
			assert(t == Type::SettleNode);
		}
		Move(Hyperedge hyperedge, int dir, Type t) : node(invalidNode), hyperedge(hyperedge), direction(dir), t(t) {
			assert(t == Type::SettleAllPins || t == Type::SettleFlowSendingPins);
		}
	};
	
	struct PiercingNode {
		Node node;
		bool isReachableFromOppositeSide;
		PiercingNode(const Node node, bool isReachableFromOppositeSide) : node(node), isReachableFromOppositeSide(isReachableFromOppositeSide) { }
	};
	
	struct NonDynamicCutterState {
		std::vector<PiercingNode> sourcePiercingNodes, targetPiercingNodes;
		int direction;
	};
	
	template<typename FlowAlgorithm>
	class CutterState {
	public:
		static constexpr bool log = false;
		
		static constexpr bool useIsolatedNodes = false;
		
		using Pin = FlowHypergraph::Pin;
		
		int viewDirection = 0;
		FlowHypergraph& hg;
		Flow flowValue = 0;

		using ReachableNodes = typename FlowAlgorithm::ReachableNodes;
		using ReachableHyperedges = typename FlowAlgorithm::ReachableHyperedges;
		ReachableNodes n;
		ReachableHyperedges h;
		std::vector<PiercingNode> sourcePiercingNodes, targetPiercingNodes;
		std::vector<Move> trackedMoves;
		
		bool augmentingPathAvailableFromPiercing = true;
		bool hasCut = false;
		bool mostBalancedCutMode = false;
		HyperedgeCuts cuts;
		NodeBorders borderNodes;
		std::array<NodeWeight, 2> maxBlockWeightPerSide;
		IsolatedNodes isolatedNodes;
		bool partitionWrittenToNodeSet = false;
		TimeReporter& timer;
		Randomizer rng;

		CutterState(FlowHypergraph& _hg, TimeReporter& timer) :
				hg(_hg),
				n(_hg),
				h(_hg),
				cuts(_hg.numHyperedges()),
				borderNodes(_hg.numNodes()),
				maxBlockWeightPerSide({NodeWeight(0), NodeWeight(0)}),
				isolatedNodes(hg, useIsolatedNodes),
				timer(timer)
		{
			timer.registerCategory("Balance Check");
		}
		
		inline bool isIsolated(const Node u) const {
			return useIsolatedNodes && !n.isSource(u) && !n.isTarget(u) && isolatedNodes.isCandidate(u);
		}
		
		inline bool canBeSettled(const Node u) const {
			return !n.isSource(u) && !n.isTarget(u) && !isIsolated(u);
		}

		inline NodeWeight unclaimedNodeWeight() const {
			return hg.totalNodeWeight() - n.sourceReachableWeight - n.targetReachableWeight - isolatedNodes.weight;
		}
		
		inline NodeWeight notSettledNodeWeight() const {
			return hg.totalNodeWeight() - n.sourceWeight - n.targetWeight - isolatedNodes.weight;
		}
		
		inline bool shouldBeAddedToCut(const Hyperedge e) const {
			return !h.areAllPinsSourceReachable(e) && !cuts.sourceSide.wasAdded(e) && hg.isSaturated(e); // the first condition is just an optimization, not really necessary
		}

		inline void addToCut(const Hyperedge e) {
			//Note: the current implementation of selecting piercing nodes relies on not inserting target-reachable nodes during most balanced cut mode
			assert(shouldBeAddedToCut(e));
			for (const Pin& px : hg.pinsOf(e)) {
				if (canBeSettled(px.pin) && !borderNodes.sourceSide->wasAdded(px.pin) && (!mostBalancedCutMode || !n.isTargetReachable(px.pin))) {
					borderNodes.sourceSide->add(px.pin, n.isTargetReachable(px.pin));
				}
			}
			cuts.sourceSide.add(e);
		}
		
		void setMaxBlockWeight(int side, NodeWeight mw) {
			maxBlockWeightPerSide[side] = mw;
			isolatedNodes.adaptMaxBlockWeight(mw);
		}
		
		NodeWeight maxBlockWeight(int side) const {
			return maxBlockWeightPerSide[side];
		}
		
		NodeWeight maxBlockWeight() const {
			return maxBlockWeight(currentViewDirection());
		}
		
		void settleNode(const Node u, bool check = true) {
			assert(!n.isSource(u) && !n.isTarget(u) && (!check || !isIsolated(u)));
			unused(check);
			
			if (!n.isSourceReachable(u))
				n.reach(u);
			n.settle(u);

			if (mostBalancedCutMode) {
				trackedMoves.emplace_back(u, currentViewDirection(), Move::Type::SettleNode);
				return;
			}
			
			if constexpr (useIsolatedNodes) {
				for (const auto& he_inc : hg.hyperedgesOf(u)) {
					const Hyperedge e = he_inc.e;
					if (!isolatedNodes.hasSettledSourcePins[e]) {
						isolatedNodes.hasSettledSourcePins.set(e);
						if (isolatedNodes.hasSettledTargetPins[e]) {	//e just became mixed
							for (const auto& px : hg.pinsOf(e)) {
								const Node p = px.pin;
								isolatedNodes.mixedIncidentHyperedges[p]++;
								if (isIsolated(p)) {
									isolatedNodes.add(p);
									if (n.isSourceReachable(p))
										n.unreachSource(p);
									if (n.isTargetReachable(p))
										n.unreachTarget(p);
								}
							}
						}
					}
				}
			}
		}
		
		void settleFlowSendingPins(const Hyperedge e) {
			if (mostBalancedCutMode) {
				trackedMoves.emplace_back(e, currentViewDirection(), Move::Type::SettleFlowSendingPins);
			}
			h.settleFlowSendingPins(e);
		}
		
		void settleAllPins(const Hyperedge e) {
			if (mostBalancedCutMode) {
				trackedMoves.emplace_back(e, currentViewDirection(), Move::Type::SettleAllPins);
			}
			h.settleAllPins(e);
		}

		void flipViewDirection() {
			viewDirection = 1 - viewDirection;
			hg.flipViewDirection();
			n.flipViewDirection();
			h.flipViewDirection();
			sourcePiercingNodes.swap(targetPiercingNodes);
			cuts.flipViewDirection();
			borderNodes.flipViewDirection();
			isolatedNodes.flipViewDirection();
		}

		int currentViewDirection() const {
			return viewDirection;
		}

		int oppositeViewDirection() const {
			return 1 - viewDirection;
		}

		void clearForSearch() {
			n.resetSourceReachableToSource(augmentingPathAvailableFromPiercing);
			h.resetSourceReachableToSource(augmentingPathAvailableFromPiercing);
		}
		
		void reset() {		// TODO could consolidate with initialize
			viewDirection = 0;
			flowValue = 0;
			n.fullReset();
			h.fullReset();
			sourcePiercingNodes.clear(); targetPiercingNodes.clear();
			trackedMoves.clear();
			augmentingPathAvailableFromPiercing = true;
			hasCut = false;
			mostBalancedCutMode = false;
			cuts.reset(hg.numHyperedges());			//this requires that FlowHypergraph is reset before resetting the CutterState
			borderNodes.reset(hg.numNodes());
			if constexpr (useIsolatedNodes) {
				isolatedNodes.reset();
			}
			partitionWrittenToNodeSet = false;
		}
		
		void initialize(const Node s, const Node t) {
			if (hg.nodeWeight(s) > maxBlockWeight(0) || hg.nodeWeight(t) > maxBlockWeight(1)) {
				throw std::runtime_error("Terminal weight already exceeds max block weight at initialization. Consider setting max block weights per side via hfc.cs.setMaxBlockWeight(  side  )");
			}
			assert(sourcePiercingNodes.empty() && targetPiercingNodes.empty());
			sourcePiercingNodes.emplace_back(s,false);
			settleNode(s, false);
			targetPiercingNodes.emplace_back(t,false);
			flipViewDirection();
			settleNode(t, false);
			flipViewDirection();
			if constexpr (useIsolatedNodes) {
				for (Node u : hg.nodeIDs()) {
					if (hg.degree(u) == 0 && u != s && u != t) {
						isolatedNodes.add(u);
					}
				}
			}
		}
		
		int sideToGrow() const {
			const double imb_s = static_cast<double>(n.sourceReachableWeight) / static_cast<double>(maxBlockWeight(currentViewDirection()));
			const double imb_t = static_cast<double>(n.targetReachableWeight) / static_cast<double>(maxBlockWeight(oppositeViewDirection()));
			return imb_s <= imb_t ? currentViewDirection() : oppositeViewDirection();
		}
		
		bool isBalanced() {
			assert(hasCut);
			assert(!partitionWrittenToNodeSet && "Cannot call isBalanced() once the partition has been written");
			
			const NodeWeight
					sw = n.sourceReachableWeight,		//cannot be split
					tw = n.targetReachableWeight,		//cannot be split
					uw = unclaimedNodeWeight(),			//cannot be split (in current stages. if we integrate proper PCKP heuristics for MBMC this would change)
					iso = isolatedNodes.weight;			//can be split
			
			const NodeWeight
					s_mbw = maxBlockWeight(currentViewDirection()),
					t_mbw = maxBlockWeight(oppositeViewDirection());

			if (sw > s_mbw || tw > t_mbw)					//this is good at late and early stages
				return false;
			if (sw + uw > s_mbw && tw + uw > t_mbw)			//this is good at early stages
				return false;

			{	//quick checks to determine whether balance is possible without invoking SubsetSum, i.e. don't split the isolated nodes.
				bool balanced = false;
				balanced |= sw + uw + iso <= s_mbw;
				balanced |= tw + uw + iso <= t_mbw;
				balanced |= sw + uw <= s_mbw && tw + iso <= t_mbw;
				balanced |= tw + uw <= t_mbw && sw + iso <= s_mbw;
				if (balanced)
					return true;
			}
			
			if constexpr (useIsolatedNodes) {
				timer.start("Balance Check");
				isolatedNodes.updateDPTable();
				
				const NodeWeight
						sRem = s_mbw - sw,
						tRem = t_mbw - tw,
						suw = sw + uw,
						tuw = tw + uw,
						suwRem = suw <= s_mbw ? s_mbw - suw : invalidWeight,
						tuwRem = tuw <= t_mbw ? t_mbw - tuw : invalidWeight;
				
				bool balanced = false;
				
				//sides: (S + U, T) + <ISO> and (S, T + U) + <ISO>
				for (const IsolatedNodes::SummableRange& sr : isolatedNodes.getSumRanges()) {
					if (suwRem != invalidWeight) {
						//S+U not overloaded. Therefore, try (S + U, T) + <ISO>
						
						//allocate as much as possible to S+U, i.e. x = min(suwRem, sr.to), the rest, i.e. iso - x has to go to T
						balanced |= suwRem >= sr.from && tw + (iso - std::min(suwRem, sr.to)) <= t_mbw;
						//analogously, allocate as much as possible to T
						balanced |= tRem >= sr.from && suw + (iso - std::min(tRem, sr.to)) <= s_mbw;
					}
					
					if (tuwRem != invalidWeight) {
						//T+U not overloaded. Therefore, try (S, T + U) + <ISO>
						balanced |= tuwRem >= sr.from && sw + (iso - std::min(tuwRem, sr.to)) <= s_mbw;
						balanced |= sRem >= sr.from && tuw + (iso - std::min(sRem, sr.to)) <= t_mbw;
					}
					
					if (balanced)
						break;
				}
				
				timer.stop("Balance Check");
				return balanced;
			}
			else {
				return false;
			}
		}
		
		NonDynamicCutterState enterMostBalancedCutMode() {
			assert(!mostBalancedCutMode);
			assert(trackedMoves.empty());
			assert(hasCut);
			mostBalancedCutMode = true;	// activates move tracking
			borderNodes.enterMostBalancedCutMode();
			cuts.enterMostBalancedCutMode();
			return { sourcePiercingNodes, targetPiercingNodes, currentViewDirection() };
		}
		
		void resetToFirstBalancedState(NonDynamicCutterState& nds) {
			if (currentViewDirection() != nds.direction) {
				flipViewDirection();
			}
			sourcePiercingNodes = nds.sourcePiercingNodes;
			targetPiercingNodes = nds.targetPiercingNodes;
			revertMoves(0);
			borderNodes.resetForMostBalancedCut();
			cuts.resetForMostBalancedCut();
		}
		
		
		/*
		 * balance criterion with individual part weights is minimize ( w_i / max_w_i ) - 1
		 *
		 * here we minimize alpha under the constraints
		 * 1) a + x <= alpha * max_a
		 * 2) b - x <= alpha * max_b
		 * 3) x in [sr.from, sr.to]
		 *
		 * Finds the optimal assignment weight of isolated nodes to reassign from b to a, assuming b has received all isolated nodes so far.
		 * Optimal under the imbalance definition (part_weight[i] / max_part_weight[i]) - 1
		 */
		static void isolatedWeightAssignmentToFirstMinimizingImbalance(NodeWeight a, NodeWeight max_a,
																	   NodeWeight b, NodeWeight max_b,
																	   const IsolatedNodes::SummableRange& sr,
																	   SimulatedNodeAssignment& assignment) {
			
			auto ddiv = [](const NodeWeight num, const NodeWeight den) -> double {
				return static_cast<double>(num) / static_cast<double>(den);
			};
			auto imb_a = [&](const NodeWeight x) -> double {
				return ddiv(a+x, max_a) - 1.0;
			};
			auto imb_b = [&](const NodeWeight x) -> double {
				return ddiv(b-x, max_b) - 1.0;
			};
			
			const double continuous_x = ddiv(max_a * b - max_b * a, max_a + max_b);
			
			if (continuous_x < sr.from) {
				assignment.trackedIsolatedWeight = sr.from;
			}
			else if (continuous_x > sr.to) {
				assignment.trackedIsolatedWeight = sr.to;
			}
			else {
				// can determine an alpha so that both inequalities are tight
				const NodeWeight x_low = std::floor(continuous_x);
				const NodeWeight x_high = std::ceil(continuous_x);
				const double imb_low = std::max(imb_a(x_low), imb_b(x_low));
				const double imb_high = std::max(imb_a(x_high), imb_b(x_high));
				assignment.trackedIsolatedWeight = imb_low < imb_high ? x_low : x_high;
			}
			
			assignment.imbalanceSourceBlock = imb_a(assignment.trackedIsolatedWeight);
			assignment.imbalanceTargetBlock = imb_b(assignment.trackedIsolatedWeight);
			
			if (!assignment.assignTrackedIsolatedWeightToSource) {
				// in this case a corresponds to the target block --> swap them.
				std::swap(assignment.imbalanceSourceBlock, assignment.imbalanceTargetBlock);
			}
			
		}
		

		// TODO this code isn't very clean unfortunately. let's clean it up.
		
		/*
		 * Simulates settling all isolated and unclaimed nodes to achieve the most balanced partition possible with the current sides
		 * Write that assignment to ReachableNodes if write_partition = true
		 * returns the smallest possible block weight difference and the necessary assignment information
		 */
		SimulatedNodeAssignment mostBalancedIsolatedNodesAssignment() {
			const NodeWeight
					sw = n.sourceReachableWeight,
					tw = n.targetReachableWeight,
					uw = unclaimedNodeWeight(),
					suw = sw + uw,
					tuw = tw + uw,
					s_mbw = maxBlockWeight(currentViewDirection()),
					t_mbw = maxBlockWeight(oppositeViewDirection()),
					t_iso = isolatedNodes.weight;

			SimulatedNodeAssignment sol, sim;
			
			// extracted as lambda to allow using it manually for unweighted nodes or in case the iso DP table is not used
			auto check_combinations = [&](const IsolatedNodes::SummableRange& sr) {
				
				sim.assignUnclaimedToSource = true;
				sim.assignTrackedIsolatedWeightToSource = true;
				isolatedWeightAssignmentToFirstMinimizingImbalance(suw, s_mbw, tw + t_iso, t_mbw, sr, sim);
				if (sim.imbalance() < sol.imbalance()) {
					sol = sim;
				}
			
				sim.assignUnclaimedToSource = false;
				sim.assignTrackedIsolatedWeightToSource = true;
				isolatedWeightAssignmentToFirstMinimizingImbalance(sw, s_mbw, tuw + t_iso, t_mbw, sr, sim);
				if (sim.imbalance() < sol.imbalance()) {
					sol = sim;
				}
			
				if constexpr (useIsolatedNodes) {
					sim.assignUnclaimedToSource = true;
					sim.assignTrackedIsolatedWeightToSource = false;
					isolatedWeightAssignmentToFirstMinimizingImbalance(tw, t_mbw, suw + t_iso, s_mbw, sr, sim);
					if (sim.imbalance() < sol.imbalance()) {
						sol = sim;
					}
				
					sim.assignUnclaimedToSource = false;
					sim.assignTrackedIsolatedWeightToSource = false;
					isolatedWeightAssignmentToFirstMinimizingImbalance(tuw, t_mbw, sw + t_iso, s_mbw, sr, sim);
					if (sim.imbalance() < sol.imbalance()) {
						sol = sim;
					}
				}
			};
			
			if constexpr (useIsolatedNodes) {
				timer.start("Assign Isolated Nodes");
				isolatedNodes.updateDPTable();
				for (const IsolatedNodes::SummableRange& sr : isolatedNodes.getSumRanges()) {
					check_combinations(sr);
				}
				timer.stop("Assign Isolated Nodes");
			}
			else {
				check_combinations(IsolatedNodes::SummableRange(NodeWeight(0), NodeWeight(0)));
			}
			
			sol.numberOfTrackedMoves = trackedMoves.size();
			sol.direction = currentViewDirection();
			
#ifndef NDEBUG
			NodeWeight s = (sol.assignUnclaimedToSource ? suw : sw)
						   + (sol.assignTrackedIsolatedWeightToSource ? sol.trackedIsolatedWeight : t_iso - sol.trackedIsolatedWeight);
			NodeWeight t = (sol.assignUnclaimedToSource ? tw : tuw)
						   + (sol.assignTrackedIsolatedWeightToSource ? t_iso - sol.trackedIsolatedWeight : sol.trackedIsolatedWeight);
			assert(s <= maxBlockWeight(currentViewDirection()));
			assert(t <= maxBlockWeight(oppositeViewDirection()));
			assert(isolatedNodes.isSummable(sol.trackedIsolatedWeight));
#endif
			
			return sol;
		}
		
		// takes the information from mostBalancedIsolatedNodesAssignment()
		// can be an old run, since the DP solution for trackedIsolatedWeight only contains nodes that were isolated during that run
		void writePartition(const SimulatedNodeAssignment& r) {
			assert(!partitionWrittenToNodeSet);
			assert(isBalanced());
			if (currentViewDirection() != r.direction)
				flipViewDirection();

			if constexpr (useIsolatedNodes) {
				auto isoSubset = isolatedNodes.extractSubset(r.trackedIsolatedWeight);
				for (const Node u : isoSubset) {
					assert(!n.isSourceReachable(u) && !n.isTargetReachable(u) && isIsolated(u));
					if (r.assignTrackedIsolatedWeightToSource) {
						n.reach(u); n.settle(u);
					}
					else {
						n.reachTarget(u); n.settleTarget(u);
					}
				}
			}

			for (const Node u : hg.nodeIDs()) {
				if (n.isSourceReachable(u) && !n.isSource(u))
					n.settle(u);
				
				if (n.isTargetReachable(u) && !n.isTarget(u))
					n.settleTarget(u);
				
				if (!n.isSourceReachable(u) && !n.isTargetReachable(u) && !isIsolated(u)) {
					if (r.assignUnclaimedToSource) {
						n.reach(u); n.settle(u);
					}
					else {
						n.reachTarget(u); n.settleTarget(u);
					}
				}
				
				if (isIsolated(u)) {
					assert(isolatedNodes.weight > r.trackedIsolatedWeight);
					if (r.assignTrackedIsolatedWeightToSource) {	// these are untracked isolated nodes
						n.reachTarget(u); n.settleTarget(u);
					}
					else {
						n.reach(u); n.settle(u);
					}
				}
			}
			
			if (currentViewDirection() != 0)
				flipViewDirection();
			
			assert(n.sourceWeight + n.targetWeight == hg.totalNodeWeight());
			partitionWrittenToNodeSet = true;
		}
		
		void writePartition() {
			writePartition(mostBalancedIsolatedNodesAssignment());
		}
		
		void revertMoves(const size_t numberOfTrackedMoves) {
			while (trackedMoves.size() > numberOfTrackedMoves) {
				Move& m = trackedMoves.back();
				if (m.node != invalidNode) {
					assert(m.hyperedge == invalidHyperedge);
					if (m.direction == currentViewDirection())
						n.unsettleSource(m.node);
					else
						n.unsettleTarget(m.node);
				}
				else {
					assert(m.node == invalidNode);
					//for timestamp and distance reachable sets, we would only need unsettleAllPins and unsettleFlowSendingPins, since S and T are disjoint by nature.
					if (currentViewDirection() == m.direction) {
						if (m.t == Move::Type::SettleAllPins)
							h.unsettleAllPins(m.hyperedge);
						else
							h.unsettleFlowSendingPins(m.hyperedge);
					}
					else {
						if (m.t == Move::Type::SettleAllPins)
							h.unsettleAllPinsTarget(m.hyperedge);
						else
							h.unsettleFlowSendingPinsTarget(m.hyperedge);
					}
				}
				trackedMoves.pop_back();
			}
		}
		
		void applyMoves(const std::vector<Move>& moves) {
			for (const Move& m : moves) {
				if (m.node != invalidNode) {
					assert(!n.isSourceReachable(m.node) && !n.isTargetReachable(m.node));
					if (m.direction == currentViewDirection()) {
						n.reach(m.node); n.settle(m.node);
					}
					else {
						n.reachTarget(m.node); n.settleTarget(m.node);
					}
				}
				else {
					assert(m.hyperedge != invalidHyperedge);
					if (currentViewDirection() == m.direction) {
						if (m.t == Move::Type::SettleAllPins)
							h.settleAllPins(m.hyperedge);
						else
							h.settleFlowSendingPins(m.hyperedge);
					}
					else {
						if (m.t == Move::Type::SettleAllPins)
							h.settleAllPinsTarget(m.hyperedge);
						else
							h.settleFlowSendingPinsTarget(m.hyperedge);
					}
				}
			}
		}
		
		std::string toString(bool skip_iso_and_unclaimed = false) {
			std::stringstream os;
			bool flipIt = currentViewDirection() != 0;
			if (flipIt)
				flipViewDirection();
			os << " cut= " << flowValue
			   << " s=" << n.sourceWeight << "|" << n.sourceReachableWeight
			   << " t=" << n.targetWeight << "|" << n.targetReachableWeight;
			if (!skip_iso_and_unclaimed)
			   os << " iso=" << isolatedNodes.weight << " u=" << unclaimedNodeWeight();
			os << " mbw=[" << maxBlockWeight(currentViewDirection()) << " " << maxBlockWeight(oppositeViewDirection()) << "]"
			   << " total=" << hg.totalNodeWeight()
			   << " dir=" << (flipIt ? 1 : 0);
			if (flipIt)
				flipViewDirection();
			return os.str();
		}

		
		void verifyFlowConstraints() {
#ifndef NDEBUG
			Flow sourceExcess = 0, targetExcess = 0;
			for (Node u : hg.nodeIDs()) {
				Flow excess = 0;
				for (auto& he_inc : hg.hyperedgesOf(u))
					excess += hg.flowSent(he_inc);
				if (n.isSource(u))
					sourceExcess += excess;
				else if (n.isTarget(u))
					targetExcess += excess;
				else
					assert(excess == 0);
			}
			assert(sourceExcess >= 0 && targetExcess <= 0);
			assert(hg.flowSent(sourceExcess) == hg.flowReceived(targetExcess));
			assert(sourceExcess == flowValue);
			
			for (Hyperedge e : hg.hyperedgeIDs()) {
				Flow flow_in = 0, flow_out = 0;
				for (Pin& p : hg.pinsOf(e)) {
					assert(std::abs(hg.flowSent(p)) <= hg.capacity(e));
					flow_in += hg.absoluteFlowSent(p);
					flow_out += hg.absoluteFlowReceived(hg.getInHe(p));
				}
				assert(flow_in >= 0);
				assert(flow_in == flow_out);
				assert(flow_in == std::abs(hg.flow(e)));
				assert(flow_in <= hg.capacity(e));
				
				for (Pin& p : hg.pinsSendingFlowInto(e))
					assert(hg.flowSent(p) > 0);
				for (Pin& p : hg.pinsReceivingFlowFrom(e))
					assert(hg.flowReceived(p) > 0);
				for (Pin& p : hg.pinsWithoutFlow(e))
					assert(hg.flowSent(p) == 0);
			}
#endif
		}
		
		void verifyCutPostConditions() {
			assert(hasCut);
			
#ifndef NDEBUG
			
			cuts.sourceSide.cleanUp([&](const Hyperedge& e) { return h.areAllPinsSources(e); });
			Flow expected_flow = 0;
			for (const Hyperedge& e : cuts.sourceSide.entries()) {
				assert(hg.isSaturated(e));
				expected_flow += hg.capacity(e);
			}
			assert(flowValue == expected_flow);

			
#endif
			verifySetInvariants();
			verifyFlowConstraints();
			verifyExtractedCutHyperedgesActuallySplitHypergraph();
			verifyCutInducedByPartitionMatchesExtractedCutHyperedges();
		}
		
		void verifySetInvariants() {
#ifndef NDEBUG
			n.verifyDisjoint();
			n.verifySettledIsSubsetOfReachable();
			h.verifyDisjoint();
			h.verifySettledIsSubsetOfReachable();
			for (Hyperedge e : hg.hyperedgeIDs()) {
				for (const Pin& p : hg.pinsOf(e)) {
					assert(!h.areAllPinsSources(e) || n.isSource(p.pin) || isIsolated(p.pin));
					assert(!h.areAllPinsSourceReachable(e) || n.isSourceReachable(p.pin));
				}
				for (const Pin& p : hg.pinsSendingFlowInto(e)) {
					assert(!h.areFlowSendingPinsSources(e) || n.isSource(p.pin) || isIsolated(p.pin));
					assert(!h.areFlowSendingPinsSourceReachable(e) || n.isSourceReachable(p.pin));
				}
			}
#endif
		}
		
		
		void verifyCutInducedByPartitionMatchesExtractedCutHyperedges() {
#ifndef NDEBUG
			std::vector<Hyperedge> cut_from_partition;
			for (Hyperedge e : hg.hyperedgeIDs()) {
				bool hasSource = false;
				bool hasOther = false;
				for (Pin& p : hg.pinsOf(e)) {
					Node v = p.pin;
					hasSource |= n.isSource(v);
					hasOther |= !n.isSource(v);
				}
				if (hasSource && hasOther) {
					cut_from_partition.push_back(e);
					assert(h.areFlowSendingPinsSources(e));
				}
				
				if (hasSource && !hasOther)
					assert(h.areAllPinsSources(e));
			}
			std::vector<Hyperedge> sorted_cut = cuts.sourceSide.copy();
			std::sort(sorted_cut.begin(), sorted_cut.end());
			assert(sorted_cut == cut_from_partition);
#endif
		}
		
		void verifyCutInducedByPartitionMatchesFlowValue() {
#ifndef NDEBUG
			Flow cut_weight = 0;
			for (Hyperedge e : hg.hyperedgeIDs()) {
				bool hasSource = false;
				bool hasOther = false;
				for (Pin& p : hg.pinsOf(e)) {
					hasSource |= n.isSource(p.pin);
					hasOther |= !n.isSource(p.pin);
				}
				if (hasSource && hasOther) {
					cut_weight += hg.capacity(e);
				}
			}
			assert(flowValue == cut_weight);
#endif
		}
		
		
		void verifyExtractedCutHyperedgesActuallySplitHypergraph() {
#ifndef NDEBUG
			BitVector he_seen(hg.numHyperedges()), node_seen(hg.numNodes());
			LayeredQueue<Node> queue(hg.numNodes());
			for (Node u : hg.nodeIDs()) {
				if (n.isSource(u)) {
					queue.push(u);
					node_seen.set(u);
				}
			}
			
			for (Hyperedge e : cuts.sourceSide.entries())
				he_seen.set(e);
			
			while (!queue.empty()) {
				Node u = queue.pop();
				for (auto& he_inc : hg.hyperedgesOf(u)) {
					Hyperedge e = he_inc.e;
					if (!he_seen[e]) {
						he_seen.set(e);
						for (auto& pin : hg.pinsOf(e)) {
							Node v = pin.pin;
							assert(!n.isTargetReachable(v));
							assert(n.isSourceReachable(v));
							if (!node_seen[v]) {
								node_seen.set(v);
								queue.push(v);
							}
						}
					}
				}
			}
			
			for (Node u : hg.nodeIDs()) {
				if (n.isTargetReachable(u))
					assert(!node_seen[u]);
				if (n.isSourceReachable(u))
					assert(node_seen[u]);
			}
			
			queue.clear();
			he_seen.reset();
			node_seen.reset();
			for (Node u : hg.nodeIDs()) {
				if (n.isTarget(u)) {
					queue.push(u);
					node_seen.set(u);
				}
			}
			
			for (Hyperedge e : cuts.sourceSide.entries())
				he_seen.set(e);
			
			while (!queue.empty()) {
				Node u = queue.pop();
				for (auto& he_inc : hg.hyperedgesOf(u)) {
					Hyperedge e = he_inc.e;
					if (!he_seen[e]) {
						he_seen.set(e);
						for (auto& pin : hg.pinsOf(e)) {
							Node v = pin.pin;
							assert(!n.isSourceReachable(v));
							//no assert(n.isTargetReachable(v)) since we removed the source-side cut
							if (!node_seen[v]) {
								node_seen.set(v);
								queue.push(v);
							}
						}
					}
				}
			}
			
			for (Node u : hg.nodeIDs()) {
				if (n.isTargetReachable(u))
					assert(node_seen[u]);
				if (n.isSourceReachable(u))
					assert(!node_seen[u]);
			}
#endif
		}
		
	};

}
