#pragma once

#include <vector>

template<typename T, typename index_t = uint32_t>
class FixedCapacityStack {
private:
	std::vector<T> stack;
	index_t __size;
public:
	explicit FixedCapacityStack(const index_t num_elements) : stack(num_elements), __size(0) { }
	explicit FixedCapacityStack(const size_t num_elements) : FixedCapacityStack(static_cast<index_t>(num_elements)) { }
	inline void clear() { __size = 0; }
	inline bool empty() const { return __size == 0; }
	inline void popDownTo(const index_t x) { assert(__size >= x + 1); __size = x + 1; }
	inline T pop() { assert(!empty()); return stack[--__size]; }
	inline T& top() { assert(!empty()); return stack[__size - 1]; }
	inline void push(const T& x) { assert(__size < stack.size()); stack[__size++] = x; }
	inline T& at(const index_t t) { return stack[t]; }
	inline index_t size() { return __size; }

	inline index_t capacity() const { return static_cast<index_t>(stack.size()); }
	inline std::vector<T>& data() { return stack; }
};
