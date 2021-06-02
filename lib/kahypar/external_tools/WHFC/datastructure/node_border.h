#pragma once

#include "../definitions.h"
#include "bitvector.h"

namespace whfc {
	
class DistanceFromCut {
public:
	DistanceFromCut(const size_t initialN) : distance(initialN, 0) { }
	
	HopDistance getHopDistanceFromCut(const Node x) const {
		return std::max(multiplier * distance[x], 0); // distances of vertices on opposite side are negative --> throw away
	}
	
	HopDistance getHopDistanceFromCut(const Node x, const int own_multiplier) const {
		return std::max(own_multiplier * distance[x], 0); // distances of vertices on opposite side are negative --> throw away
	}
	
	HopDistance& operator[](const size_t idx) {
		return distance[idx];
	}
	
	int multiplier = -1;
	std::vector<HopDistance> distance;
};

class NodeBorder {
public:
	NodeBorder(const size_t initialN, const DistanceFromCut& dfc, const int multiplier) :
			was_added(initialN),
			buckets(10, {Bucket(), Bucket()}),
			maxOccupiedBucket({-1,-1}),
			minOccupiedBucket({0,0}),
			backupMaxOccupiedBucket({-1,-1}),
			backupMinOccupiedBucket({0,0}),
			removed_during_most_balanced_cut_mode({Bucket(), Bucket()}),
			distance(dfc),
			multiplier(multiplier)
	{
	
	}
	
	bool wasAdded(const Node u) const {
		return was_added[u];
	}
	
	void add(const Node u, bool is_tr) {
		assert(!mostBalancedCutMode || !is_tr);
		assert(!wasAdded(u));
		was_added.set(u);
		const HopDistance d = getDistance(u);
		is_tr |= mostBalancedCutMode;				//reuse target_reachable_bucket_index buckets for nodes inserted during mbc
		const auto i = static_cast<Index>(is_tr);
		insertIntoBucket(u, i, d);
	}
	
	void insertIntoBucket(const Node u, const Index i, const HopDistance d) {
		buckets[d][i].push_back(u);
		maxOccupiedBucket[i] = std::max(maxOccupiedBucket[i], d);
		minOccupiedBucket[i] = std::min(minOccupiedBucket[i], d);
	}
	
	void reset(const size_t newN) {
		mostBalancedCutMode = false;
		was_added.reset(0, newN);
		
		for (Index i = 0; i < 2; ++i) {
			clearBuckets(i);
			assert(removed_during_most_balanced_cut_mode[i].empty());
		}
		verifyBucketsAreClean();
		
		HopDistance maxDistance = 0;
		for (Node i(0); i < newN; ++i) {
			maxDistance = std::max(maxDistance, getDistance(i));
		}
		if (static_cast<size_t>(maxDistance) >= buckets.size()) {
			buckets.resize(static_cast<size_t>(maxDistance + 1));
		}
	}
	
	void resetForMostBalancedCut() {
		// remove everything that was added during most balanced cut and is still in the buckets
		for (HopDistance d = minOccupiedBucket[most_balanced_cut_bucket_index]; d <= maxOccupiedBucket[most_balanced_cut_bucket_index]; ++d) {
			for (Node u : buckets[d][most_balanced_cut_bucket_index])
				was_added.reset(u);
			buckets[d][most_balanced_cut_bucket_index].clear();
		}
		
		// reinsert the non-target-reachable nodes that were removed during most balanced cut
		for (Node u : removed_during_most_balanced_cut_mode[not_target_reachable_bucket_index]) {
			buckets[getDistance(u)][not_target_reachable_bucket_index].push_back(u);
		}
		
		for (Node u : removed_during_most_balanced_cut_mode[most_balanced_cut_bucket_index]) {
			was_added.reset(u);
		}
		
		removed_during_most_balanced_cut_mode[not_target_reachable_bucket_index].clear();
		removed_during_most_balanced_cut_mode[most_balanced_cut_bucket_index].clear();
		
		maxOccupiedBucket = backupMaxOccupiedBucket;
		minOccupiedBucket = backupMinOccupiedBucket;
	}
	
	using Bucket = std::vector<Node>;
	
	
	void clearBuckets(const Index i) {
		for (HopDistance d = minOccupiedBucket[i]; d <= maxOccupiedBucket[i]; ++d)
			buckets[d][i].clear();
		minOccupiedBucket[i] = 0;
		maxOccupiedBucket[i] = -1;
	}
	
	void enterMostBalancedCutMode () {
		mostBalancedCutMode = true;
		clearBuckets(target_reachable_bucket_index);
		backupMaxOccupiedBucket = maxOccupiedBucket;
		backupMinOccupiedBucket = minOccupiedBucket;
	}
	
	HopDistance getDistance(const Node u) const {
		return distance.getHopDistanceFromCut(u, multiplier);
	}
	
	static constexpr Index position_for_added_but_removed = invalidIndex - 1;
	BitVector was_added;
	
	static constexpr Index not_target_reachable_bucket_index = 0, target_reachable_bucket_index = 1, most_balanced_cut_bucket_index = 1;
	std::vector< std::array<Bucket, 2> > buckets;
	
	std::array<HopDistance, 2> maxOccupiedBucket, minOccupiedBucket, backupMaxOccupiedBucket, backupMinOccupiedBucket;
	
	std::array<Bucket, 2> removed_during_most_balanced_cut_mode;
	
	const DistanceFromCut& distance;
	
	int multiplier;
	bool mostBalancedCutMode = false;
	
private:
	
	void verifyBucketsAreClean() {
#ifndef NDEBUG
		for (auto& bb : buckets) {
			for (Bucket& b : bb) {
				assert(b.empty());
			}
		}
#endif
	}
	
};

class NodeBorders {
public:
	
	NodeBorders(const size_t initialN) : distance(initialN),
										 sourceSide(std::make_unique<NodeBorder>(initialN, distance, -1)),
										 targetSide(std::make_unique<NodeBorder>(initialN, distance, 1)) { }
	
	void flipViewDirection() {
		std::swap(sourceSide, targetSide);
		distance.multiplier *= -1;
	}
	
	void reset(const size_t newN) {
		assert(sourceSide->multiplier == -1);
		sourceSide->reset(newN);
		targetSide->reset(newN);
	}
	
	void enterMostBalancedCutMode() {
		sourceSide->enterMostBalancedCutMode();
		targetSide->enterMostBalancedCutMode();
	}
	
	void resetForMostBalancedCut() {
		sourceSide->resetForMostBalancedCut();
		targetSide->resetForMostBalancedCut();
	}
	
	DistanceFromCut distance;
	std::unique_ptr<NodeBorder> sourceSide, targetSide;
};

}