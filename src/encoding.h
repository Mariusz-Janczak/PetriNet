#ifndef __encoding_h__
#define __encoding_h__

#include "types.h"
#include "tools.h"

namespace encoding {
  void ListOfConcurrency(FNameStr& FileName, boolean WriteFile, ptnet_places_t macronet_places, PPeak& Peak, PConcurrency& Concurrency);

  void ListOfNonConcurrency(FNameStr& FileName, boolean WriteFile, PConcurrency& Concurrency, ptnet_places_t macronet_places, PNonconcurrency& NonConcurrency);

  void VerifiedList(FNameStr& FileName, boolean WriteFile, PNonconcurrency NonConcurrency, PVerified& Verified);

  void CodesOfMcPlaces(FNameStr& FileName, boolean WriteFile, ptnet_places_t macronet_places, PConcurrency& Concurrency, PVerified Verified, PCodes& McCodes);

  void CodesOfPlaces(FNameStr& FileName, boolean WriteFile, boolean Rst, PConcurrency Concurrency, PPeak Peak, ptnet_places_t Marking, ptnet_places_t macronet_places, byte LastMacro, TContents contents, PCodes McCodes, PCodes& PlcCodes, PListCodes& ListCodes);

  void FullTable(FNameStr& FileName, PCodes PlcCodes);

  void WriteSeq(FNameStr& FileName, ptnet_transitions_list_t Net, ptnet_registers_list_t Reg, ptnet_input_data_t input_data, ptnet_places_t Marking, PListCodes ListCodes);
}

using namespace encoding;

#endif /* __encoding_h__ */
