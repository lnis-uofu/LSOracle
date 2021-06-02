#pragma once

#include <vector>
#include "../util/range.h"
#include <boost/range/irange.hpp>

template<typename T, typename queue_size_type=uint32_t>
class LayeredQueue {
private:
	std::vector<T> queue;
public:
	using size_type = queue_size_type;
	size_type layerfront, layerend, qfront, qend;
	explicit LayeredQueue(const size_type num_elements) : queue(num_elements), layerfront(0), layerend(0), qfront(0), qend(0) { }
	explicit LayeredQueue(const size_t num_elements) : LayeredQueue(static_cast<size_type>(num_elements)) { }
	//Note. Use reinitialize() if you want to keep entries in the underlying vector intact, and ensure these won't be pushed again
	inline void reinitialize(size_type x) { layerfront = x; layerend = x; qfront = x; qend = x; }
	inline void reinitialize() { reinitialize(queueEnd()); }
	inline void clear() { reinitialize(0); }
	inline bool empty() const { return qfront == qend; }
	inline bool currentLayerEmpty() const { return qfront == layerend; }
	inline T pop() { return queue[qfront++]; }
	inline T previousLayerPop() { return queue[layerfront++]; }
	inline void finishNextLayer() { layerend = qend; }
	inline void push(const T x) { assert(qend < queue.size()); queue[qend++] = x; }
	inline bool previousLayerEmpty() const { return layerfront == layerend; }
	inline T capacity() const { return static_cast<T>(queue.size()); }
	inline std::vector<T>& data() { return queue; }
	template<typename Func> inline void forAllEverContainedElements(Func f) { for (size_type i = 0; i < qend; i++) { f(queue[i]); } }
	inline const_range<std::vector<T>> range(size_type __begin, size_type __end) { return { queue.begin() + __begin, queue.cbegin() + __end }; }
	inline const_range<std::vector<T>> currentLayer() { return range(qfront, layerend); }
	inline const_range<std::vector<T>> allElements() { return range(0, qend); }
	inline size_type queueEnd() const { return qend; }
	inline T popBack() { assert(qend > 0); return queue[--qend]; }

	inline decltype(auto) currentLayerIndices() { return boost::irange<size_type>(qfront, layerend); }
	inline void truncateQueue(size_type new_end) {
		qend = std::min(new_end, qend);
		layerend = std::min(new_end, layerend);
		qfront = std::min(new_end, qfront);
		layerfront = std::min(new_end, layerfront);
	}

	inline T elementAt(const size_type pos) const { return queue[pos]; }
	inline void setTo(const size_type pos, T element) { queue[pos] = element; }

	inline T swapFrontToPositionAndPop(size_type pos) {
		std::swap(queue[pos], queue[qfront]);
		return pop();
	}

	std::vector<T> extract() { return std::move(queue); }

	template<typename URBG>
	void shuffleQueue(URBG &&urbg, size_type a, size_type b) {
		std::shuffle(queue.begin() + a, queue.begin() + b, urbg);
	}

	template<typename URBG>
	void shuffleQueue(URBG&& urbg) {
		shuffleQueue(urbg, qfront, qend);
	}

	template<typename URBG>
	void shuffleCurrentLayer(URBG &&urbg) {
		shuffleQueue(urbg, qfront, layerend);
	}



	template<bool resize = true>
	void inject(std::vector<T> external_queue, size_type num_elements) {
		queue = std::move(external_queue);
		layerfront = 0;
		layerend = 0;
		qfront = 0;
		qend = queue.size();
		if (resize)
			queue.resize(num_elements);
	}
};
