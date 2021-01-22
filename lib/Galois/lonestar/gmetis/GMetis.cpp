/*
 * This file belongs to the Galois project, a C++ library for exploiting parallelism.
 * The code is being released under the terms of the 3-Clause BSD License (a
 * copy is located in LICENSE.txt at the top-level directory).
 *
 * Copyright (C) 2018, The University of Texas at Austin. All rights reserved.
 * UNIVERSITY EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES CONCERNING THIS
 * SOFTWARE AND DOCUMENTATION, INCLUDING ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR ANY PARTICULAR PURPOSE, NON-INFRINGEMENT AND WARRANTIES OF
 * PERFORMANCE, AND ANY WARRANTY THAT MIGHT OTHERWISE ARISE FROM COURSE OF
 * DEALING OR USAGE OF TRADE.  NO WARRANTY IS EITHER EXPRESS OR IMPLIED WITH
 * RESPECT TO THE USE OF THE SOFTWARE OR DOCUMENTATION. Under no circumstances
 * shall University be liable for incidental, special, indirect, direct or
 * consequential damages or loss of profits, interruption of business, or
 * related expenses which may arise from use of Software or Documentation,
 * including but not limited to those resulting from defects in Software and/or
 * Documentation, or loss or inaccuracy of data of any kind.
 */

#include <vector>
#include <set>
#include <map>
#include <iostream>
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
//#include "GraphReader.h"
#include "Lonestar/BoilerPlate.h"
#include "galois/graphs/FileGraph.h"
#include "galois/LargeArray.h"

namespace cll = llvm::cl;

static const char* name = "HYPAR";
static const char* desc =
    "Partitions a hypergraph into K parts and minimizing the graph cut";
static const char* url = "HyPar";

static cll::opt<scheduleMode> schedulingMode(
    cll::desc("Choose a inital scheduling mode:"),
    cll::values(clEnumVal(PLD, "PLD"), clEnumVal(PP, "PP"), clEnumVal(WD, "WD"),
                clEnumVal(RI, "RI"), clEnumVal(MRI, "MRI"),clEnumVal(MDEG, "MDEG"),clEnumVal(DEG, "DEG"),clEnumVal(MWD, "MWD"),clEnumVal(HIS, "HIS"),clEnumValEnd),
    cll::init(PLD));

static cll::opt<bool>
    mtxInput("mtxinput",
             cll::desc("Use text mtx files instead of binary galois gr files"),
             cll::init(false));
static cll::opt<bool> weighted("weighted", cll::desc("weighted"),
                               cll::init(false));
static cll::opt<bool>
    verbose("verbose",
            cll::desc("verbose output (debugging mode, takes extra time)"),
            cll::init(false));
static cll::opt<std::string> outfile("output",
                                     cll::desc("output partition file name"));
static cll::opt<std::string>
    orderedfile("ordered", cll::desc("output ordered graph file name"));
static cll::opt<std::string>
    permutationfile("permutation", cll::desc("output permutation file name"));
static cll::opt<std::string> filename(cll::Positional,
                                      cll::desc("<input file>"), cll::Required);
static cll::opt<std::string> output(cll::Positional,
                                      cll::desc("<output file>"), cll::Required);

static cll::opt<int> num_part("p", cll::desc("Number of partitions (default value 1)"), cll::init(1));

static cll::opt<unsigned> csize(cll::Positional,
                                   cll::desc("<size of coarsest graph>")
                                   );

static cll::opt<unsigned> refiter(cll::Positional,
                                   cll::desc("<number of iterations in ref>")
                                   );
static cll::opt<double> imbalance(
    "balance",
    cll::desc("Fraction deviated from mean partition size (default 0.01)"),
    cll::init(0.5));

// const double COARSEN_FRACTION = 0.9;

int cutsize(GGraph& g) { 
  unsigned size = std::distance(g.cellList().begin(), g.cellList().end());
  unsigned sizen = std::distance(g.getNets().begin(), g.getNets().end());
  int cutsize = 0;
  std::vector<int> cells;
  for (auto n : g.getNets()) { 
    bool cut_status = false;
    for (auto e : g.edges(n)) {
      auto cell1 = g.getEdgeDst(e);
    for (auto c : g.edges(n)) {
        auto cell2 = g.getEdgeDst(c);
        if(g.getData(cell1).getPart() != g.getData(cell2).getPart() && cell1 != cell2) {
          cutsize++;
          cut_status = true;
          break;
        }
      }
      if (cut_status == true)
        break;
    }
  }
  return cutsize;
}
/**
 * Partitioning 
 */
void Partition(MetisGraph* metisGraph, unsigned coarsenTo, unsigned refineTo) {
  //galois::StatTimer TM;
  //TM.start();

  //galois::StatTimer T("CoarsenSEP");
  //T.start();
  MetisGraph* mcg = coarsen(metisGraph, coarsenTo, schedulingMode);
  //T.stop();

  //galois::StatTimer T2("PartitionSEP");
  int cuts = std::numeric_limits<int>::max();
  //T2.start();
  partition(mcg);
  //T2.stop();


  //galois::StatTimer T3("Refine");
  //T3.start();
  refine(mcg, refineTo);
  //T3.stop();
  //int one = 0;
  //int zero = 0;
  //std::cout << "clustering:," << T2.get() << '\n';
  //std::cout << "coarsen:," << T.get() << "\n";
  //std::cout << "Refinement:," << T3.get() << "\n";
  return;
}

int computingCut(GGraph& g) {

  GNodeBag bag;
  galois::GAccumulator<int> accum;
  galois::do_all(galois::iterate(g.getNets()),
        [&](GNode n) {
            std::set<int> pps;
            for (auto x : g.edges(n)) {
              auto cc = g.getEdgeDst(x);
              int part = g.getData(cc).getPart();
              pps.insert(part);

            }
            if (pps.size() > 1) accum += (pps.size() - 1);
        },
        galois::loopname("cutsize"));
  return accum.reduce();
}

int computingBalance(GGraph& g, unsigned k) {
  unsigned bal[k];
  unsigned size = 0;
  for (int i = 0; i < k; i++) bal[i] = 0;
  for (auto c : g.cellList()) {
    size++;
    int part = g.getData(c).getPart();
    bal[part]++;
  }
  int balance = size / k;
  int lo = balance - balance/10;
  int hi = balance + balance/10;
  for (int i = 0 ; i < k; i++) {
    std::cout<<bal[i]<<" ";
  } 
  std::cout<<"\n";
  return balance;
}

int computingBalance(GGraph& g) {
  int zero = 0, one = 0;
  for (auto c : g.cellList()) {
    int part = g.getData(c).getPart();
    if (part == 0) zero++;
    else one++;
  }
  return std::abs(zero - one);
}
// printGraphBeg(*graph)

typedef galois::graphs::FileGraph FG;
typedef FG::GraphNode FN;
template <typename GNode, typename Weights>
struct order_by_degree {
  GGraph& graph;
  Weights& weights;
  order_by_degree(GGraph& g, Weights& w) : graph(g), weights(w) {}
  bool operator()(const GNode& a, const GNode& b) {
    uint64_t wa = weights[a];
    uint64_t wb = weights[b];
    int pa      = graph.getData(a, galois::MethodFlag::UNPROTECTED).getPart();
    int pb      = graph.getData(b, galois::MethodFlag::UNPROTECTED).getPart();
    if (pa != pb) {
      return pa < pb;
    }
    return wa < wb;
  }
};

typedef galois::substrate::PerThreadStorage<std::map<GNode, uint64_t>>
    PerThreadDegInfo;


std::map<uint64_t, uint64_t> cellToNet(std::map< uint64_t, std::vector< uint64_t> > netToCell) {
  std::map<uint64_t, uint64_t> celltonet;
  for (auto n : netToCell) {
    for (auto c : n.second) {
      celltonet[c]++;
    }
  }
  return celltonet;
}

int hash(unsigned val) {
  unsigned long int seed = val * 1103515245 + 12345;
  return((unsigned)(seed/65536) % 32768);
}

int main(int argc, char** argv) {
  galois::SharedMemSys G;
  LonestarStart(argc, argv, name, desc, url);

 // srand(-1);
  MetisGraph metisGraph;
  GGraph& graph = *metisGraph.getGraph();
  std::ifstream f(filename.c_str());
  std::string line;
  std::getline(f, line);
  std::stringstream ss(line);
  int i1, i2;
  ss >> i1 >> i2;
  const int hedges = i1, nodes = i2;
  printf("hedges: %d\n", hedges);
  printf("nodes: %d\n\n", nodes);

  // read rest of input and initialize hedges (build hgraph)
  std::unordered_set<int>* const hedge = new std::unordered_set<int> [hedges];
  int cnt = 0, entries = 0;
  while (std::getline(f, line)) {
    if (cnt >= hedges) {printf("ERROR: too many lines in input file\n"); exit(-1);}
    std::stringstream ss(line);
    int val;
    while (ss >> val) {
      if ((val < 1) || (val > nodes)) {printf("ERROR: node value %d out of bounds\n", val); exit(-1);}
      hedge[cnt].insert(val - 1);
      entries++;
    }
    cnt++;
  }
  f.close();
  std::map<int, GNode> hnets;
  // create nodes
  for(int i = 0; i < nodes; i++){
    GNode node;
    MetisNode n1;
    n1.netnum = INT_MAX;
    n1.netrand = INT_MAX;
    n1.netval = INT_MAX;
    n1.nodeid = i + 1;
    //n1.setWeight(hash(i+1));
    node = graph.createNode(n1); 
    graph.addNode(node);
    graph.addCell(node);
    hnets[i] = node;
  }
  // create hyperedges
  for (int i = 0; i < hedges; i++){
    GNode a;
    MetisNode n1;
    n1.netnum = i+1;
    unsigned rand = hash(i+1);
    n1.netrand = rand;
    //n1.setWeight(rand);
    a = graph.createNode(n1);
    graph.addNode(a);
    graph.addHyperedge(a);
    for (auto v : hedge[i]) {
      GNode b = hnets[v];
      graph.addEdge(a,b);
    }
  }

  graphStat(graph);
  std::cout<<"\n";
  galois::preAlloc(galois::runtime::numPagePoolAllocTotal() * 5);
  galois::reportPageAlloc("MeminfoPre");
  galois::do_all(
      galois::iterate(graph.cellList()),
      [&](GNode item) {
        //accum += g->getData(item).getWeight();
        graph.getData(item, galois::MethodFlag::UNPROTECTED).initRefine(0, true);
        graph.getData(item, galois::MethodFlag::UNPROTECTED).initPartition();
      },
      galois::loopname("initPart"));
  
  const int k = num_part;//numPartitions;
  std::cout << "number of partitions = " << k << "\n";
  //calculating number of iterations/levels required
  int num = log2(k) + 1;

  int kValue[k];
  for (int i = 0; i < k; i++)kValue[i] = 0;

  kValue[0] = k;
  //running it level by level
 
  std::set<int> toProcess;
  std::set<int> toProcessNew;
  toProcess.insert(0);
  for(int level = 0; level<num; level++) {
	
  MetisGraph* metisGraph;
  metisGraph = new MetisGraph[k];
  // for(int i = 0; i < k; i++){
  //   metisGraph[i] = new MetisGraph();
  // }
	// std::array<MetisGraph, k> metisGraph;// = new MetisGraph[2500];

	//creating separate graphs for each partition number
	for(auto n: graph.cellList()){
		
		unsigned pp = graph.getData(n).getPart();
		if(kValue[pp] > 1) {
			GGraph& gg = *metisGraph[pp].getGraph();
			gg.addNode(n);
			gg.addCell(n);
		}
	}
		
	for(auto h: graph.getNets()){
	
		bool flag = true;
		auto c = graph.edges(h).begin();
  		GNode dst = graph.getEdgeDst(*c);
  		unsigned int nPart = graph.getData(dst).getPart();
  		for (auto n : graph.edges(h)){
    			if (graph.getData(graph.getEdgeDst(n)).getPart() != nPart)
      			{
				flag = false;
				break;
			}
		}
		if(flag && kValue[nPart] > 1){
			GGraph& gg = *metisGraph[nPart].getGraph();
			gg.addNode(h);
    			gg.addHyperedge(h);
    			for (auto v : graph.edges(h)) {
      				gg.addEdge(h,graph.getEdgeDst(v));
    			}
		}
	}

	//calling Partition for each partition number
	//std::set<int> toProcessNew;
	for(auto i : toProcess) {
	  if(kValue[i] > 1){	
	    Partition(&metisGraph[i], 2, 2);
            MetisGraph *mcg = &metisGraph[i];

            while(mcg->getCoarserGraph() != NULL){
              mcg = mcg->getCoarserGraph();
            }

            while(mcg->getFinerGraph() != NULL && mcg->getFinerGraph()->getFinerGraph() != NULL){
              mcg = mcg->getFinerGraph();
              delete mcg->getCoarserGraph();
            }
              int zeroOne[2];
              for (int i = 0; i < 2; i++) zeroOne[i]=0;
	      GGraph& gg = *metisGraph[i].getGraph();
              for (auto c : gg.cellList()) {
                int part = gg.getData(c).getPart();
                zeroOne[part]++;
              }
			
			int tmp = kValue[i];
			kValue[i] = (tmp + 1)/2;
			kValue[i+(tmp+1)/2] = (tmp)/2;
			toProcessNew.insert(i);
			toProcessNew.insert(i + (tmp+1)/2);

		

			for(auto n: gg.cellList()){

                		unsigned pp = gg.getData(n).getPart();
				if(pp == 0){
					gg.getData(n).setPart(i);
				}
				else if (pp == 1){
					gg.getData(n).setPart(i+(tmp+1)/2);
				}
                
        		}
       // galois::steal(), galois::loopname("setPart"));
		}
	}
	delete [] metisGraph;
	toProcess = toProcessNew;
        toProcessNew.clear();
}
  std::ofstream ofs(output.c_str());
  std::map<int, int> cell;
  for (auto c : graph.cellList()) {
    int id = graph.getData(c).nodeid;
    int p = graph.getData(c).getPart();
    cell[id] = p;
  }
  bool first = true;
  for (int i = 1; i <= cell.size(); i++) {
    if(first)
      first = false;
    else
      ofs<<"\n";

    ofs<<cell[i];  
  }
  ofs.close();
 // Partition(&metisGraph, csize, refiter);
  //std::cout<<"Total Edge Cut: "<<computingCut(graph)<<"\n";
  galois::runtime::reportStat_Single("HyPar", "Edge Cut", computingCut(graph));
  //galois::runtime::reportStat_Single("HyParzo", "zero-one", computingBalance(graph, 2500));
  // galois::reportPageAlloc("MeminfoPost");

  return 0;
}

