/******************************************************************************/
/*                     Department of Computer Engineering                     */
/*                Higher College of Engineering in Zielona Gora               */
/*                                                                            */
/*                Decomposition and Place Encoding of Petri Net.              */
/*                 Master's thesis, Supervisor Prof. M.Adamski.               */
/*                                                                            */
/*                                            Author: Mariusz Janczak (1993)  */
/******************************************************************************/
/*  Module for decomposition a Petri network into component automata.         */
/******************************************************************************/

#include "automat.h"

namespace automat {


  /*����������������������������������������������������������������������������*/

  void EssencialPlc(ptnet_places_t& Place,
    TPalete Color,
    integer ColorsNr)
  {
    integer I, J, K, L;
    TEssencialPlc EssPlc;
    REssencialPlc EP;

    for (K = 1; K <= 100; K++)
    {
      EssPlc[K].Nr = 0;
      EssPlc[K].Rep = false;
    }
    K = 1;
    Place = set::of(eos);
    for (L = 1; L <= ColorsNr; L++)
    {
      J = 1;
      while (J < 200)
      {
        if (Color[L].has(J))
          if (Place.has(J))
          {
            I = 0;
            do {
              I += 1;
            } while (!(EssPlc[I].Nr == J));
            EssPlc[I].Rep = true;
          }
          else
          {
            EssPlc[K].Nr = J;
            Place = Place + set::of(J, eos);
            K += 1;
          }
        J += 1;
      }
    }
    I = 1;
    while (EssPlc[I].Nr != 0)
    {
      J = I + 1;
      while (EssPlc[J].Nr != 0)
      {
        if (EssPlc[I].Nr > EssPlc[J].Nr)
        {
          EP = EssPlc[I];
          EssPlc[I] = EssPlc[J];
          EssPlc[J] = EP;
        }
        J += 1;
      }
      I += 1;
    }
    Place = set::of(eos);
    K = 1;
    while (EssPlc[K].Nr != 0)
    {
      if (!(EssPlc[K].Rep))  Place = Place + set::of(EssPlc[K].Nr, eos);
      K += 1;
    }
  }

  /*��������������������� Rozklad na skladowe automatowe �����������������������*/

  void Automats(FNameStr& FileName,
    ptnet_input_data_t input_data,
    ptnet_transitions_list_t& Net,
    ptnet_registers_list_t Reg,
    ptnet_places_t Marking,
    TPalete Color,
    integer ColorsNr, integer What);

  static text Plik;

  static PCompAut CompAut, Aut1, Aut2;


  /*==========================================================================*/

  static void AddPlc(PCompAut& CompAut,
    byte L,
    byte& LastPlc,
    ptnet_places_t& APlc)
  {
    byte I, J;
    PCompAut Pom;

    Pom = CompAut;
    I = 0;
    do {
      I += 1;
    } while (!((Pom->Plc[I].Plc == L) || (I > Pom->CondNr)));
    if (I <= Pom->CondNr)
    {
      if (Pom->D.D != set::of(eos))  Pom->CondNr -= 1;
      Pom->D.D = Pom->D.D + set::of(L, eos);
      if (Pom->D.Plc == 0)
      {
        Pom->D.Plc = LastPlc;
        APlc = APlc + set::of(LastPlc, eos);
        LastPlc += 1;
      }
      Pom->MaxFF = round(trunc(log(Pom->CondNr) / log(2)) + 1);
      for (J = 0; J <= Pom->Plc[0].Plc - 2; J++)
      {
        Pom->Plc[I + J].Plc = Pom->Plc[I + J + 1].Plc;
        Pom->Plc[I + J].EssPlc = Pom->Plc[I + J + 1].EssPlc;
      }
      Pom->Plc[Pom->Plc[0].Plc].Plc = 0;
      Pom->Plc[Pom->Plc[0].Plc].EssPlc = false;
      Pom->Plc[0].Plc -= 1;
    }
  }



  /*==========================================================================*/

  static void WrtInData(ptnet_places_t Data,
    TCiag Str1,
    char Chr)
  {
    integer I;
    boolean Start;

    if (Data != set::of(eos))
    {
      Plik << Str1;
      Start = true;
      I = 0;
      while (Data != set::of(eos))
      {
        if (Data.has(I))
          if (Start)
          {
            Plik << Chr << I;
            Start = false;
          }
          else
            Plik << ',' << Chr << I;
        Data = Data - set::of(I, eos);
        I += 1;
      }
      Plik << NL;
    }
  }



  /*==========================================================================*/

  static void GetInput(ptnet_input_data_t input_data,
    ptnet_places_t APlc, ptnet_places_t Marking,
    text& Plik, integer& What)
  {
    integer I;
    boolean Start;

    Plik << " Decomposed Petri net." << NL;
    Plik << "-----------------------" << NL;
    Plik << NL;
    WrtInData(input_data.Plc + APlc, "PLACE ", 'P');
    WrtInData(input_data.inp_, "INPUT ", 'X');
    if (What == 1)  WrtInData(input_data.Reg, "REG_OUTD ", 'Y');
    else WrtInData(input_data.Reg, "REG_OUTSR ", 'Y');
    WrtInData(input_data.Com, "COMB_OUT ", 'Y');
    WrtInData(Marking, "MARKING ", 'P');
    Plik << NL;
  }



  /*==========================================================================*/

  static void WriteTrn(PCompAut CompAut,
    ptnet_places_t Plc, ptnet_places_t Sgnl, ptnet_places_t Sgnl1,
    integer Nr,
    boolean Left, integer& What)
  {
    integer I;
    boolean Start;

    Start = true;
    if (Left)
    {
      Aut2 = CompAut;
      while (Aut2->ColNr != Nr)  Aut2 = Aut2->NextAut;
      if ((Aut2->D.Plc != 0) && (Plc.has(Aut2->D.Plc)))
      {
        Plik << 'P' << Aut2->D.Plc;
        Plc = Plc - set::of(Aut2->D.Plc, eos);
        Start = false;
      }
      if (Start)
        for (I = 1; I <= Aut2->Plc[0].Plc; I++)
          if (Plc.has(Aut2->Plc[I].Plc))
          {
            Plik << 'P' << Aut2->Plc[I].Plc;
            Plc = Plc - set::of(Aut2->Plc[I].Plc, eos);
          }
      if (Plc != set::of(eos))
      {
        Aut2 = CompAut;
        Start = true;
        if (Aut2->ColNr == Nr)  Aut2 = Aut2->NextAut;
        while ((Plc != set::of(eos)) && (Aut2 != nil))
        {
          if ((Aut2->D.Plc != 0) && (Plc.has(Aut2->D.Plc)))
          {
            Plik << " * <P" << Aut2->D.Plc << '[' << Aut2->ColNr << "]>";
            Plc = Plc - set::of(Aut2->D.Plc, eos);
            Start = false;
          }
          if (Start)
            for (I = 1; I <= Aut2->Plc[0].Plc; I++)
              if (Plc.has(Aut2->Plc[I].Plc))
              {
                Plik << " * <P" << Aut2->Plc[I].Plc << '[' << Aut2->ColNr << "]>";
                Plc = Plc - set::of(Aut2->Plc[I].Plc, eos);
              }
          Aut2 = Aut2->NextAut;
          if (Aut2->ColNr == Nr)  Aut2 = Aut2->NextAut;
        }
      }
    }
    else
    {
      I = 1;
      while (!(Plc.has(I)))  I += 1;
      Plik << "@P" << I;
    }
    I = 0;
    while (Sgnl != set::of(eos))
    {
      while (!(Sgnl.has(I)))  I += 1;
      if (Left)
        if (Sgnl1.has(I))  Plik << " * /X" << I;
        else Plik << " * X" << I;
      else
        if (What == 2)
          if (Sgnl1.has(I))  Plik << " * RY" << I;
          else Plik << " * SY" << I;
        else
          if (!(Sgnl1.has(I)))  Plik << " * DY" << I;
      Sgnl = Sgnl - set::of(I, eos);
    }
  }

  void Automats(FNameStr& FileName,
    ptnet_input_data_t input_data,
    ptnet_transitions_list_t& Net,
    ptnet_registers_list_t Reg,
    ptnet_places_t Marking,
    TPalete Color,
    integer ColorsNr, integer What)
  {
    byte I, J, K, L, N, LastPlc;
    boolean Start;
    ptnet_places_t Place, APlc, Plc;
    ptnet_transitions_list_t Trn, Trn1, Trn2;
    string Ext;

    /*==========================================================================*/


    if (What == 1)  Ext = "DCD";
    else Ext = "DKJ";
    PTNetFileNameNew(FileName, Ext);
    assign(Plik, FileName);
    rewrite(Plik);
    LastPlc = 1;
    while (input_data.Plc.has(LastPlc))  LastPlc += 1;
    APlc = set::of(eos);
    EssencialPlc(Place, Color, ColorsNr);
    /*------------------------- Utworzenie automatow --------------------------*/
    Mark(CompAut);
    CompAut = nil;
    Aut2 = CompAut;
    Aut1 = Aut2;
    for (I = 1; I <= ColorsNr; I++)
    {
      J = 1;
      K = 1;
      Aut1 = new TCompAut;
      for (L = 0; L <= 20; L++)
      {
        Aut1->Plc[L].Plc = 0;
        Aut1->Plc[L].EssPlc = false;
      }
      while (J < 100)
      {
        if (Color[I].has(J))
        {
          Aut1->Plc[K].Plc = J;
          if (Place.has(J))  Aut1->Plc[K].EssPlc = true;
          else Aut1->Plc[K].EssPlc = false;
          K += 1;
        }
        J += 1;
      }
      Aut1->Plc[0].Plc = K - 1;
      Aut1->D.Plc = 0;
      Aut1->D.D = set::of(eos);
      Aut1->CondNr = K - 1;
      Aut1->ColNr = I;
      Aut1->MinFF = 0;
      Aut1->MaxFF = 0;
      Aut1->Roz = false;
      Aut1->inp_ = set::of(eos);
      Aut1->Reg = set::of(eos);
      Aut1->Comb = set::of(eos);
      Aut1->TrnNr = set::of(eos);
      Aut1->Trn = nil;
      Aut1->NextAut = nil;
      if (CompAut == nil)  CompAut = Aut1;
      else Aut2->NextAut = Aut1;
      Aut2 = Aut1;
    }
    /*---------------------- Max. liczba przerzutnikow ------------------------*/
    Aut1 = CompAut;
    do {
      Aut1->MaxFF = round(trunc(log(Aut1->CondNr) / log(2)) + 1);
      Aut1 = Aut1->NextAut;
    } while (!(Aut1 == nil));
    /*---------------------- Min. liczba przerzutnikow ------------------------*/
    Aut1 = CompAut;
    for (I = 1; I <= ColorsNr; I++)
    {
      for (J = 1; J <= Aut1->Plc[0].Plc; J++)
        if (Place.has(Aut1->Plc[J].Plc))
          Aut1->MinFF = Aut1->MinFF + 1;
      Aut1->MinFF = round(trunc(log(Aut1->MinFF + 1) / log(2)) + 1);
      Aut1 = Aut1->NextAut;
    }
    /*-------------------- Dopisanie miejsc spoczynkowych ---------------------*/
    N = 0;
    do {
      Aut1 = CompAut;
      K = 0;
      J = 0;
      do {
        {
          TCompAut& with = *Aut1;
          if (!(with.Roz) && (with.MaxFF == with.MinFF) && (with.MinFF > K))
          {
            J = with.ColNr;
            K = with.MinFF;
          }
        }
        Aut1 = Aut1->NextAut;
      } while (!(Aut1 == nil));
      /*--------- ---------------------------------*/
      Aut1 = CompAut;
      if (J == 0)
        do {
          if (!(Aut1->Roz))  J = Aut1->ColNr;
          Aut1 = Aut1->NextAut;
        } while (!((J > 0) || (Aut1 == nil)));
        /*-------------------------------------------*/
        Aut1 = CompAut;
        while (Aut1->ColNr != J)  Aut1 = Aut1->NextAut;
        Aut1->Roz = true;
        for (I = 1; I <= Aut1->Plc[0].Plc; I++)
          if (!(Aut1->Plc[I].EssPlc))
          {
            Aut2 = CompAut;
            L = Aut1->Plc[I].Plc;
            do {
              if (!(Aut2->Roz))  AddPlc(Aut2, L, LastPlc, APlc);
              Aut2 = Aut2->NextAut;
            } while (!(Aut2 == nil));
          }
        N += 1;
    } while (!(N == ColorsNr - 1));
    /*---------------- Przypisanie wyjsc rejestrowych automatom ---------------*/
    Aut1 = CompAut;
    while (Aut1 != nil)
    {
      for (I = 1; I <= Aut1->Plc[0].Plc; I++)
        for (J = 1; J <= Reg[0].Plc; J++)
          if (Aut1->Plc[I].Plc == Reg[J].Plc)
            Aut1->Comb = Aut1->Comb + set::of(Reg[J].Out, eos);
      Aut1 = Aut1->NextAut;
    }
    /*-------------- Przypisanie miejsc, wejsc i wyjsc automatom --------------*/
    Trn = Net;
    while (Trn != nil)
    {
      Aut1 = CompAut;
      while (Aut1 != nil)
      {
        for (I = 1; I <= Aut1->Plc[0].Plc; I++)
          if ((Trn->InpPlc.has(Aut1->Plc[I].Plc)) ||
            (Trn->OutPlc.has(Aut1->Plc[I].Plc)))
          {
            Aut1->TrnNr = Aut1->TrnNr + set::of(Trn->Nr, eos);
            Aut1->inp_ = Aut1->inp_ + Trn->inp_;
            if (What == 1)
            {
              Plc = Trn->Out;
              J = 0;
              while (Plc != set::of(eos))
              {
                if (Trn->Out.has(J))
                  if (!(Trn->OutR.has(J)))
                    Aut1->Reg = Aut1->Reg + set::of(J, eos);
                Plc = Plc - set::of(J, eos);
                J += 1;
              }
            }
            else
              Aut1->Reg = Aut1->Reg + Trn->Out;
          }
        Aut1 = Aut1->NextAut;
      }
      Trn = Trn->NextTrn;
    }
    /*-------------------- Utworzenie tranzycji automatow ---------------------*/
    Aut1 = CompAut;
    while (Aut1 != nil)
    {
      Trn = Net;
      while (Trn != nil)
      {
        if (Aut1->TrnNr.has(Trn->Nr))
        {
          Trn1 = new ptnet_transition_t;
          Trn1->NextTrn = nil;
          Trn1->Nr = Trn->Nr;
          Trn1->InpPlc = set::of(eos);
          Trn1->OutPlc = set::of(eos);
          Trn1->inp_ = Trn->inp_;
          Trn1->Out = Trn->Out;
          Trn1->InpN = Trn->InpN;
          Trn1->OutR = Trn->OutR;
          Aut2 = CompAut;
          while (Aut2 != nil)
          {
            for (I = 1; I <= Aut2->Plc[0].Plc; I++)
              if (Trn->InpPlc.has(Aut2->Plc[I].Plc))
                Trn1->InpPlc = Trn1->InpPlc + set::of(Aut2->Plc[I].Plc, eos);
            if ((Aut2->D.Plc != 0) && (Aut2->TrnNr.has(Trn->Nr)))
            {
              I = 1;
              Plc = Aut2->D.D;
              while (Plc != set::of(eos))
              {
                if (Plc.has(I))
                {
                  if (Trn->InpPlc.has(I))
                    Trn1->InpPlc = Trn1->InpPlc + set::of(Aut2->D.Plc, eos);
                  Plc = Plc - set::of(I, eos);
                }
                I += 1;
              }
            }
            Aut2 = Aut2->NextAut;
          }
          for (I = 1; I <= Aut1->Plc[0].Plc; I++)
            if (Trn->OutPlc.has(Aut1->Plc[I].Plc))
              Trn1->OutPlc = Trn1->OutPlc + set::of(Aut1->Plc[I].Plc, eos);
          if ((Trn1->OutPlc == set::of(eos)) && (Aut1->D.Plc != 0))
            Trn1->OutPlc = Trn1->OutPlc + set::of(Aut1->D.Plc, eos);
          if (Aut1->Trn == nil)  Aut1->Trn = Trn1;
          else Trn2->NextTrn = Trn1;
          Trn2 = Trn1;
        }
        Trn = Trn->NextTrn;
      }
      Aut1 = Aut1->NextAut;
    }
    /*----------------------- Wyswietlenie automatow --------------------------*/
    GetInput(input_data, APlc, Marking, Plik, What);
    Aut1 = CompAut;
    do {
      Start = true;
      Plik << "AUTOMAT" << Aut1->ColNr << NL;
      Plik << "PLACE ";
      if (Aut1->D.Plc != 0)
      {
        Plik << 'P' << Aut1->D.Plc;
        Start = false;
      }
      for (I = 1; I <= Aut1->Plc[0].Plc; I++)
        if (Start)
        {
          Plik << 'P' << Aut1->Plc[I].Plc;
          Start = false;
        }
        else
          Plik << ",P" << Aut1->Plc[I].Plc;
      Plik << NL;
      WrtInData(Aut1->inp_, "INPUT ", 'X');
      if (What == 1)  WrtInData(Aut1->Reg, "REG_OUTD ", 'Y');
      else WrtInData(Aut1->Reg, "REG_OUTSR ", 'Y');
      WrtInData(Aut1->Comb, "COMB_OUT ", 'Y');
      Plik << "MARKING ";
      if (Aut1->D.Plc != 0)
        Plik << 'P' << Aut1->D.Plc << NL;
      else
        for (I = 1; I <= Aut1->Plc[0].Plc; I++)
          if (Marking.has(Aut1->Plc[I].Plc))
            Plik << 'P' << Aut1->Plc[I].Plc << NL;
      Plik << "TRANSITION" << NL;
      Trn = Aut1->Trn;
      while (Trn != nil)
      {
        Plik << 'T' << Trn->Nr << ": ";
        WriteTrn(CompAut, Trn->InpPlc, Trn->inp_, Trn->InpN, Aut1->ColNr, true, What);
        Plik << " |- ";
        WriteTrn(CompAut, Trn->OutPlc, Trn->Out, Trn->OutR, Aut1->ColNr, false, What);
        Plik << ';' << NL;
        Trn = Trn->NextTrn;
      }
      Plik << NL;
      if (Aut1->Comb != set::of(eos))
      {
        /*writeln(Plik,'OUTPUTS');*/
        Plc = Aut1->Comb;
        for (I = 1; I <= Reg[0].Plc; I++)
          if (Aut1->Comb.has(Reg[I].Out))
            Plik << 'P' << Reg[I].Plc << " |- Y" << Reg[I].Out << ';' << NL;
      }
      Plik << NL;
      Aut1 = Aut1->NextAut;
    } while (!(Aut1 == nil));
    /*-------------------------------------------------------------------------*/
    Release(CompAut);
    close(Plik);
  }

  /*����������������������������������������������������������������������������*/

}
