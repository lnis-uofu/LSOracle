
#include "BiPart.h"

namespace cll = llvm::cl;

const char* name = "HYPAR";
const char* desc =
    "Partitions a hypergraph into K parts and minimizing the graph cut";
const char* url = "HyPar";
cll::opt<scheduleMode> schedulingMode(
    cll::desc("Choose a inital scheduling mode:"),
    cll::values(clEnumVal(PLD, "PLD"), clEnumVal(PP, "PP"), clEnumVal(WD, "WD"),
                clEnumVal(RI, "RI"), clEnumVal(MRI, "MRI"),clEnumVal(MDEG, "MDEG"),clEnumVal(DEG, "DEG"),clEnumVal(MWD, "MWD"),clEnumVal(HIS, "HIS"),clEnumValEnd),
    cll::init(PLD));

cll::opt<bool>
    mtxInput("mtxinput",
             cll::desc("Use text mtx files instead of binary galois gr files"),
             cll::init(false));
cll::opt<bool> weighted("weighted", cll::desc("weighted"),
                               cll::init(false));
cll::opt<bool>
    verbose("verbose",
            cll::desc("verbose output (debugging mode, takes extra time)"),
            cll::init(false));
cll::opt<std::string> outfile("output",
                                     cll::desc("output partition file name"));
cll::opt<std::string>
    orderedfile("ordered", cll::desc("output ordered graph file name"));
cll::opt<std::string>
    permutationfile("permutation", cll::desc("output permutation file name"));
cll::opt<std::string> filename(cll::Positional,
                                      cll::desc("<input file>"), cll::Required);
cll::opt<std::string> output(cll::Positional,
                                     cll::desc("<output file>"), cll::Required);

static cll::opt<int> num_part("p", cll::desc("Number of partitions (default value 1)"), cll::init(1));

cll::opt<unsigned> csize(cll::Positional,
                                   cll::desc("<size of coarsest graph>")
                                   );

cll::opt<unsigned> refiter(cll::Positional,
                                   cll::desc("<number of iterations in ref>")
                                   );
cll::opt<double> imbalance(
    "balance",
    cll::desc("Fraction deviated from mean partition size (default 0.01)"),
    cll::init(0.01));

void Partition(MetisGraph* metisGraph, unsigned coarsenTo, unsigned refineTo) {
  MetisGraph* mcg = coarsen(metisGraph, coarsenTo, schedulingMode);
  partition(mcg);
  refine(mcg, refineTo);
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

int hash(unsigned val) {
  unsigned long int seed = val * 1103515245 + 12345;
  return((unsigned)(seed/65536) % 32768);
}

std::map<int,int> biparting(std::vector<std::vector<uint32_t>> hedge, uint32_t node_num, int num_part, int num_threads, scheduleMode schedulingMode) {
  galois::SharedMemSys G;
  LonestarStart(num_threads, name, desc, url);
  uint32_t hedges = hedge.size();
 // srand(-1);
  MetisGraph metisGraph;
  GGraph& graph = *metisGraph.getGraph();
  std::map<int, GNode> hnets;
  // create nodes
  for(int i = 0; i < node_num; i++){
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
    //for (auto v : hedge[i]) {
    for(int j = 0; j < hedge.at(i).size(); j++){
      auto v = hedge.at(i).at(j);
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
  // std::cout << "number of partitions = " << k << "\n";
  //calculating number of iterations/levels required
  int num = log2(k) + 1;

  int kValue[k];
  for (int i = 0; i < k; i++)kValue[i] = 0;

  kValue[0] = k;
  //running it level by level
 
  std::set<int> toProcess;
  std::set<int> toProcessNew;
  toProcess.insert(0);
  for (int level = 0; level<num; level++) {
    MetisGraph* metisGraph;
    metisGraph = new MetisGraph[k];

	//creating separate graphs for each partition number
    for (auto n: graph.cellList()){
		
      unsigned pp = graph.getData(n).getPart();
      if(kValue[pp] > 1) {
	GGraph& gg = *metisGraph[pp].getGraph();
	gg.addNode(n);
	gg.addCell(n);
      }
    }
		
    for (auto h: graph.getNets()) {	
      bool flag = true;
      auto c = graph.edges(h).begin();
      GNode dst = graph.getEdgeDst(*c);
      unsigned int nPart = graph.getData(dst).getPart();
      for (auto n : graph.edges(h)){
        if (graph.getData(graph.getEdgeDst(n)).getPart() != nPart) {
		flag = false;
		break;
	}
      }

      if (flag && kValue[nPart] > 1) {
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
      if(kValue[i] > 1) {	
        Partition(&metisGraph[i], 25, 2);
        MetisGraph *mcg = &metisGraph[i];

        while(mcg->getCoarserGraph() != NULL) {
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
	     if(pp == 0) {
	       gg.getData(n).setPart(i);
	     }
	     else if (pp == 1){
	       gg.getData(n).setPart(i+(tmp+1)/2);
	     }
                
         }
      }
    }
	
    toProcess = toProcessNew;
    toProcessNew.clear();
}
  // std::ofstream ofs(output.c_str());
  std::map<int, int> cell;
  for (auto c : graph.cellList()) {
    int id = graph.getData(c).nodeid;
    int p = graph.getData(c).getPart();
    cell[id] = p;
  }
  return cell;
  // for (int i = 1; i <= cell.size(); i++) {
  //   ofs<<cell[i]<<"\n";  
  // }
  // ofs.close();
}

