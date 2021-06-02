#pragma once

#include "bitset_reachable_sets.h"
#include "timestamp_reachable_sets.h"

namespace whfc {

	class ReachableNodesChecker : public ReachableNodesBase {
	public:
		using Base = ReachableNodesBase;
		explicit ReachableNodesChecker(const FlowHypergraph& hg) : Base(hg), bits(hg), timestamps(hg) { }
		
		inline size_t capacity() const {
			assert(bits.capacity() == timestamps.capacity());
			return bits.capacity();
		}
		
		inline bool isSource(const Node u) const {
			assert(bits.isSource(u) == timestamps.isSource(u));
			return bits.isSource(u);
		}
		
		inline bool isSourceReachable(const Node u) const {
			assert(bits.isSourceReachable(u) == timestamps.isSourceReachable(u));
			return bits.isSourceReachable(u);
		}
		inline bool isTarget(const Node u) const {
			assert(bits.isTarget(u) == timestamps.isTarget(u));
			return bits.isTarget(u);
		}
		inline bool isTargetReachable(const Node u) const {
			assert(bits.isTargetReachable(u) == timestamps.isTargetReachable(u));
			return bits.isTargetReachable(u);
		}
		inline void reach(const Node u) {
			Base::reach(u);
			bits.reach(u);
			timestamps.reach(u);
		}
		inline void settle(const Node u) {
			Base::settle(u);
			bits.settle(u);
			timestamps.settle(u);
		}
		
		inline void reachTarget(const Node u) {
			Base::reachTarget(u);
			bits.reachTarget(u);
			timestamps.reachTarget(u);
		}
		
		inline void settleTarget(const Node u) {
			Base::settleTarget(u);
			bits.settleTarget(u);
			timestamps.settleTarget(u);
		}
		
		inline void unreachSource(const Node u) {
			Base::unreachSource(u);
			bits.unreachSource(u);
			timestamps.unreachSource(u);
		}
		
		inline void unreachTarget(const Node u) {
			Base::unreachTarget(u);
			bits.unreachTarget(u);
			timestamps.unreachTarget(u);
		}
		
		inline void unsettleSource(const Node u) {
			assert(isSource(u)); Base::unsettleSource(u);
			bits.unsettleTarget(u);
			timestamps.unsettleTarget(u);
		}
		inline void unsettleTarget(const Node u) {
			assert(isTarget(u)); Base::unsettleTarget(u);
			bits.unsettleTarget(u);
			timestamps.unsettleTarget(u);
		}
		
		
		void flipViewDirection() {
			Base::flipViewDirection();
			bits.flipViewDirection();
			timestamps.flipViewDirection();
		}
		
		void resetSourceReachableToSource(bool augmenting_path_available) {
			Base::resetSourceReachableToSource();
			bits.resetSourceReachableToSource(augmenting_path_available);
			timestamps.resetSourceReachableToSource(augmenting_path_available);
		}
		
		void fullReset() {
			Base::fullReset();
			bits.fullReset();
			timestamps.fullReset();
		}
		
		void verifyDisjoint() const {
			bits.verifyDisjoint();
			timestamps.verifyDisjoint();
		}
		
		void verifySettledIsSubsetOfReachable() const {
			bits.verifySettledIsSubsetOfReachable();
			timestamps.verifySettledIsSubsetOfReachable();
		}
		
	private:
		BitsetReachableNodes bits;
		TimestampReachableNodes<uint8_t> timestamps;
	};
	
	
	class ReachableHyperedgesChecker {
	public:
		explicit ReachableHyperedgesChecker(const FlowHypergraph& hg) : bits(hg), timestamps(hg) { }
		
		inline size_t capacity() const {
			assert(bits.capacity() == timestamps.capacity());
			return bits.capacity();
		}
		
		inline bool areAllPinsSources(const Hyperedge e) const {
			assert(bits.areAllPinsSources(e) == timestamps.areAllPinsSources(e));
			return bits.areAllPinsSources(e);
		}
		
		inline bool areAllPinsSourceReachable(const Hyperedge e) const {
			assert(bits.areAllPinsSourceReachable(e) == timestamps.areAllPinsSourceReachable(e));
			return bits.areAllPinsSourceReachable(e);
		}
		
		inline void settleAllPins(const Hyperedge e) {
			bits.settleAllPins(e);
			timestamps.settleAllPins(e);
		}
		
		inline void reachAllPins(const Hyperedge e) {
			bits.reachAllPins(e);
			timestamps.reachAllPins(e);
		}
		
		inline bool areFlowSendingPinsSources(const Hyperedge e) const {
			assert(bits.areFlowSendingPinsSources(e) == timestamps.areFlowSendingPinsSources(e));
			return bits.areFlowSendingPinsSources(e);
		}
		
		inline bool areFlowSendingPinsSourceReachable(const Hyperedge e) const {
			assert(bits.areFlowSendingPinsSourceReachable(e) == timestamps.areFlowSendingPinsSourceReachable(e));
			return bits.areFlowSendingPinsSourceReachable(e);
		}
		
		inline void settleFlowSendingPins(const Hyperedge e) {
			bits.settleFlowSendingPins(e);
			timestamps.settleFlowSendingPins(e);
		}
		
		inline void reachFlowSendingPins(const Hyperedge e) {
			bits.reachFlowSendingPins(e);
			timestamps.reachFlowSendingPins(e);
		}
		
		void resetSourceReachableToSource(bool augmenting_path_available) {
			bits.resetSourceReachableToSource(augmenting_path_available);
			timestamps.resetSourceReachableToSource(augmenting_path_available);
		}
		
		void fullReset() {
			bits.fullReset();
			timestamps.fullReset();
		}
		
		void verifyDisjoint() const {
			bits.verifyDisjoint();
			timestamps.verifyDisjoint();
		}
		
		void verifySettledIsSubsetOfReachable() const {
			bits.verifySettledIsSubsetOfReachable();
			timestamps.verifySettledIsSubsetOfReachable();
		}
		void flipViewDirection() {
			bits.flipViewDirection();
			timestamps.flipViewDirection();
		}
	
	private:
		BitsetReachableHyperedges bits;
		TimestampReachableHyperedges<uint8_t> timestamps;
	};
	
	
}