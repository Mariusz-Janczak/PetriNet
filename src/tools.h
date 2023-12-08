#ifndef __tools_h__
#define __tools_h__

#include "petri.h"

namespace tools {
  std::string PTNetFileNameNew(const std::string& fname, const std::string& fext);
  ptnet_file_error_t PTNetReadData(PetriNet& petrinet);
}

using namespace tools;

#endif /* __tools_h__ */