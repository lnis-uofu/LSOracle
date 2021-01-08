#ifndef __REPLACE_TIMING_STA__
#define __REPLACE_TIMING_STA__ 0

#include <sta/Machine.hh>
#include <sta/Liberty.hh>
#include <sta/StringUtil.hh>
#include <sta/Vector.hh>
#include <sta/Sta.hh>
#include <sta/Sdc.hh>
#include <sta/StaMain.hh>
#include <sta/Stats.hh>
#include <sta/Report.hh>
#include <sta/StringUtil.hh>
#include <sta/PatternMatch.hh>
#include <sta/PortDirection.hh>
#include <sta/FuncExpr.hh>
#include <sta/Units.hh>
#include <sta/MinMax.hh>
#include <sta/Transition.hh>
#include <sta/TimingRole.hh>
#include <sta/TimingArc.hh>
#include <sta/InternalPower.hh>
#include <sta/LeakagePower.hh>
#include <sta/Liberty.hh>
#include <sta/EquivCells.hh>
#include <sta/MinMax.hh>
#include <sta/Network.hh>
#include <sta/Clock.hh>
#include <sta/PortDelay.hh>
#include <sta/ExceptionPath.hh>
#include <sta/Graph.hh>
#include <sta/GraphDelayCalc.hh>
#include <sta/Parasitics.hh>
#include <sta/Wireload.hh>
#include <sta/DelayCalc.hh>
#include <sta/DcalcAnalysisPt.hh>
#include <sta/Corner.hh>
#include <sta/PathVertex.hh>
#include <sta/PathRef.hh>
#include <sta/PathEnd.hh>
#include <sta/PathGroup.hh>

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
