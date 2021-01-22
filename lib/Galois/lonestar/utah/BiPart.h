#include <vector>
#include <set>
#include <map>
#include <string.h>
#include <stdlib.h>
#include <numeric>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <array>
#include <unordered_set>

#include "Metis.h"
#include "galois/graphs/Util.h"
#include "galois/Timer.h"
#include "Lonestar/BoilerPlate.h"
#include "galois/LargeArray.h"



namespace cll = llvm::cl;

extern cll::opt<scheduleMode> schedulingMode;

extern cll::opt<bool> mtxInput;
extern cll::opt<bool> weighted;
extern cll::opt<bool> verbose;
extern cll::opt<std::string> outfile;
extern cll::opt<std::string> orderedfile;
extern cll::opt<std::string> permutationfile;
extern cll::opt<std::string> filename;
extern cll::opt<std::string> output;
extern cll::opt<unsigned> csize;
extern cll::opt<unsigned> refiter;
extern cll::opt<double> imbalance;

void Partition(MetisGraph* metisGraph, unsigned coarsenTo, unsigned refineTo); 
int computingCut(GGraph& g); 
int hash(unsigned val); 
std::map<int, int> biparting(std::vector<std::vector<uint32_t>> hedge, uint32_t nodes, int num_part, int num_threads, scheduleMode schedulingMode);
