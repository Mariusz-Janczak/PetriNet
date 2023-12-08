#ifndef __automat_h__
#define __automat_h__

#include "types.h"
#include "tools.h"

namespace automat {
  void Automats(FNameStr& FileName, ptnet_input_data_t input_data, ptnet_transitions_list_t& Net, ptnet_registers_list_t Reg, ptnet_places_t Marking, TPalete Color, integer ColorsNr, integer What);

  void EssencialPlc(ptnet_places_t& Place, TPalete Color, integer ColorsNr);
}

using namespace automat;

#endif /* __automat_h__ */
