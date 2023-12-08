#ifndef __colored_h__
#define __colored_h__

#include "types.h"
#include "tools.h"
#include "automat.h"

namespace colored {
  void Coloring(ptnet_branches_list_t branches, TPalete& Color, integer& ColNr);

  void WhichColor(TPalete MColor, integer ColNr, TContents contents, integer LastMacro, TPalete& Color);

  void WriteColored(FNameStr& FileName, ptnet_transitions_list_t Net, TPalete Color, integer ColNr, integer What, ptnet_registers_list_t Reg, boolean Macro);
}

using namespace colored;

#endif /* __colored_h__ */
