#include <iostream>
#include <fstream>
#include <set>
#include <vector>

using namespace std;

typedef set<int> ptnet_places_t;
typedef vector<ptnet_places_t> TPalete;
typedef vector<ptnet_places_t> TContents;
typedef vector<int> ptnet_registers_list_t;

struct PBranch {
  ptnet_places_t MrkPlc;
  PBranch* RdyTrn;
};

struct ptnet_transitions_list_t {
  int Nr;
  ptnet_places_t InpPlc;
  ptnet_places_t Inp;
  ptnet_places_t InpN;
  ptnet_places_t OutPlc;
  ptnet_places_t Out;
  ptnet_places_t OutR;
  ptnet_transitions_list_t* NextTrn;
};

struct ptnet_branches_list_t {
  PBranch* Addr;
  ptnet_branches_list_t* Next;
};

void Coloring(ptnet_branches_list_t* branches, TPalete& Color, int& ColNr);
void WhichColor(TPalete MColor, int ColNr, TContents contents, int LastMacro, TPalete& Color);
void WriteColored(string FileName, ptnet_transitions_list_t* Net, TPalete Color, int ColNr, int What, ptnet_registers_list_t Reg, bool Macro);

void Coloring(ptnet_branches_list_t* branches, TPalete& Color, int& ColNr) {
  int I, J;
  PBranch* MaxBranch;
  ptnet_places_t PlcAct;
  ptnet_places_t ForbPlc;

  void MrkMax(ptnet_branches_list_t * branches, PBranch * &MaxBranch, int& PlcsNbr) {
    int I;

    int PlcNr(ptnet_places_t Coll) {
      int I, DPlcNr = 0;
      for (I = 1; I <= 255; I++) {
        if (Coll.find(I) != Coll.end()) {
          DPlcNr++;
        }
      }
      return DPlcNr;
    }

    MaxBranch = branches->Addr;
    PlcsNbr = PlcNr(branches->Addr->MrkPlc);
    branches = branches->Next;
    while (branches != nullptr) {
      if (PlcsNbr < PlcNr(branches->Addr->MrkPlc)) {
        PlcsNbr = PlcNr(branches->Addr->MrkPlc);
        MaxBranch = branches->Addr;
      }
      branches = branches->Next;
    }
  }

  void WriteColor(ptnet_branches_list_t * branches, int J, ptnet_places_t & Color, ptnet_places_t & ForbPlc) {
    Color.insert(J);
    ptnet_branches_list_t* DBranches = branches;
    while (DBranches != nullptr) {
      if (DBranches->Addr->MrkPlc.find(J) != DBranches->Addr->MrkPlc.end()) {
        ForbPlc.insert(DBranches->Addr->MrkPlc.begin(), DBranches->Addr->MrkPlc.end());
        ForbPlc.erase(J);
      }
      DBranches = DBranches->Next;
    }
  }

  void Dye(ptnet_branches_list_t * branches, PBranch * Trees, int ColMin, int PlcMin, int ColNr, TPalete & Color, PBranch * &aktBranch) {
    set<int> PlcAct, Colored;
    set<int> ColFree;
    int HowMany;

    void FreeColors(ptnet_places_t Places, int ColNr, TPalete & Color, set<int>&ColFree) {
      ColFree.clear();
      for (int I = 1; I <= ColNr; I++) {
        ColFree.insert(I);
      }
      int I = 0;
      do {
        do {
          I++;
        } while ((Places.find(I) == Places.end()) && (I != 256));
        if (I < 256) {
          int NrCol = 0;
          for (int J = 1; J <= ColNr; J++) {
            if (Color[J].find(I) != Color[J].end()) {
              NrCol++;
            }
          }
          if (NrCol == 0) {
            NrCol = 1;
          }
          ColFree.erase(NrCol);
        }
      } while (I != 256);
    }

    void GetHowMany(ptnet_branches_list_t * branches, TPalete Color, ptnet_places_t ForbPlc, int Place, int ColNr, int& HowMany) {
      int I, J, DHowMany, NrCol;
      HowMany = ColNr;
      DHowMany = ColNr;
      while ((branches != nullptr) && (DHowMany > 0)) {
        if (ForbPlc.find(Place) != ForbPlc.end()) {
          I = 0;
          DHowMany = ColNr;
          do {
            do {
              I++;
            } while ((branches->Addr->MrkPlc.find(I) == branches->Addr->MrkPlc.end()) && (I != 256));
            if (I < 256) {
              NrCol = 0;
              for (J = 1; J <= ColNr; J++) {
                if (Color[J].find(I) != Color[J].end()) {
                  NrCol++;
                }
              }
              if (NrCol == 0) {
                NrCol = 1;
              }
              DHowMany -= NrCol;
            }
          } while (I != 256);
          if (HowMany > DHowMany) {
            HowMany = DHowMany;
          }
        }
        branches = branches->Next;
      }
    }

    while (Trees != nullptr) {
      Colored.clear();
      for (int L = 1; L <= ColNr; L++) {
        Colored.insert(Color[L].begin(), Color[L].end());
      }
      if (!(Trees->MrkPlc <= Colored)) {
        FreeColors(Trees->MrkPlc, ColNr, Color, ColFree);
        PlcAct = Trees->MrkPlc - Colored;
        int J = PlcMin - 1;
        for (int I = ColMin; I <= ColNr; I++) {
          if (ColFree.find(I) != ColFree.end()) {
            do {
              do {
                J++;
              } while ((PlcAct.find(J) == PlcAct.end()) && (J != 256));
              if ((ForbPlc[I].find(J) == ForbPlc[I].end()) && (J < 256)) {
                ColFree.erase(I);
                WriteColor(branches, J, Color[I], ForbPlc[I]);
              }
              if (ForbPlc[I].find(J) != ForbPlc[I].end()) {
                int K = ColMin - 1;
                do {
                  do {
                    K++;
                  } while ((ColFree.find(K) == ColFree.end()) && (K > ColNr));
                } while ((ForbPlc[K].find(J) != ForbPlc[K].end()) || (K > ColNr));
                if (K <= ColNr) {
                  WriteColor(branches, J, Color[K], ForbPlc[K]);
                  ColFree.erase(K);
                }
              }
            } while ((ForbPlc[I].find(J) != ForbPlc[I].end()) || (J == 256));
            if ((ColFree.size() != 0) && (J == 256)) {
              PlcAct = Trees->MrkPlc - Colored;
              J = 0;
              do {
                do {
                  J++;
                } while ((PlcAct.find(J) == PlcAct.end()) && (J != 256));
                GetHowMany(branches, Color, ForbPlc, J, ColNr, HowMany);
                while ((HowMany > 0) && (ColFree.size() != 0)) {
                  int K = 0;
                  do {
                    do {
                      K++;
                    } while ((ColFree.find(K) == ColFree.end()) && (K > ColNr));
                  } while ((ForbPlc[K].find(J) != ForbPlc[K].end()) || (K > ColNr));
                  if (K <= ColNr) {
                    WriteColor(branches, J, Color[K], ForbPlc[K]);
                    ColFree.erase(K);
                    HowMany--;
                  }
                }
                PlcAct.erase(J);
              } while (ColFree.size() != 0);
            }
          }
        }
        Dye(branches, Trees->RdyTrn, 1, 1, ColNr, Color, Trees);
      }
      Trees = Trees->RdyTrn;
    }
  }

  MrkMax(branches, MaxBranch, ColNr);
  int J = 0;
  PlcAct = MaxBranch->MrkPlc;
  for (int I = 1; I <= ColNr; I++) {
    do {
      J++;
    } while (PlcAct.find(J) == PlcAct.end());
    Color[I].clear();
    ForbPlc[I].clear();
    WriteColor(branches, J, Color[I], ForbPlc[I]);
  }
  Dye(branches, MaxBranch->RdyTrn, 1, 1, ColNr, Color, MaxBranch);
}

void WhichColor(TPalete MColor, int ColNr, TContents contents, int LastMacro, TPalete& Color) {
  for (int I = 1; I <= ColNr; I++) {
    Color[I] = MColor[I];
    for (int J = 101; J <= LastMacro; J++) {
      if (Color[I].find(J) != Color[I].end()) {
        Color[I].insert(contents[J].begin(), contents[J].end());
        Color[I].erase(J);
      }
    }
  }
}

void GetColored(ofstream& Plik, ptnet_transitions_list_t* Net, TPalete Color, int ColNr, int What, ptnet_places_t& Place, bool Macro) {
  void WritePlcCl(ptnet_places_t Places, ptnet_places_t ster, ptnet_places_t stersig, TPalete Color, int ColNr, bool Inp, bool prim, bool Macro) {
    bool Start = true;
    for (int I = 0; I <= 199; I++) {
      if (Places.find(I) != Places.end()) {
        if (!Start) {
          Plik << " * ";
        }
        else {
          Start = false;
        }
        if (I > 100) {
          if (!prim) {
            Plik << "@M";
          }
          else {
            Plik << "M";
          }
        }
        if (I <= 100) {
          if (!prim) {
            Plik << "@";
          }
        }
        Plik << "P" << I << "[";
        Start = true;
        for (int J = 1; J <= ColNr; J++) {
          if (Color[J].find(I) != Color[J].end()) {
            if (!Start) {
              Plik << ",";
            }
            else {
              Start = false;
            }
            Plik << J;
          }
        }
        Plik << "]";
      }
    }
    if (!Macro) {
      for (int I = 0; I <= 99; I++) {
        if (ster.find(I) != ster.end()) {
          if ((stersig.find(I) != stersig.end()) && (Inp)) {
            Plik << " * /X" << I;
          }
          else if ((stersig.find(I) != stersig.end()) && (!Inp)) {
            Plik << " * RY" << I;
          }
          else if (Inp) {
            Plik << " * X" << I;
          }
          else {
            Plik << " * SY" << I;
          }
        }
      }
    }
  }

  if (What == 1) {
    Plik << "Coloured net." << endl;
    Plik << "---------------" << endl;
  }
  else {
    Plik << "Coloured macronet." << endl;
    Plik << "--------------------" << endl;
  }
  Plik << endl;
  for (int I = 1; I <= ColNr; I++) {
    Plik << "COLOR: " << I << ",   PLACE: ";
    int J = 1;
    bool Start = true;
    while (J < 200) {
      if (Color[I].find(J) != Color[I].end()) {
        if (J <= 100) {
          if (Start) {
            Plik << "P";
          }
          else {
            Plik << ",P";
          }
        }
        else {
          if (Start) {
            Plik << "MP";
          }
          else {
            Plik << ",MP";
          }
        }
        Start = false;
        Plik << J;
      }
      J++;
    }
    Plik << endl;
  }
  Plik << endl;
  Plik << "ESSENTIAL PLACES: ";
  Start = true;
  for (int K = 1; K <= 200; K++) {
    if (Place.find(K) != Place.end()) {
      if (K < 100) {
        if (Start) {
          Plik << "P";
        }
        else {
          Plik << ",P";
        }
      }
      else {
        if (Start) {
          Plik << "MP";
        }
        else {
          Plik << ",MP";
        }
      }
      Start = false;
      Plik << K;
    }
  }
  Plik << endl;
  Plik << endl;
  Plik << "TRANSITIONS" << endl;
  while (Net != nullptr) {
    Plik << "T" << Net->Nr << ": ";
    WritePlcCl(Net->InpPlc, Net->Inp, Net->InpN, Color, ColNr, true, true, Macro);
    Plik << " |- ";
    WritePlcCl(Net->OutPlc, Net->Out, Net->OutR, Color, ColNr, false, false, Macro);
    Plik << ";" << endl;
    Net = Net->NextTrn;
  }
  Plik << endl;
}

void WriteColored(string FileName, ptnet_transitions_list_t* Net, TPalete Color, int ColNr, int What, ptnet_registers_list_t Reg, bool Macro) {
  ofstream Plik;
  Plik.open(FileName);

  GetColored(Plik, Net, Color, ColNr, What, Place, Macro, false);

  if (!Macro) {
    Plik << "OUTPUTS" << endl;
    for (int I = 1; I <= Reg[0].Plc - 1; I++) {
      Plik << "P" << Reg[I].Plc << "[";
      bool Start = false;
      for (int J = 1; J <= ColNr; J++) {
        if (Reg[I].Plc.find(J) != Reg[I].Plc.end()) {
          if (Start) {
            Plik << ",";
          }
          Plik << J;
          Start = true;
        }
      }
      Plik << "] |- Y" << Reg[I].Out << ";" << endl;
    }
    Plik << "P" << Reg[I + 1].Plc << "[";
    bool Start = false;
    for (int J = 1; J <= ColNr; J++) {
      if (Reg[I + 1].Plc.find(J) != Reg[I + 1].Plc.end()) {
        if (Start) {
          Plik << ",";
        }
        Plik << J;
        Start = true;
      }
    }
    Plik << "] |- Y" << Reg[I + 1].Out << ";" << endl;
  }

  Plik.close();
}


