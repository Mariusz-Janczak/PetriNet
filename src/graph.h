#ifndef __graph_h__
#define __graph_h__

#include "petri.h"

namespace graph {
  void PTNetCreateGraph(PetriNet& petrinet, bool macro, PPeak& Peak, bool& dead);
  void PTNetWriteGraph(PetriNet& petrinet, bool macro);

  void PTNetCreateMacronet(PetriNet& petrinet, int& LastMacro, bool& dead, bool& Danger);
  void PTNetWriteMacronet(PetriNet& petrinet, int LastMacro);
}
using namespace graph;

#endif /* __graph_h__ */
