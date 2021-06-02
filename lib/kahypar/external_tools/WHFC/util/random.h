#pragma once

#include <vector>
#include <algorithm>
#include <random>

//adapted from KaHyPar
namespace whfc {
	class Randomizer {
	public:
		
		Randomizer() : gen(), bool_dist(0, 1), uint_dist(0, std::numeric_limits<uint32_t>::max()), size_t_dist(0, std::numeric_limits<size_t>::max()) { }
		
		bool coinToss() {
			return static_cast<bool>(bool_dist(gen));
		}

		uint32_t randomNumber() {
			return uint_dist(gen);
		}

		uint32_t randomNumber(const uint32_t a, const uint32_t b) {
			return uint_dist(gen, std::uniform_int_distribution<uint32_t>::param_type(a,b));
		}
		
		size_t randomIndex(const size_t a, const size_t b) {
			return size_t_dist(gen, std::uniform_int_distribution<size_t>::param_type(a,b));
		}
		
		template<class T>
		T selectRandomElement(std::vector<T>& range) {
			if (range.empty())
				return T::Invalid();
			return range[randomIndex(0, range.size() - 1)];
		}
		
		template<class T>
		T selectAndRemoveRandomElement(std::vector<T>& range) {
			if (range.empty())
				return T::Invalid();
			size_t it = randomIndex(0, range.size() - 1);
			T res = range[it];
			range[it] = range.back();
			range.pop_back();
			return res;
		}
		
		void setSeed(int seed) {
			gen.seed(seed);
		}

		std::mt19937& getGenerator() {
			return gen;
		}
		
		std::uniform_int_distribution<size_t>& get64BitUintDistribution() {
			return size_t_dist;
		}
		
	protected:
		std::mt19937 gen;
		std::uniform_int_distribution<int> bool_dist;
		std::uniform_int_distribution<uint32_t> uint_dist;
		std::uniform_int_distribution<size_t> size_t_dist;
	};
}
