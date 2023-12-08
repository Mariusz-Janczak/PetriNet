#ifndef __types_h__
#define __types_h__

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>

#include "defs.h"

using namespace std;

namespace types {

  typedef enum ptnet_file_error_e {
    DATA_FILE_OK = 0,
    DATA_FILE_OPEN_ERROR,
    DATA_FILE_PARSE_ERROR,
    DATA_INCONSISTENT,
    DATA_FILE_UNKNOWN
  } ptnet_file_error_t;

  typedef enum ptnet_flip_flop_type_e {
    flip_flop_d = 0,
    flip_flop_jk
  } ptnet_flip_flop_type_t;

  typedef enum ptnet_encoding_results_e {
    list_of_macroplace_concurrency = 1,
    list_of_macroplace_nonconcurrency = 2,
    verified_list_of_conconcurrency = 4,
    codes_of_macroplaces = 8,
    codes_of_places = 16,
    full_table_of_codes = 32,
    encoded_sequents = 64
  } ptnet_encoding_results_t;

  typedef enum ptnet_encoding_init_e {
    initial_code_set = 0,
    initial_code_reset
  } ptnet_encoding_init_t;

  typedef std::set<int> ptnet_places_t;

  typedef struct ptnet_input_data_s {
    ptnet_places_t Plc;
    ptnet_places_t Inp;
    ptnet_places_t Reg;
    ptnet_places_t Com;
  } ptnet_input_data_t;

  typedef struct ptnet_transition_s {
    int Nr;
    ptnet_places_t InpPlc, Inp, InpN;
    ptnet_places_t OutPlc, Out, OutR;
  } ptnet_transition_t;
  typedef std::vector<ptnet_transition_t> ptnet_transitions_list_t;

  typedef struct ptnet_register_s {
    ptnet_places_t Plc;
    ptnet_places_t Out;
  } ptnet_register_t;
  typedef std::vector<ptnet_register_t> ptnet_registers_list_t;

  //typedef ptnet_tree_t* PTree;

  typedef struct ptnet_branch_s {
    ptnet_places_t MrkPlc;
    //PTree RdyTrn;
  } ptnet_branch_t;
  typedef std::vector<ptnet_branch_t> ptnet_branches_list_t;

  typedef struct ptnet_tree_s {
    ptnet_transition_t& TranNr;
    ptnet_branch_t& Branch;
  } ptnet_tree_t;
  typedef std::vector<ptnet_tree_t> ptnet_tree_list_t;

  struct REssencialPlc {
    std::byte Nr;
    bool Rep;
  };
  typedef REssencialPlc TEssencialPlc[100];

  typedef ptnet_places_t TContents[100];

  typedef ptnet_places_t TPalete[50];

  typedef std::string TCiag;

  typedef std::string TName;

  typedef std::string TExtension;

  struct RComp {
    std::byte Plc;
    bool EssPlc;
  };
  typedef RComp TComp[21];

  struct TPlc {
    std::byte Plc;
    ptnet_places_t D;
  };

  typedef struct TCompAut* PCompAut;
  struct TCompAut {
    TComp Plc;
    TPlc D;
    std::byte CondNr, ColNr;
    int MinFF, MaxFF;
    bool Roz;
    ptnet_places_t inp_, Reg, Comb;
    ptnet_places_t TrnNr;
    ptnet_transitions_list_t Trn;
    PCompAut NextAut;
  };

  typedef struct TContMc* PContMc;
  struct TContMc {
    std::byte McNr;
    std::byte Price;
    ptnet_places_t ContMc;
    PContMc Next;
  };

  typedef struct TPeak* PPeak;
  struct TPeak {
    ptnet_places_t PeakCont;
    PPeak Next;
  };

  typedef struct TConcurrency* PConcurrency;
  struct TConcurrency {
    std::byte McNr;
    ptnet_places_t ConcMc;
    PConcurrency Next;
  };

  typedef struct TNonconcurrency* PNonconcurrency;
  struct TNonconcurrency {
    std::byte McNr;
    ptnet_places_t NoncMc;
    PNonconcurrency Next;
  };

  typedef struct TVerified* PVerified;
  struct TVerified {
    std::byte IncLev;
    std::byte UniteNr;
    ptnet_places_t UniteMc;
    ptnet_places_t NoncMc;
    PVerified Next;
  };

  typedef struct TFlipFlop* PFlipFlop;
  struct TFlipFlop {
    std::byte FFNr;
    char Ch;
    PFlipFlop Next;
  };

  typedef struct TCodes* PCodes;
  struct TCodes {
    std::byte PlcNr;
    std::byte McNr;
    PFlipFlop PlcCode;
    PCodes Next;
  };

  typedef struct TListCodes* PListCodes;
  struct TListCodes {
    std::byte PlcNr;
    std::string StrCod;
    PListCodes Next;
  };
}
using namespace types;

#endif /* __types_h__ */
