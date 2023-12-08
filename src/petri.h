#ifndef __PETRI_H__
#define __PETRI_H__

#include "types.h"

using namespace std;

class PetriNet
{
private:
  /* data */
public:
  PetriNet(const std::string& fname);
  ~PetriNet();

  void About();
  void ReachabilityGraph(bool macro = false);
  void CreateMacronet(void);
  void Coloring(bool macro = false);
  void Decomposition(void);
  void Encoding(void);

  std::string file_name;
  bool save_intermediate_results;
  ptnet_file_error_t file_status;
  ptnet_input_data_t input_data;
  ptnet_transitions_list_t net, macronet, transfers;
  ptnet_registers_list_t reg;
  ptnet_branches_list_t branches, macronet_branches;
  ptnet_places_t net_marking, macronet_marking, macronet_places;
  TContents contents;
};

#endif /* __PETRI_H__ */
