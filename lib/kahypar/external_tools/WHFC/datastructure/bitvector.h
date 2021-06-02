#pragma once

#include <vector>
#include <algorithm>
#include <numeric>
#include <boost/dynamic_bitset.hpp>

#include "../util/filter.h"

namespace HackBoostDynamicBitset {
	struct BlockOutputIterator {
		size_t n_blocks;
		const boost::dynamic_bitset<>& copy_from;
		boost::dynamic_bitset<>& copy_to;
		BlockOutputIterator(const size_t n_blocks, const boost::dynamic_bitset<>& copy_from, boost::dynamic_bitset<>& copy_to) :
				n_blocks(n_blocks), copy_from(copy_from), copy_to(copy_to) { }
	};
	
	template <typename Block, typename Allocator>
	size_t num_blocks(const boost::dynamic_bitset<Block, Allocator>& b, size_t pos) {
		if (pos == 0)
			return 0;
		return ((pos-1) / b.bits_per_block) + 1;
	}
	
}

namespace boost {
	template <>
	inline void to_block_range(const dynamic_bitset<>& , HackBoostDynamicBitset::BlockOutputIterator h) {
		assert(h.copy_to.num_blocks() >= h.n_blocks);
		assert(h.copy_from.num_blocks() >= h.n_blocks);
		std::copy_n(h.copy_from.m_bits.begin(), h.n_blocks, h.copy_to.m_bits.begin());
	}
}

namespace whfc {
	
	using BitVector = boost::dynamic_bitset<>;
	
	void bitvector_copy_first_n(BitVector& to, BitVector& from, size_t n) {
		using BOI = HackBoostDynamicBitset::BlockOutputIterator;
		size_t n_blocks = HackBoostDynamicBitset::num_blocks(from, n);
		boost::to_block_range(to, BOI(n_blocks, from, to));
	}

}