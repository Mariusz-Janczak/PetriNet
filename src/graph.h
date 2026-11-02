#ifndef __graph_h__
#define __graph_h__

#include "petri.h"

namespace graph {
  void PTNetCreateGraph(PetriNet& petrinet, bool macro, bool& dead);
  void PTNetWriteGraph(PetriNet& petrinet, bool macro);

  void PTNetCreateMacronet(PetriNet& petrinet, bool& dead, bool& unsafe);
  void PTNetWriteMacronet(PetriNet& petrinet);
}
using namespace graph;

#endif /* __graph_h__ */
