#pragma once

#include <array>
#include <fstream>
#include <iostream>
#include <string>

#include <ez/direct_iterator.hpp>
#include <fmt/format.h>

#include <aig/gia/gia.h>
#include <aig/gia/giaAig.h>
#include <map/if/if.h>

namespace oracle
{
  // only works with AIGs. If one wants to conver another type of network can first use the resynthesis method on mockturtle to convert it onto an AIG
  template<class Ntk>
  void ntk_to_abc( Ntk const& ntk )
  {
    // variables to use with ABC
    abc::Gia_Man_t *pNew;
    abc::Vec_Int_t * vLits = NULL;

    int nTotal, nInputs, nOutputs, nLatches, nAnds, i;
    int iObj, iNode0, iNode1;

    unsigned uLit0, uLit1;

    abc::Vec_Int_t * vNodes, * vDrivers;
    
    nOutputs = ntk.num_pos(); 
    nLatches = ntk.num_registers();
    nInputs = ntk.num_pis(); 
    nTotal = nLatches + nInputs + ntk.num_gates(); 

    // initialize the GIA manager and allocate the empty AIG
    pNew = abc::Gia_ManStart( nTotal + nLatches + nOutputs + 1 );

    // assuming there is no constraints nor is the simple AIG reader - it is a simple interface 
    pNew->nConstrs = 0;
    pNew->fGiaSimple = 0;

    // prepare the array of nodes
    vNodes = abc::Vec_IntAlloc( 1 + nTotal + nOutputs );
    abc::Vec_IntPush( vNodes, 0 );

    std::cout << "create PIs\n";
    // create PIs
    ntk.foreach_pi( [&]( auto const& pi, auto index ) {
      iObj = abc::Gia_ManAppendCi(pNew);    
      abc::Vec_IntPush( vNodes, iObj );
    });

    std::cout << "begin hash\n";
    // initialize structural hashing table 
    abc::Gia_ManHashAlloc( pNew );

    i = 0;
    // create AND nodes 
    ntk.foreach_node( [&]( auto const& n ) {
      // skip PIs/Pseudo PIs
      if( !ntk.is_ci( n ) )
      {
        iNode0 = 0; iNode1 = 0;
        
        auto nodeIdx = ntk.node_to_index( n );
        
        uLit1 = ntk._storage->nodes[nodeIdx].children[1].data;
        uLit0 = ntk._storage->nodes[nodeIdx].children[0].data;

        iNode0 = abc::Abc_LitNotCond( abc::Vec_IntEntry(vNodes, uLit0 >> 1), uLit0 & 1 );
        iNode1 = abc::Abc_LitNotCond( abc::Vec_IntEntry(vNodes, uLit1 >> 1), uLit1 & 1 );
        
        assert( abc::Vec_IntSize(vNodes) == i + 1 + nInputs + nLatches );
        
        abc::Vec_IntPush( vNodes, abc::Gia_ManHashAnd(pNew, iNode0, iNode1) );
        
        i++;
      }
    });

    std::cout << "stop hash\n";
    // stops manager's structural hashing    
    abc::Gia_ManHashStop( pNew );
    std::cout << "creating POs\n";
    // create POs - skipping latches for now 
    vDrivers = abc::Vec_IntAlloc( nLatches + nOutputs );
    ntk.foreach_po( [&]( auto const& po, auto index ) {
      // just pos for now 
      if(index < ntk.num_pos() - ntk.num_latches())
      {
        uLit0 = abc::Vec_IntEntry( vLits, index + nLatches );
        uLit0 = ntk._storage->outputs[index].data; 
        iNode0 = abc::Abc_LitNotCond( abc::Vec_IntEntry(vNodes, uLit0 >> 1), (uLit0 & 1) );
        abc::Vec_IntPush( vDrivers, iNode0 );
      }
    });

    ntk.foreach_po( [&]( auto const& po, auto index ) {
      abc::Gia_ManAppendCo( pNew, abc::Vec_IntEntry(vDrivers, nLatches + index) );
    });

    // std::string fileName = "test.aig"; 

    // abc::Gia_AigerWrite( pNew, fileName.c_str(), 0, 0 );
    std::cout << "ABC network stats\n";
    abc::Gia_ManPrintStats(pNew, 0);

    std::cout << "ABC LUT mapping\n";
    abc::If_Par_t Pars, * pPars = &Pars;
    abc::If_ManSetDefaultPars( pPars );
    pPars->nLutSize = 4;
    std::cout << "running\n";
    pNew = Gia_ManPerformMapping(pNew, pPars);
    std::cout << "done\n";
    abc::Gia_ManPrintStats(pNew, 0);
  }
}