#pragma once

#include <array>
#include <fstream>
#include <iostream>
#include <string>

#include <ez/direct_iterator.hpp>
#include <fmt/format.h>
#include <aig/gia/gia.h>
#include <aig/aig/aig.h>
#include <aig/gia/giaAig.h>
#include <base/abc/abc.h>
#include <base/abci/abc.c>
#include <base/main/abcapis.h>

/* 
  Creates a simple wrapper with ABC for AIG. The mockturtle AIG network is converted into an ABC gia.
  Author: Walter Lau Neto
*/ 

namespace mockturtle
{
  // only works with AIGs. If one wants to conver another type of network can first use the resynthesis method on mockturtle to convert it onto an AIG

  class abc_view
  {
    public:

      abc_view ()
      {
      }

      // constructor - derives abc network from mockturtle AIG
      abc_view( mockturtle::aig_network const& ntk )
      {
        this->to_abc(ntk); 

        std::string fileName = "test.aig"; 
        abc::Gia_AigerWrite( pNew, fileName.c_str(), 0, 0 );
        
        // if(pNew != NULL)
        //   std::cout << "Conversion done, pointer is not null!\n";
      }

      void free_manager()
      {
        abc::Gia_ManHashStop( pNew );
      }

      void stats() const 
      {
        std::cout << "ABC network stats\n";
        abc::Gia_ManPrintStats(pNew, 0);
      }

      void foreach_node() const
      {
        abc::Gia_Obj_t* pObj; 
        for ( auto i = 0; (i < pNew->nObjs) && ((pObj) = abc::Gia_ManObj(pNew, i)); i++ ){
          if(abc::Gia_ObjIsAnd(pObj))
          {
            // to do: iterate over GIA's AND nodes. Just an example 
          }
        }
      }

      void to_abc( mockturtle::aig_network const& ntk )
      {
        // gets global frame. It is needed for some optimization procedures
        pAbc = abc::Abc_FrameGetGlobalFrame();

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

        // create PIs
        ntk.foreach_pi( [&]( auto const& pi, auto index ) {
          iObj = abc::Gia_ManAppendCi(pNew);    
          abc::Vec_IntPush( vNodes, iObj );
        });

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

        // stops manager's structural hashing    
        // abc::Gia_ManHashStop( pNew );
        
        // create COs  
        vDrivers = abc::Vec_IntAlloc( nLatches + nOutputs );
        
        for(int index = 0; index < ntk.num_pos() - ntk.num_latches(); index++){
          uLit0 = ntk._storage->outputs[index].data; 
          iNode0 = abc::Abc_LitNotCond( abc::Vec_IntEntry(vNodes, uLit0 >> 1), (uLit0 & 1) );
          abc::Vec_IntPush( vDrivers, iNode0 );
        }

        ntk.foreach_ri([&](auto fi) {
          uLit0 = fi.data;
          iNode0 = abc::Abc_LitNotCond( abc::Vec_IntEntry(vNodes, uLit0 >> 1), (uLit0 & 1) );
          abc::Vec_IntPush( vDrivers, iNode0 );
        });

        for(i=0; i < ntk.num_pos(); i++){
          abc::Gia_ManAppendCo( pNew, abc::Vec_IntEntry(vDrivers, i) );
        }
        
        if(nLatches > 0)
          Gia_ManSetRegNum( pNew, nLatches );
        
      }

      mockturtle::aig_network from_abc()
      {
        //pNew = pAbc->pGia; 
        
        // reads ABC GIA 
        if(pNew == NULL)
        {
          std::cout << "There is no AIG stored in ABC. You must first call the ABC view to deliver an AIG from your current network! \n"; 
          return; 
        }

        pTopo = abc::Gia_ManDupNormalize( pNew, 0 );
        //abc::Gia_ManTransferMapping( pTopo, pNew );
        //abc::Gia_ManTransferPacking( pTopo, pNew );
        //abc::Gia_ManTransferTiming( pTopo, pNew );
        //pTopo->nConstrs   = pNew->nConstrs;
        
        using signal = typename mockturtle::aig_network::signal; 

        // creates a new mockturtle AIG
        mockturtle::aig_network pMock; 
        abc::Gia_Obj_t* pObj;
        int i = 0;
        unsigned uLit0, uLit1, uLit;

        int counter = 0;

        signal left, right, po; 

        // creates constant
        pMock.get_constant( false ); 

        for ( i = 0; (i < abc::Gia_ManPiNum(pTopo)) && ((pObj) = abc::Gia_ManCi(pTopo, i)); i++ )
        {
          // creates PIs with default name. TO DO: registers 
          pMock.create_pi();
        }

        for ( i = 0; (i < pTopo->nObjs) && ((pObj) = abc::Gia_ManObj(pTopo, i)); i++ )  if ( !abc::Gia_ObjIsAnd(pObj) ) {} else
        {
          // create AND nodes 
          uLit  = abc::Abc_Var2Lit( i, 0 );
          uLit0 = abc::Gia_ObjFaninLit0( pObj, i );
          uLit1 = abc::Gia_ObjFaninLit1( pObj, i );
          left  = pMock.child_to_signal( uLit0 );
          right = pMock.child_to_signal( uLit1 );

          if( uLit0 & 1 )
          {
            left = pMock.create_not( left ); 
          }
          
          if( uLit1 & 1 )
          {
            right = pMock.create_not( right ); 
          }
          pMock.create_and( left, right );
          counter++;
        }

        for ( i = 0; (i < abc::Gia_ManPoNum(pNew)) && ((pObj) = abc::Gia_ManCo(pNew, i)); i++ )
        {
          if(!abc::Gia_ObjIsCo(abc::Gia_ObjChild0(pObj)))
          {
            //uLit  = abc::Abc_Var2Lit( i, 0 );
            uLit = abc::Gia_ObjFaninLit0( pObj, Gia_ObjId(pNew,pObj) );
          }

          else if (!abc::Gia_ObjIsCo(abc::Gia_ObjChild1(pObj)))
          {
            uLit = abc::Gia_ObjFaninLit1( pObj, Gia_ObjId(pNew,pObj) );
          }
          
          po = pMock.child_to_signal( uLit );

          if( uLit & 1 )
          {
            po = pMock.create_not( po );
          }

          pMock.create_po( po );

        }
        return pMock; 
      }

      void dc2()
      {
        if(pNew == NULL){
          return; 
        }

        abc::Gia_Man_t *pTmp; 

        pTmp = pNew; 
        pNew = abc::Gia_ManCompress2( pTmp, 1, 0 );
        abc::Gia_ManPrintStats(pNew, 0);
        abc::Abc_FrameUpdateGia( pAbc, pNew );
      }

      bool abc_has_aig()
      {
        if(pNew == NULL)
          return false;
        return true; 
      }

      private: 
        // variables to use with ABC
        abc::Gia_Man_t *pNew;
        abc::Gia_Man_t *pTopo; 
        abc::Abc_Frame_t *pAbc; 
        abc::Vec_Int_t * vLits = NULL;

        int nTotal, nInputs, nOutputs, nLatches, nAnds, i;
        int iObj, iNode0, iNode1;

        unsigned uLit0, uLit1;

        abc::Vec_Int_t * vNodes, * vDrivers;
  };
}