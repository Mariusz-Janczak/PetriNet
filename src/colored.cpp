/******************************************************************************/
/*                     Department of Computer Engineering                     */
/*                Higher College of Engineering in Zielona Gora               */
/*                                                                            */
/*                Decomposition and Place Encoding of Petri Net.              */
/*                 Master's thesis, Supervisor Prof. M.Adamski.               */
/*                                                                            */
/*                                            Author: Mariusz Janczak (1993)  */
/******************************************************************************/
/*  Module for coloring a Petri network and macronetsworks.                   */
/******************************************************************************/

#include "colored.h"

namespace colored {


  /*������������������������ Kolorowanie (makro)sieci ��������������������������*/

  void Coloring(ptnet_branches_list_t branches,
    TPalete& Color,
    integer& ColNr);

  static TPalete ForbPlc;
  static void MrkMax(ptnet_branches_list_t branches,
    PBranch& MaxBranch,
    integer& PlcsNbr);


  /*------------------------------------------------------------------------*/

  static integer PlcNr(ptnet_places_t Coll)
  {
    integer I, DPlcNr;

    integer PlcNr_result;
    DPlcNr = 0;
    for (I = 1; I <= 255; I++)
      if (Coll.has(I))  DPlcNr += 1;
    PlcNr_result = DPlcNr;
    return PlcNr_result;
  }



  /*==========================================================================*/

  static void MrkMax(ptnet_branches_list_t branches,
    PBranch& MaxBranch,
    integer& PlcsNbr)
  {
    byte I;

    /*------------------------------------------------------------------------*/


    MaxBranch = branches->Addr;
    PlcsNbr = PlcNr(branches->Addr->MrkPlc);
    branches = branches->Next;
    while (branches != nil)
    {
      if (PlcsNbr < PlcNr(branches->Addr->MrkPlc))
      {
        PlcsNbr = PlcNr(branches->Addr->MrkPlc);
        MaxBranch = branches->Addr;
      }
      branches = branches->Next;
    }
  }



  /*==========================================================================*/

  static void WriteColor(ptnet_branches_list_t branches,
    integer J,
    ptnet_places_t& Color, ptnet_places_t& ForbPlc)
  {
    ptnet_branches_list_t DBranches;

    Color = Color + set::of(J, eos);
    DBranches = branches;
    while (DBranches != nil)
    {
      if (DBranches->Addr->MrkPlc.has(J))
        ForbPlc = ForbPlc + DBranches->Addr->MrkPlc - set::of(J, eos);
      DBranches = DBranches->Next;
    }
  }

  static void Dye(ptnet_branches_list_t branches,
    PTree Trees,
    integer ColMin, integer PlcMin, integer ColNr,
    TPalete& Color,
    PBranch& aktBranch);

  typedef set Rainbow;



  /*------------------------------------------------------------------------*/

  static void FreeColors(ptnet_places_t Places,
    integer ColNr,
    TPalete& Color,
    Rainbow& ColFree)
  {
    integer I, J;

    ColFree = set::of(range(1, ColNr), eos);
    I = 0;
    do {
      do {
        I += 1;
      } while (!((Places.has(I)) || (I == 256)));
      if (I < 256)
        for (J = 1; J <= ColNr; J++)
          if (Color[J].has(I))  ColFree = ColFree - set::of(J, eos);
    } while (!(I == 256));
  }



  /*------------------------------------------------------------------------*/

  static void GetHowMany(ptnet_branches_list_t branches,
    TPalete Color, TPalete ForbPlc,
    integer Place, integer ColNr,
    byte& HowMany)
  {
    integer I, J, DHowMany, NrCol;

    HowMany = ColNr;
    DHowMany = ColNr;
    while ((branches != nil) && (DHowMany > 0))
    {
      if (branches->Addr->MrkPlc.has(Place))
      {
        I = 0;
        DHowMany = ColNr;
        do {
          do {
            I += 1;
          } while (!((branches->Addr->MrkPlc.has(I)) || (I == 256)));
          if (I < 256)
          {
            NrCol = 0;
            for (J = 1; J <= ColNr; J++)
              if (Color[J].has(I))  NrCol += 1;
            if (NrCol == 0)  NrCol = 1;
            DHowMany = DHowMany - NrCol;
          }
        } while (!(I == 256));
        if (HowMany > DHowMany)  HowMany = DHowMany;
      }
      branches = branches->Next;
    }
  }



  /*==========================================================================*/

  static void Dye(ptnet_branches_list_t branches,
    PTree Trees,
    integer ColMin, integer PlcMin, integer ColNr,
    TPalete& Color,
    PBranch& aktBranch)
  {
    ptnet_places_t PlcAct, Colored;
    integer I, J, K, L, M;
    Rainbow ColFree;
    byte HowMany;

    /*------------------------------------------------------------------------*/


    while (Trees != nil)
    {
      Colored = set::of(eos);
      for (L = 1; L <= ColNr; L++) Colored = Colored + Color[L];
      if (!(Trees->Branch->MrkPlc <= Colored))
      {
        FreeColors(Trees->Branch->MrkPlc, ColNr, Color, ColFree);
        PlcAct = Trees->TranNr->OutPlc - Colored;
        J = PlcMin - 1;
        for (I = ColMin; I <= ColNr; I++)
          if (ColFree.has(I))
          {
            do {
              do {
                J += 1;
              } while (!((PlcAct.has(J)) || (J == 256)));
              if ((!(ForbPlc[I].has(J))) && (J < 256))
              {
                ColFree = ColFree - set::of(I, eos);
                WriteColor(branches, J, Color[I], ForbPlc[I]);
              }
              if (ForbPlc[I].has(J))
              {
                K = ColMin - 1;
                do {
                  do {
                    K += 1;
                  } while (!((ColFree.has(K)) || (K > ColNr)));
                } while (!((!(ForbPlc[K].has(J))) || (K > ColNr)));
                if (K <= ColNr)
                {
                  WriteColor(branches, J, Color[K], ForbPlc[K]);
                  ColFree = ColFree - set::of(K, eos);
                }
              }
            } while (!((!(ForbPlc[I].has(J))) || (J == 256)));
            if ((ColFree != set::of(eos)) && (J == 256))
            {
              PlcAct = Trees->TranNr->OutPlc - Colored;
              J = 0;
              do {
                do {
                  J += 1;
                } while (!((PlcAct.has(J)) || (J == 256)));
                GetHowMany(branches, Color, ForbPlc, J, ColNr, HowMany);
                while ((HowMany > 0) && (ColFree != set::of(eos)))
                {
                  K = 0;
                  do {
                    do {
                      K += 1;
                    } while (!((ColFree.has(K)) || (K > ColNr)));
                  } while (!((!(ForbPlc[J].has(J))) || (J > ColNr)));
                  if (K <= ColNr)
                  {
                    WriteColor(branches, J, Color[K], ForbPlc[K]);
                    ColFree = ColFree - set::of(K, eos);
                    HowMany = HowMany - 1;
                  }
                }
                PlcAct = PlcAct - set::of(J, eos);
              } while (!(ColFree == set::of(eos)));
            }
          }
        Dye(branches, Trees->Branch->RdyTrn, 1, 1, ColNr, Color, Trees->Branch);
      }
      Trees = Trees->NextTree;
    }
  }

  void Coloring(ptnet_branches_list_t branches,
    TPalete& Color,
    integer& ColNr)
  {
    integer I, J;
    PBranch MaxBranch;
    ptnet_places_t PlcAct;
    text Plik;

    /*==========================================================================*/


    MrkMax(branches, MaxBranch, ColNr);
    J = 0;
    PlcAct = MaxBranch->MrkPlc;
    for (I = 1; I <= ColNr; I++)
    {
      do {
        J += 1;
      } while (!(PlcAct.has(J)));
      Color[I] = set::of(eos);
      ForbPlc[I] = set::of(eos);
      WriteColor(branches, J, Color[I], ForbPlc[I]);
    }
    Dye(branches, MaxBranch->RdyTrn, 1, 1, ColNr, Color, MaxBranch);
  }

  /*����������������������������������������������������������������������������*/

  void WhichColor(TPalete MColor,
    integer ColNr,
    TContents contents,
    integer LastMacro,
    TPalete& Color)
  {
    byte I, J;

    for (I = 1; I <= ColNr; I++)
    {
      Color[I] = MColor[I];
      for (J = 101; J <= LastMacro; J++)
        if (Color[I].has(J))
          Color[I] = Color[I] + contents[J] - set::of(J, eos);
    }
  }

  /*����������������������������������������������������������������������������*/

  void GetColored(text& Plik,
    ptnet_transitions_list_t Net,
    TPalete Color,
    integer ColNr, integer What,
    ptnet_places_t& Place,
    boolean Macro, boolean aut);


  /*--------------------------------------------------------------------------*/

  static void WritePlcCl(ptnet_places_t Places, ptnet_places_t ster, ptnet_places_t stersig,
    TPalete Color,
    integer ColNr,
    boolean inp_, boolean prim, boolean Macro, text& Plik)
  {
    byte I, J;
    boolean Start;

    Start = true;
    for (I = 0; I <= 199; I++)
      if (Places.has(I))
      {
        if (!Start)  Plik << " * ";
        else Start = false;
        if (I > 100)
          if (!prim)  Plik << "@M";
          else Plik << 'M';
        if (I <= 100)
          if (!prim)  Plik << '@';
        Plik << 'P' << I << '[';
        Start = true;
        for (J = 1; J <= ColNr; J++)
          if (Color[J].has(I))
          {
            if (!Start)  Plik << ',';
            else Start = false;
            Plik << J;
          }
        Plik << ']';
      }
    if (!(Macro))
      for (I = 0; I <= 99; I++)
        if (ster.has(I))
        {
          if ((stersig.has(I)) && (inp_))  Plik << " * /X" << I;
          else
            if ((stersig.has(I)) && !(inp_))  Plik << " * RY" << I;
            else
              if (inp_)  Plik << " * X" << I;
              else Plik << " * SY" << I;
        }
  }

  void GetColored(text& Plik,
    ptnet_transitions_list_t Net,
    TPalete Color,
    integer ColNr, integer What,
    ptnet_places_t& Place,
    boolean Macro, boolean aut)
  {
    integer I, J, K;
    boolean Consent;
    char Ch;
    boolean Start;

    /*--------------------------------------------------------------------------*/


    if (What == 1)
    {
      Plik << " Coloured net." << NL;
      Plik << "---------------" << NL;
    }
    else
    {
      Plik << " Coloured macronet." << NL;
      Plik << "--------------------" << NL;
    }
    Plik << NL;
    for (I = 1; I <= ColNr; I++)
    {
      Plik << "COLOR: " << format(I, 2) << ",   PLACE: ";
      J = 1;
      Start = true;
      while (J < 200)
      {
        if (Color[I].has(J))
        {
          if (J <= 100)
            if (Start)  Plik << 'P';
            else Plik << ",P";
          else
            if (Start)  Plik << "MP";
            else Plik << ",MP";
          Start = false;
          Plik << J;
        }
        J += 1;
      }
      Plik << NL;
    }
    Plik << NL;
    /*-------------------------------------------------------------------------*/
    EssencialPlc(Place, Color, ColNr);
    Plik << "ESSENTIAL PLACES: ";
    Start = true;
    for (K = 1; K <= 200; K++)
      if (Place.has(K))
      {
        if (K < 100)
          if (Start)  Plik << 'P';
          else Plik << ",P";
        else
          if (Start)  Plik << "MP";
          else Plik << ",MP";
        Start = false;
        Plik << K;
      }
    Plik << NL;
    Plik << NL;
    /*-------------------------------------------------------------------------*/
    Plik << "TRANSITIONS" << NL;
    while (Net != nil)
    {
      Plik << 'T' << Net->Nr << ": ";
      WritePlcCl(Net->InpPlc, Net->inp_, Net->InpN, Color, ColNr, true, true, Macro, Plik);
      Plik << " |- ";
      WritePlcCl(Net->OutPlc, Net->Out, Net->OutR, Color, ColNr, false, false, Macro, Plik);
      Plik << ';' << NL;
      Net = Net->NextTrn;
    }
    Plik << NL;
  }

  /*============================================================================*/

  void WriteColored(FNameStr& FileName,
    ptnet_transitions_list_t Net,
    TPalete Color,
    integer ColNr, integer What,
    ptnet_registers_list_t Reg,
    boolean Macro)
  {
    text Plik;
    byte I, J;
    boolean Start;
    ptnet_places_t Place;
    string Ext;

    if (What == 1)  Ext = "COL";
    else Ext = "MCL";
    PTNetFileNameNew(FileName, Ext);
    assign(Plik, FileName);
    rewrite(Plik);
    GetColored(Plik, Net, Color, ColNr, What, Place, Macro, false);
    if (!Macro)
    {
      Plik << "OUTPUTS" << NL;
      for (I = 1; I <= Reg[0].Plc - 1; I++)
      {
        Plik << 'P' << Reg[I].Plc << '[';
        Start = false;
        for (J = 1; J <= ColNr; J++)
          if (Color[J].has(Reg[I].Plc))
          {
            if (Start)  Plik << ',';
            Plik << J;
            Start = true;
          }
        Plik << "] |- Y" << Reg[I].Out << ';' << NL;
      }
      Plik << 'P' << Reg[I + 1].Plc << '[';
      Start = false;
      for (J = 1; J <= ColNr; J++)
        if (Color[J].has(Reg[I + 1].Plc))
        {
          if (Start)  Plik << ',';
          Plik << J;
          Start = true;
        }
      Plik << "] |- Y" << Reg[I + 1].Out << ';' << NL;
    }
    close(Plik);
  }

  /*����������������������������������������������������������������������������*/

}
