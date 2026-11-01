#ifndef __types_h__
#define __types_h__

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <bitset>
#include <initializer_list>

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

  /*
   * Equivalent of Pascal "set of 1..255" (TPlace). Operators follow Pascal
   * semantics: + union, - difference, * intersection, <= subset.
   * Iteration yields the elements in ascending order.
   */
  class ptnet_places_t {
  public:
    static constexpr int max = 255;

    class const_iterator {
    public:
      const_iterator(const std::bitset<max + 1>& bits, int pos) : bits(bits), pos(pos) { skip(); }
      int operator*() const { return pos; }
      const_iterator& operator++() { pos++; skip(); return *this; }
      bool operator==(const const_iterator& other) const { return pos == other.pos; }
      bool operator!=(const const_iterator& other) const { return pos != other.pos; }
    private:
      void skip() { while ((pos <= max) && !bits.test(pos)) pos++; }
      const std::bitset<max + 1>& bits;
      int pos;
    };

    ptnet_places_t() = default;
    ptnet_places_t(std::initializer_list<int> items) { for (int i : items) bits.set(i); }

    bool has(int i) const { return bits.test(i); }
    bool empty() const { return bits.none(); }
    size_t size() const { return bits.count(); }
    void insert(int i) { bits.set(i); }
    void erase(int i) { bits.reset(i); }
    void clear() { bits.reset(); }

    const_iterator begin() const { return const_iterator(bits, 0); }
    const_iterator end() const { return const_iterator(bits, max + 1); }

    bool operator==(const ptnet_places_t& other) const { return bits == other.bits; }
    bool operator!=(const ptnet_places_t& other) const { return bits != other.bits; }
    bool operator<=(const ptnet_places_t& other) const { return (bits & ~other.bits).none(); }
    bool operator>=(const ptnet_places_t& other) const { return other <= *this; }

    ptnet_places_t operator+(const ptnet_places_t& other) const { return ptnet_places_t(bits | other.bits); }
    ptnet_places_t operator-(const ptnet_places_t& other) const { return ptnet_places_t(bits & ~other.bits); }
    ptnet_places_t operator*(const ptnet_places_t& other) const { return ptnet_places_t(bits & other.bits); }
    ptnet_places_t& operator+=(const ptnet_places_t& other) { bits |= other.bits; return *this; }
    ptnet_places_t& operator-=(const ptnet_places_t& other) { bits &= ~other.bits; return *this; }
    ptnet_places_t& operator*=(const ptnet_places_t& other) { bits &= other.bits; return *this; }

  private:
    explicit ptnet_places_t(const std::bitset<max + 1>& bits) : bits(bits) {}
    std::bitset<max + 1> bits;
  };

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

  /*
   * Reachability graph (TBranch/TTree/TAddress in typy.pas). Nodes are kept
   * in a vector in creation order and arcs refer to them by index, so the
   * vector may grow while the graph is being built.
   */
  typedef struct ptnet_arc_s {
    size_t transition; /* index into the transitions list the graph was built from */
    size_t branch;     /* index of the target node in the branches list */
  } ptnet_arc_t;

  typedef struct ptnet_branch_s {
    ptnet_places_t MrkPlc;
    std::vector<ptnet_arc_t> RdyTrn;
  } ptnet_branch_t;
  typedef std::vector<ptnet_branch_t> ptnet_branches_list_t;

  /* Arrays below are sized so that the original Pascal indices can be used. */
  struct REssencialPlc {
    int Nr;
    bool Rep;
  };
  typedef REssencialPlc TEssencialPlc[101]; /* [1..100] */

  typedef ptnet_places_t TContents[200]; /* [101..199] */

  typedef ptnet_places_t TPalete[51]; /* [1..50] */

  typedef std::string TCiag;

  typedef std::string TName;

  typedef std::string TExtension;

  struct RComp {
    int Plc;
    bool EssPlc;
  };
  typedef RComp TComp[21]; /* [0..20] */

  struct TPlc {
    int Plc;
    ptnet_places_t D;
  };

  typedef struct TCompAut* PCompAut;
  struct TCompAut {
    TComp Plc;
    TPlc D;
    int CondNr, ColNr;
    int MinFF, MaxFF;
    bool Roz;
    ptnet_places_t inp_, Reg, Comb;
    ptnet_places_t TrnNr;
    ptnet_transitions_list_t Trn;
    PCompAut NextAut;
  };

  typedef struct TContMc* PContMc;
  struct TContMc {
    int McNr;
    int Price;
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
    int McNr;
    ptnet_places_t ConcMc;
    PConcurrency Next;
  };

  typedef struct TNonconcurrency* PNonconcurrency;
  struct TNonconcurrency {
    int McNr;
    ptnet_places_t NoncMc;
    PNonconcurrency Next;
  };

  typedef struct TVerified* PVerified;
  struct TVerified {
    int IncLev;
    int UniteNr;
    ptnet_places_t UniteMc;
    ptnet_places_t NoncMc;
    PVerified Next;
  };

  typedef struct TFlipFlop* PFlipFlop;
  struct TFlipFlop {
    int FFNr;
    char Ch;
    PFlipFlop Next;
  };

  typedef struct TCodes* PCodes;
  struct TCodes {
    int PlcNr;
    int McNr;
    PFlipFlop PlcCode;
    PCodes Next;
  };

  typedef struct TListCodes* PListCodes;
  struct TListCodes {
    int PlcNr;
    std::string StrCod;
    PListCodes Next;
  };
}
using namespace types;

#endif /* __types_h__ */
