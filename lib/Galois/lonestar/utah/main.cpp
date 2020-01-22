#include <iostream>
#include "BiPart.h"

int main(int argc, char** argv) {
	
	std::string filename = "sin.hpg";
	std::string output = "sin_test.txt";
	int num_part = 19;
	int num_threads = 14;
	scheduleMode mode = PP;
  biparting(filename, output, num_part, num_threads, mode);
  return 0;
};
