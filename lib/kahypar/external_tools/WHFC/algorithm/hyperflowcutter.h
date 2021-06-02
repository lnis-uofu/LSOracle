#pragma once

#include "../datastructure/flow_hypergraph.h"
#include "cutter_state.h"
#include "grow_assimilated.h"
#include "piercing.h"

namespace whfc {

	template<class FlowAlgorithm>
	class HyperFlowCutter {
	public:
		TimeReporter timer;
		FlowHypergraph& hg;
		CutterState<FlowAlgorithm> cs;
		FlowAlgorithm flow_algo;
		Flow upperFlowBound;
		Piercer<FlowAlgorithm> piercer;
		bool find_most_balanced = true;

		static constexpr bool log = false;
		
		HyperFlowCutter(FlowHypergraph& hg, int seed) :
				timer("HyperFlowCutter"),
				hg(hg),
				cs(hg, timer),
				flow_algo(hg),
				upperFlowBound(maxFlow),
				piercer(hg, cs, timer)
		{
			cs.rng.setSeed(seed);
		}

		void reset() {
			cs.reset();
			piercer.reset();
			flow_algo.reset();
			upperFlowBound = maxFlow;
			//timer.clear();
		}
		
		void setPiercingNode(const Node piercingNode) {
			cs.augmentingPathAvailableFromPiercing = cs.n.isTargetReachable(piercingNode);
			cs.sourcePiercingNodes.clear();
			cs.sourcePiercingNodes.emplace_back(piercingNode, cs.n.isTargetReachable(piercingNode));
			cs.settleNode(piercingNode);
			cs.hasCut = false;
		}
		
		bool pierce(bool reject_piercing_if_it_creates_an_augmenting_path = false) {
			Node piercingNode = piercer.findPiercingNode();
			if (piercingNode == invalidNode)
				return false;
			if (reject_piercing_if_it_creates_an_augmenting_path && cs.n.isTargetReachable(piercingNode))
				return false;
			setPiercingNode(piercingNode);
			return true;
		}
		
		//for flow-based interleaving
		bool advanceOneFlowIteration(bool reject_piercing_if_it_creates_an_augmenting_path = false) {
			const bool pierceInThisIteration = cs.hasCut;
			if (pierceInThisIteration) {
				const bool early_reject = !pierce(reject_piercing_if_it_creates_an_augmenting_path);
				if (early_reject) {
					return false;
				}
			}
			
			timer.start("Flow");
			if (cs.augmentingPathAvailableFromPiercing) {
				timer.start("Augment", "Flow");
				if (pierceInThisIteration) {
					cs.flowValue += flow_algo.recycleDatastructuresFromGrowReachablePhase(cs);	//the flow due to recycled datastructures does not matter when deciding whether we have a cut available
				}
				Flow flow_diff = flow_algo.growFlowOrSourceReachable(cs);
				cs.flowValue += flow_diff;
				cs.hasCut = flow_diff == 0;
				timer.stop("Augment");
				
				if (cs.hasCut) {
					cs.flipViewDirection();
					timer.start("Grow Backward Reachable", "Flow");
					flow_algo.growReachable(cs);
					timer.stop("Grow Backward Reachable");
				}
			}
			else {
				timer.start("Grow Reachable due to AAP", "Flow");
				flow_algo.growReachable(cs);		//don't grow target reachable
				timer.stop("Grow Reachable due to AAP");
				cs.hasCut = true;
			}
			timer.stop("Flow");
			
			cs.verifyFlowConstraints();
			
			if (cs.hasCut) {
				cs.verifySetInvariants();
				if (cs.sideToGrow() != cs.currentViewDirection()) {
					cs.flipViewDirection();
				}
				timer.start("Grow Assimilated");
				GrowAssimilated<FlowAlgorithm>::grow(cs, flow_algo.getScanList());
				timer.stop("Grow Assimilated");
				cs.verifyCutPostConditions();
				LOGGER << cs.toString();
			}
			
			return true;
		}

		bool runUntilBalancedOrFlowBoundExceeded(const Node s, const Node t) {
			cs.initialize(s,t);
			bool piercingFailedOrFlowBoundReachedWithNonAAPPiercingNode = false;
			bool has_balanced_cut = false;
			
			while (cs.flowValue <= upperFlowBound && !has_balanced_cut) {
				piercingFailedOrFlowBoundReachedWithNonAAPPiercingNode = !advanceOneFlowIteration(cs.flowValue == upperFlowBound);
				if (piercingFailedOrFlowBoundReachedWithNonAAPPiercingNode)
					break;
				has_balanced_cut = cs.hasCut && cs.isBalanced(); //no cut ==> run and don't check for balance.
			}

			LOGGER << V(has_balanced_cut) << V(cs.flowValue) << V(upperFlowBound);

			if (has_balanced_cut && cs.flowValue <= upperFlowBound) {
				assert(cs.sideToGrow() == cs.currentViewDirection());
				const double imb_S_U_ISO = static_cast<double>(hg.totalNodeWeight() - cs.n.targetReachableWeight) / static_cast<double>(cs.maxBlockWeight(cs.currentViewDirection()));
				const double imb_T = static_cast<double>(cs.n.targetReachableWeight) / static_cast<double>(cs.maxBlockWeight(cs.oppositeViewDirection()));
				const bool better_balance_impossible = cs.unclaimedNodeWeight() == 0 || imb_S_U_ISO <= imb_T;
				LOGGER << V(better_balance_impossible);
				if (find_most_balanced && !better_balance_impossible) {
					mostBalancedCut();
				}
				else {
					cs.writePartition();
				}
				
				LOGGER << cs.toString(true);
				cs.verifyCutInducedByPartitionMatchesFlowValue();
			}
			
			// Turn back to initial view direction
			if (cs.currentViewDirection() != 0) {
				cs.flipViewDirection();
			}
			
			return !piercingFailedOrFlowBoundReachedWithNonAAPPiercingNode && cs.flowValue <= upperFlowBound && has_balanced_cut;
		}
		
		bool findNextCut(bool reject_piercing_if_it_creates_an_augmenting_path = false) {
			if (cs.hasCut) {	// false on the first call, true on subsequent calls.
				const bool early_reject = !pierce(reject_piercing_if_it_creates_an_augmenting_path);
				if (early_reject) {
					return false;
				}
			}
			
			if (cs.augmentingPathAvailableFromPiercing) {
				cs.hasCut = flow_algo.exhaustFlow(cs);
				if (cs.hasCut) {
					cs.flipViewDirection();
					flow_algo.growReachable(cs);
				}
			}
			else {
				flow_algo.growReachable(cs);	// don't grow target reachable
				cs.hasCut = true;
			}
			cs.verifyFlowConstraints();
			
			if (cs.hasCut) {
				cs.verifySetInvariants();
				if (cs.sideToGrow() != cs.currentViewDirection()) {
					cs.flipViewDirection();
				}
				GrowAssimilated<FlowAlgorithm>::grow(cs, flow_algo.getScanList());
				cs.verifyCutPostConditions();
			}
			
			return cs.hasCut && cs.flowValue <= upperFlowBound;
		}
		
		
		/*
		 * Equivalent to runUntilBalancedOrFlowBoundExceeded(s,t) except that it does not use the flow-based interleaving that is necessary when running multiple HFC instances
		 */
		bool enumerateCutsUntilBalancedOrFlowBoundExceeded(const Node s, const Node t) {
			flow_algo.upperFlowBound = upperFlowBound;
			cs.initialize(s,t);
			bool has_balanced_cut_below_flow_bound = false;
			while (!has_balanced_cut_below_flow_bound && findNextCut(cs.flowValue == upperFlowBound)) {
				has_balanced_cut_below_flow_bound |= cs.isBalanced();
			}
			
			if (has_balanced_cut_below_flow_bound) {
				assert(cs.sideToGrow() == cs.currentViewDirection());
				const double imb_S_U_ISO = static_cast<double>(hg.totalNodeWeight() - cs.n.targetReachableWeight) / static_cast<double>(cs.maxBlockWeight(cs.currentViewDirection()));
				const double imb_T = static_cast<double>(cs.n.targetReachableWeight) / static_cast<double>(cs.maxBlockWeight(cs.oppositeViewDirection()));
				const bool better_balance_impossible = cs.unclaimedNodeWeight() == 0 || imb_S_U_ISO <= imb_T;
				LOGGER << V(better_balance_impossible);
				if (find_most_balanced && !better_balance_impossible) {
					mostBalancedCut();
				}
				else {
					cs.writePartition();
				}
				
				LOGGER << cs.toString(true);
				cs.verifyCutInducedByPartitionMatchesFlowValue();
			}
			
			if (cs.currentViewDirection() != 0) {
				cs.flipViewDirection();
			}
			
			return has_balanced_cut_below_flow_bound;
		}
		
		void mostBalancedCut() {
			timer.start("MBMC");
			
			LOGGER << "MBC Mode";
			
			//settle target reachable nodes, so we don't have to track them in the moves
			cs.flipViewDirection();
			GrowAssimilated<FlowAlgorithm>::grow(cs, flow_algo.getScanList());
			cs.verifyCutPostConditions();
			cs.flipViewDirection();
			
			assert(cs.n.sourceReachableWeight == cs.n.sourceWeight);
			assert(cs.n.targetReachableWeight == cs.n.targetWeight);
			
			NonDynamicCutterState first_balanced_state = cs.enterMostBalancedCutMode();
			SimulatedNodeAssignment initial_sol = cs.mostBalancedIsolatedNodesAssignment();
			std::vector<Move> best_moves;
			SimulatedNodeAssignment best_sol = initial_sol;
			
			const size_t mbc_iterations = 7;
			for (size_t i = 0; i < mbc_iterations && !best_sol.isPerfectlyBalanced(); ++i) {
				LOGGER << "MBC it" << i;
				assert(cs.sideToGrow() == cs.currentViewDirection());
				SimulatedNodeAssignment sol = best_sol;
				while (!sol.isPerfectlyBalanced() && pierce(true)) {
					GrowAssimilated<FlowAlgorithm>::grow(cs, flow_algo.getScanList(), true);
					cs.hasCut = true;
					cs.verifyCutPostConditions();
					LOGGER << cs.toString();
					
					if (cs.sideToGrow() != cs.currentViewDirection()) {
						cs.flipViewDirection();
					}
					
					SimulatedNodeAssignment sim = cs.mostBalancedIsolatedNodesAssignment();
					if (sim.imbalance() < sol.imbalance()) {
						sol = sim;
					}
				}
				
				if (sol.imbalance() < best_sol.imbalance()) {
					best_sol = sol;
					cs.revertMoves(sol.numberOfTrackedMoves);
					best_moves = cs.trackedMoves;
				}
				cs.resetToFirstBalancedState(first_balanced_state);
			}
			
			cs.applyMoves(best_moves);
			cs.writePartition(best_sol);
			
			timer.stop("MBMC");
		}
		
		
	};

}
