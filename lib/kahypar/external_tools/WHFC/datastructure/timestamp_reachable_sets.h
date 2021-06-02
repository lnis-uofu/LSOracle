#pragma once

#include <limits>
#include <vector>
#include <cassert>
#include <stdexcept>
#include "reachable_sets_base.h"

namespace whfc {
	template<typename Timestamp>
	class TimestampReachableNodes : public ReachableNodesBase {
	public:
		using Base = ReachableNodesBase;
		using Type = TimestampReachableNodes<Timestamp>;
		static constexpr bool log = false;
		
		TimestampReachableNodes(const FlowHypergraph& hg) : Base(hg), timestamps(hg.numNodes(), unreachableTS) { }

		inline size_t capacity() const { return timestamps.size(); }
		inline bool isSource(const Node u) const { assert(u < capacity()); return timestamps[u] == sourceSettledTS; }
		inline bool isSourceReachable(const Node u) const { return isSource(u) || timestamps[u] == sourceReachableTS; }
		inline bool isTarget(const Node u) const { assert(u < capacity()); return timestamps[u] == targetSettledTS; }
		inline bool isTargetReachable(const Node u) const { return isTarget(u) || timestamps[u] == targetReachableTS; }
		inline void reach(const Node u) { assert(!isSourceReachable(u)); timestamps[u] = sourceReachableTS; Base::reach(u); }
		inline void reachTarget(const Node u) { assert(!isSourceReachable(u) && !isTargetReachable(u)); timestamps[u] = targetReachableTS; Base::reachTarget(u); }
		inline void settle(const Node u) { assert(isSourceReachable(u)); timestamps[u] = sourceSettledTS; Base::settle(u); }
		inline void settleTarget(const Node u) { assert(!isSourceReachable(u) && isTargetReachable(u)); timestamps[u] = targetSettledTS; Base::settleTarget(u); }
		
		inline void unreachSource(const Node u) { assert(isSourceReachable(u) && !isTargetReachable(u)); timestamps[u] = unreachableTS; Base::unreachSource(u); }
		inline void unreachTarget(const Node u) { assert(isTargetReachable(u) && !isSourceReachable(u)); timestamps[u] = unreachableTS; Base::unreachTarget(u); }
		inline void unsettleSource(const Node u) { assert(isSource(u)); Base::unsettleSource(u); unreachSource(u); }
		inline void unsettleTarget(const Node u) { assert(isTarget(u)); Base::unsettleTarget(u); unreachTarget(u); }
		
		void flipViewDirection() {
			LOGGER << "flip";
			std::swap(sourceSettledTS, targetSettledTS);
			std::swap(sourceReachableTS, targetReachableTS);
			Base::flipViewDirection();
		}
		
		void fullReset() {
			std::fill_n(timestamps.begin(), hg.numNodes(), unreachableTS);
			generation = initialTS;
			sourceSettledTS = 1;
			targetSettledTS = 2;
			sourceReachableTS = sourceSettledTS;
			targetReachableTS = targetSettledTS;
			Base::fullReset();
		}

		void resetSourceReachableToSource(bool augmenting_path_available) {
			if (!augmenting_path_available)
				return;
			
			if (generation == std::numeric_limits<Timestamp>::max()) {
				for (const Node u : hg.nodeIDs()) {
					auto& ts = timestamps[u];
					if (ts == targetReachableTS && ts != targetSettledTS)
						ts = initialTS;
					else if (ts != sourceSettledTS && ts != targetSettledTS)
						ts = unreachableTS;
				}
				generation = initialTS + 1;
			}
			else
				generation++;
			
			LOGGER << "reset" << " gen=" << (int)generation << " SR=" << (int)sourceReachableTS << " TR=" << (int)targetReachableTS;
			if (sourceReachableTS == generation || targetReachableTS == generation)	//timestamp conflict: do it again
				resetSourceReachableToSource(augmenting_path_available);
			
			sourceReachableTS = generation;
			Base::resetSourceReachableToSource();
		}
		
		std::string toString() {
			std::stringstream os, sr, tr, s, t;
			sr << "SR = [ ";
			tr << "TR = [ ";
			s << "S = [";
			t << "T = [";
			for (const Node u : hg.nodeIDs()) {
				if (isSource(u))
					s << u << " ";
				if (isTarget(u))
					t << u << " ";
				if (isSourceReachable(u))
					sr << u << " ";
				if (isTargetReachable(u))
					tr << u << " ";
			}
			sr << "]\n";
			tr << "]\n";
			s << "]\n";
			t << "]\n";
			
			os << sr.str() << tr.str() << s.str() << t.str();
			return os.str();
		}

		void verifyDisjoint() const {
			//disjoint by default
		}

		void verifySettledIsSubsetOfReachable() const {
			//is subset by default
		}

	protected:
		static constexpr Timestamp initialTS = 3;
		static constexpr Timestamp unreachableTS = 0;
		Timestamp generation = initialTS;
		Timestamp sourceSettledTS = 1, targetSettledTS = 2;
		Timestamp sourceReachableTS = sourceSettledTS;
		Timestamp targetReachableTS = targetSettledTS;
		std::vector<Timestamp> timestamps;

	};

	template<typename Timestamp>
	class TimestampReachableHyperedges {
	public:
		using Type = TimestampReachableHyperedges<Timestamp>;

		TimestampReachableHyperedges(const FlowHypergraph& hg) : hg(hg), in(hg.numHyperedges(), unreachableTS), out(hg.numHyperedges(), unreachableTS) { }
		
		inline size_t capacity() const { return out.size(); }
		inline bool areAllPinsSources(const Hyperedge e) const { return out[e] == sourceSettledTS; }
		inline bool areAllPinsSourceReachable(const Hyperedge e) const { return areAllPinsSources(e) || out[e] == sourceReachableTS; }
		inline void settleAllPins(const Hyperedge e) { assert(!areAllPinsSources(e)); assert(in[e] != targetSettledTS); out[e] = sourceSettledTS; }
		inline void reachAllPins(const Hyperedge e) { assert(!areAllPinsSourceReachable(e)); assert(in[e] != targetSettledTS); out[e] = sourceReachableTS; }

		inline bool areFlowSendingPinsSources(const Hyperedge e) const { return in[e] == sourceSettledTS; }
		inline bool areFlowSendingPinsSourceReachable(const Hyperedge e) const { return areFlowSendingPinsSources(e) || in[e] == sourceReachableTS; }
		inline void settleFlowSendingPins(const Hyperedge e) { assert(!areFlowSendingPinsSources(e)); in[e] = sourceSettledTS; }
		inline void reachFlowSendingPins(const Hyperedge e) { assert(!areFlowSendingPinsSourceReachable(e)); in[e] = sourceReachableTS; }
		
		void resetSourceReachableToSource(bool augmenting_path_available) {
			if (!augmenting_path_available)
				return;
			
			if (generation == std::numeric_limits<Timestamp>::max()) {
				for (const Hyperedge e : hg.hyperedgeIDs()) {
					auto& t_out = out[e];
					if (t_out == targetReachableTS && t_out != targetSettledTS)
						t_out = initialTS;
					else if (t_out != sourceSettledTS && t_out != targetSettledTS)
						t_out = unreachableTS;
					
					auto& t_in = in[e];
					if (t_in == targetReachableTS && t_in != targetSettledTS)
						t_in = initialTS;
					else if (t_in != sourceSettledTS && t_in != targetSettledTS)
						t_in = unreachableTS;
				}
				generation = initialTS + 1;
			}
			else
				generation++;
			
			if (sourceReachableTS == generation || targetReachableTS == generation) {//timestamp conflict: do it again
				resetSourceReachableToSource(augmenting_path_available);
			}
			sourceReachableTS = generation;
		}
		
		void fullReset() {
			std::fill_n(in.begin(), hg.numHyperedges(), unreachableTS);
			std::fill_n(out.begin(), hg.numHyperedges(), unreachableTS);
			generation = initialTS;
			sourceSettledTS = 1;
			targetSettledTS = 2;
			sourceReachableTS = sourceSettledTS;
			targetReachableTS = targetSettledTS;
		}
		
		void flipViewDirection() {
			std::swap(in, out);
			std::swap(sourceSettledTS, targetSettledTS);
			std::swap(sourceReachableTS, targetReachableTS);
		}

		void verifyDisjoint() const {
			//disjoint by default
		}

		void verifySettledIsSubsetOfReachable() const {
			//is subset by default
		}

	protected:
		const FlowHypergraph& hg;
		static constexpr Timestamp initialTS = 3;
		static constexpr Timestamp unreachableTS = 0;
		Timestamp generation = initialTS;
		Timestamp sourceSettledTS = 1, targetSettledTS = 2;
		Timestamp sourceReachableTS = sourceSettledTS;
		Timestamp targetReachableTS = targetSettledTS;
		std::vector<Timestamp> in, out;
	};


}