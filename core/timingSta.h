#ifndef __REPLACE_TIMING_STA__
#define __REPLACE_TIMING_STA__ 0

#include <Machine.hh>
#include <Liberty.hh>
#include <StringUtil.hh>
#include <Vector.hh>
#include <Sta.hh>
#include <Sdc.hh>
#include <StaMain.hh>
#include <Stats.hh>
#include <Report.hh>
#include <StringUtil.hh>
#include <PatternMatch.hh>
#include <PortDirection.hh>
#include <FuncExpr.hh>
#include <Units.hh>
#include <MinMax.hh>
#include <Transition.hh>
#include <TimingRole.hh>
#include <TimingArc.hh>
#include <InternalPower.hh>
#include <LeakagePower.hh>
#include <Liberty.hh>
#include <EquivCells.hh>
#include <MinMax.hh>
#include <Network.hh>
#include <Clock.hh>
#include <PortDelay.hh>
#include <ExceptionPath.hh>
#include <Graph.hh>
#include <GraphDelayCalc.hh>
#include <Parasitics.hh>
#include <Wireload.hh>
#include <DelayCalc.hh>
#include <DcalcAnalysisPt.hh>
#include <Corner.hh>
#include <PathVertex.hh>
#include <PathRef.hh>
#include <PathEnd.hh>
#include <PathGroup.hh>

#ifdef YY_INPUT
#undef YY_INPUT
#endif

using namespace sta;

// to import Swig
extern "C" {
extern int Sta_Init(Tcl_Interp *interp);
}

// to import TCL functions
namespace sta {
    extern const char *tcl_inits[];
}

#endif
