#pragma once

#include "reachable_sets_base.h"
#include "bitvector.h"

namespace whfc {

	class BitsetReachableNodes : public ReachableNodesBase {
	public:
		using Base = ReachableNodesBase;
		using Type = BitsetReachableNodes;
		explicit BitsetReachableNodes(const FlowHypergraph& hg) : Base(hg), hg(hg), S(hg.numNodes()), SR(hg.numNodes()), T(hg.numNodes()), TR(hg.numNodes()) { }

		inline size_t capacity() const { return S.size(); }
		inline bool isSource(const Node u) const { return S[u]; }
		inline bool isSourceReachable(const Node u) const { return SR[u]; }
		inline bool isTarget(const Node u) const { return T[u]; }
		inline bool isTargetReachable(const Node u) const { return TR[u]; }
		inline void reach(const Node u) { SR.set(u); Base::reach(u); }
		inline void settle(const Node u) { S.set(u); Base::settle(u); }
		inline void reachTarget(const Node u) { TR.set(u); Base::reachTarget(u); }
		inline void settleTarget(const Node u) { T.set(u); Base::settleTarget(u); }
		
		
		inline void unreachSource(const Node u) { assert(isSourceReachable(u)); SR.reset(u); Base::unreachSource(u); }
		inline void unreachTarget(const Node u) { assert(isTargetReachable(u)); TR.reset(u); Base::unreachTarget(u); }
		inline void unsettleSource(const Node u) { assert(isSource(u)); Base::unsettleSource(u); S.reset(u); unreachSource(u); }
		inline void unsettleTarget(const Node u) { assert(isTarget(u)); Base::unsettleTarget(u); T.reset(u); unreachTarget(u); }


		void flipViewDirection() {
			std::swap(S, T);
			std::swap(SR, TR);
			Base::flipViewDirection();
		}

		void resetSourceReachableToSource(bool augmenting_path_available) {
			//SR = S;
			if (augmenting_path_available) {
				bitvector_copy_first_n(SR, S, hg.numNodes());
				Base::resetSourceReachableToSource();
			}
		}

		void fullReset() {
			Base::fullReset();
			SR.reset(0, hg.numNodes());
			TR.reset(0, hg.numNodes());
			S.reset(0, hg.numNodes());
			T.reset(0, hg.numNodes());
		}
		
		void verifyDisjoint() const {
			assert((SR & TR).none());
			assert((S & T).none());
		}

		void verifySettledIsSubsetOfReachable() const {
			assert(S.is_subset_of(SR));
			assert(T.is_subset_of(TR));
		}

	protected:
		const FlowHypergraph& hg;
	public:
		BitVector S, SR, T, TR;
	};

	class BitsetReachableHyperedges {
	public:
		using Type = BitsetReachableHyperedges;

		explicit BitsetReachableHyperedges(const FlowHypergraph& hg) :
				hg(hg),
				IN_SETTLED_S(hg.numHyperedges()),
				OUT_SETTLED_S(hg.numHyperedges()),
				IN_REACHED_S(hg.numHyperedges()),
				OUT_REACHED_S(hg.numHyperedges()),
				IN_SETTLED_T(hg.numHyperedges()),
				OUT_SETTLED_T(hg.numHyperedges()),
				IN_REACHED_T(hg.numHyperedges()),
				OUT_REACHED_T(hg.numHyperedges())
		{

		}

		inline size_t capacity() const { return IN_SETTLED_S.size(); }
		inline bool areAllPinsSources(const Hyperedge e) const { return OUT_SETTLED_S[e]; }
		inline bool areAllPinsSourceReachable(const Hyperedge e) const { return OUT_REACHED_S[e]; }
		inline void settleAllPins(const Hyperedge e) { assert(!areAllPinsSources(e)); OUT_SETTLED_S.set(e); }
		inline void reachAllPins(const Hyperedge e) { assert(!areAllPinsSourceReachable(e)); OUT_REACHED_S.set(e); }

		inline bool areFlowSendingPinsSources(const Hyperedge e) const { return IN_SETTLED_S[e]; }
		inline bool areFlowSendingPinsSourceReachable(const Hyperedge e) const { return IN_REACHED_S[e]; }
		inline void settleFlowSendingPins(const Hyperedge e) { assert(!areFlowSendingPinsSources(e)); IN_SETTLED_S.set(e); }
		inline void reachFlowSendingPins(const Hyperedge e) { assert(!areFlowSendingPinsSourceReachable(e)); IN_REACHED_S.set(e); }

		void resetSourceReachableToSource(bool augmenting_path_available) {
			//IN_REACHED_S = IN_SETTLED_S;
			//OUT_REACHED_S = OUT_SETTLED_S;
			if (augmenting_path_available) {
				bitvector_copy_first_n(IN_REACHED_S, IN_SETTLED_S, hg.numHyperedges());
				bitvector_copy_first_n(OUT_REACHED_S, OUT_SETTLED_S, hg.numHyperedges());
			}
		}
		
		void fullReset() {
			IN_SETTLED_S.reset(0, hg.numHyperedges());
			OUT_SETTLED_S.reset(0, hg.numHyperedges());
			IN_REACHED_S.reset(0, hg.numHyperedges());
			OUT_REACHED_S.reset(0, hg.numHyperedges());
			IN_SETTLED_T.reset(0, hg.numHyperedges());
			OUT_SETTLED_T.reset(0, hg.numHyperedges());
			IN_REACHED_T.reset(0, hg.numHyperedges());
			OUT_REACHED_T.reset(0, hg.numHyperedges());
		}

		void flipViewDirection() {
			std::swap(IN_SETTLED_S, OUT_SETTLED_T);
			std::swap(OUT_SETTLED_S, IN_SETTLED_T);
			std::swap(IN_REACHED_S, OUT_REACHED_T);
			std::swap(OUT_REACHED_S, IN_REACHED_T);
		}

		void verifyDisjoint() const {
			assert((OUT_REACHED_S & OUT_REACHED_T).none());
			assert((OUT_SETTLED_S & OUT_SETTLED_T).none());
			assert((IN_REACHED_S & IN_REACHED_T).none());
			assert((IN_SETTLED_S & IN_SETTLED_T).none());
		}

		void verifySettledIsSubsetOfReachable() const {
			assert(OUT_SETTLED_S.is_subset_of(OUT_REACHED_S));
			assert(IN_SETTLED_S.is_subset_of(IN_REACHED_S));
			assert(OUT_SETTLED_T.is_subset_of(OUT_REACHED_T));
			assert(IN_SETTLED_T.is_subset_of(IN_REACHED_T));
		}

	protected:
		const FlowHypergraph& hg;
		BitVector IN_SETTLED_S, OUT_SETTLED_S, IN_REACHED_S, OUT_REACHED_S;
		BitVector IN_SETTLED_T, OUT_SETTLED_T, IN_REACHED_T, OUT_REACHED_T;
	};

}
