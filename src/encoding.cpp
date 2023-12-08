/******************************************************************************/
/*                     Department of Computer Engineering                     */
/*                Higher College of Engineering in Zielona Gora               */
/*                                                                            */
/*                Decomposition and Place Encoding of Petri Net.              */
/*                 Master's thesis, Supervisor Prof. M.Adamski.               */
/*                                                                            */
/*                                            Author: Mariusz Janczak (1993)  */
/******************************************************************************/
/*  Module for encoding a Petri network places.                               */
/******************************************************************************/

#include "encoding.h"

namespace encoding {


  /*������������������ Tworzenie listy rownoleglosci makromiejsc ���������������*/

  void ListOfConcurrency(FNameStr& FileName,
    boolean WriteFile,
    ptnet_places_t macronet_places,
    PPeak& Peak,
    PConcurrency& Concurrency);


  /*==========================================================================*/

  static void CheckConc(PConcurrency Conc,
    byte AktMc,
    boolean& ItIs,
    PConcurrency& AktConc)
  {
    ItIs = false;
    while ((ItIs == false) && (Conc != nil))
    {
      if (Conc->McNr == AktMc)
      {
        ItIs = true;
        AktConc = Conc;
      }
      Conc = Conc->Next;
    }
  }



  /*==========================================================================*/

  static void WriteConcurrency(FNameStr& FileName,
    PConcurrency Concurrency)
  {
    text Plik;
    byte I;


    PTNetFileNameNew(FileName, "CON");
    assign(Plik, FileName);
    rewrite(Plik);
    Plik << " Macroplaces concurrency." << NL;
    Plik << "--------------------------" << NL;
    Plik << NL;
    while (Concurrency != nil)
    {
      Plik << "   " << format(Concurrency->McNr, 3) << " :";
      for (I = 0; I <= 199; I++)
        if (Concurrency->ConcMc.has(I))  Plik << format(I, 4);
      Plik << NL;
      Concurrency = Concurrency->Next;
    }
    close(Plik);
  }

  void ListOfConcurrency(FNameStr& FileName,
    boolean WriteFile,
    ptnet_places_t macronet_places,
    PPeak& Peak,
    PConcurrency& Concurrency)
  {
    byte PrncMc, SecMc;
    PConcurrency AktConc, NewConc, OldConc;
    boolean ItIs;
    PPeak Peak1;

    /*==========================================================================*/


    Concurrency = nil;
    AktConc = nil;
    Peak1 = nil;
    Peak1 = Peak;
    while (Peak1 != nil)
    {
      for (PrncMc = 0; PrncMc <= 199; PrncMc++)
        if (Peak1->PeakCont.has(PrncMc))
        {
          CheckConc(Concurrency, PrncMc, ItIs, AktConc);
          if (!ItIs)
          {
            if (Concurrency == nil)
            {
              NewConc = new TConcurrency;
              NewConc->McNr = PrncMc;
              /*
              NewConc^.ConcMc:=[];
              */
              FillChar(NewConc->ConcMc, sizeof(NewConc->ConcMc), 0);
              NewConc->Next = nil;
              Concurrency = NewConc;
              OldConc = NewConc;
              AktConc = NewConc;
            }
            else
            {
              NewConc = new TConcurrency;
              NewConc->McNr = PrncMc;
              NewConc->ConcMc = set::of(eos);
              NewConc->Next = nil;
              OldConc->Next = NewConc;
              OldConc = NewConc;
              AktConc = NewConc;
            }
          }
          for (SecMc = 0; SecMc <= 199; SecMc++)
            if ((Peak1->PeakCont.has(SecMc)) && (SecMc != PrncMc) &&
              (!(AktConc->ConcMc.has(SecMc))))
              AktConc->ConcMc = AktConc->ConcMc + set::of(SecMc, eos);
        }
      Peak1 = Peak1->Next;
    }
    if (WriteFile)  WriteConcurrency(FileName, Concurrency);
  }

  /*���������������� Tworzenie listy nierownoleglosci makromiejsc ��������������*/

  void ListOfNonConcurrency(FNameStr& FileName,
    boolean WriteFile,
    PConcurrency& Concurrency,
    ptnet_places_t macronet_places,
    PNonconcurrency& NonConcurrency);


  /*==========================================================================*/

  static void WriteNonConcurrency(FNameStr& FileName,
    PNonconcurrency NonConcurrency)
  {
    text Plik;
    byte I;


    PTNetFileNameNew(FileName, "NON");
    assign(Plik, FileName);
    rewrite(Plik);
    Plik << " Macroplaces nonconcurrency." << NL;
    Plik << "-----------------------------" << NL;
    Plik << NL;
    while (NonConcurrency != nil)
    {
      Plik << "   " << format(NonConcurrency->McNr, 3) << " :";
      for (I = 0; I <= 199; I++)
        if (NonConcurrency->NoncMc.has(I))  Plik << format(I, 4);
      Plik << NL;
      NonConcurrency = NonConcurrency->Next;
    }
    close(Plik);
  }

  void ListOfNonConcurrency(FNameStr& FileName,
    boolean WriteFile,
    PConcurrency& Concurrency,
    ptnet_places_t macronet_places,
    PNonconcurrency& NonConcurrency)
  {
    byte Nr;
    PConcurrency Conc;
    PNonconcurrency NewNonc, OldNonc;

    /*==========================================================================*/


    NonConcurrency = nil;
    Conc = Concurrency;
    while (Conc != nil)
    {
      if (NonConcurrency == nil)
      {
        NewNonc = new TNonconcurrency;
        NewNonc->McNr = Conc->McNr;
        NewNonc->NoncMc = set::of(eos);
        NewNonc->Next = nil;
        OldNonc = NewNonc;
        NonConcurrency = NewNonc;
      }
      else
      {
        NewNonc = new TNonconcurrency;
        NewNonc->McNr = Conc->McNr;
        NewNonc->NoncMc = set::of(eos);
        NewNonc->Next = nil;
        OldNonc->Next = NewNonc;
        OldNonc = NewNonc;
      }
      for (Nr = 1; Nr <= 199; Nr++)
        if ((macronet_places.has(Nr)) && (Nr != Conc->McNr) &&
          (!(Conc->ConcMc.has(Nr))))
          NewNonc->NoncMc = NewNonc->NoncMc + set::of(Nr, eos);
      Conc = Conc->Next;
    }
    if (WriteFile)  WriteNonConcurrency(FileName, NonConcurrency);
  }

  /*�������������� Tworzenie zweryfikowanej listy nierownoleglosci �������������*/

  void VerifiedList(FNameStr& FileName,
    boolean WriteFile,
    PNonconcurrency NonConcurrency,
    PVerified& Verified);
  static void WriteVerified(FNameStr& FileName,
    PVerified Verified);


  /*------------------------------------------------------------------------*/

  static void HowManyUnite(PVerified Verified,
    byte& maxil)

  {
    maxil = 0;
    while (Verified != nil)
    {
      if (Verified->UniteNr > maxil)  maxil = Verified->UniteNr;
      Verified = Verified->Next;
    }
  }



  /*==========================================================================*/

  static void WriteVerified(FNameStr& FileName,
    PVerified Verified)
  {
    text Plik;
    byte i, ilepustych, maxil;

    /*------------------------------------------------------------------------*/


    PTNetFileNameNew(FileName, "VER");
    assign(Plik, FileName);
    rewrite(Plik);
    Plik << " Verified macroplaces nonconcurrency." << NL;
    Plik << "--------------------------------------" << NL;
    Plik << NL;
    HowManyUnite(Verified, maxil);
    while (Verified != nil)
    {
      Plik << "   ";
      ilepustych = maxil - Verified->UniteNr;
      i = 0;
      while (i < ilepustych)
      {
        Plik << "    ";
        i += 1;
      }
      for (i = 1; i <= 199; i++)
        if (Verified->UniteMc.has(i))  Plik << format(i, 4);
      Plik << " :";
      for (i = 0; i <= 199; i++)
        if (Verified->NoncMc.has(i))  Plik << format(i, 4);
      Plik << NL;
      Verified = Verified->Next;
    }
    close(Plik);
  }

  void VerifiedList(FNameStr& FileName,
    boolean WriteFile,
    PNonconcurrency NonConcurrency,
    PVerified& Verified)
  {
    PVerified W1;
    boolean wpisane;
    PNonconcurrency Nonc;
    PVerified NewVrf, OldVrf;

    /*==========================================================================*/


    Verified = nil;
    Nonc = NonConcurrency;
    while (Nonc != nil)
    {
      W1 = Verified;
      wpisane = false;
      while ((W1 != nil) && (!wpisane))
      {
        if (Nonc->NoncMc == W1->NoncMc)
        {
          W1->UniteMc = W1->UniteMc + set::of(Nonc->McNr, eos);
          W1->UniteNr = succ(byte, W1->UniteNr);
          wpisane = true;
        }
        W1 = W1->Next;
      }
      if (!wpisane)
      {
        if (Verified == nil)
        {
          NewVrf = new TVerified;
          NewVrf->IncLev = 0;
          NewVrf->UniteNr = 0;
          NewVrf->UniteMc = set::of(eos);
          NewVrf->NoncMc = set::of(eos);
          NewVrf->Next = nil;
          OldVrf = NewVrf;
          Verified = NewVrf;
        }
        else
        {
          NewVrf = new TVerified;
          NewVrf->IncLev = 0;
          NewVrf->UniteNr = 0;
          NewVrf->UniteMc = set::of(eos);
          NewVrf->NoncMc = set::of(eos);
          NewVrf->Next = nil;
          OldVrf->Next = NewVrf;
          OldVrf = NewVrf;
        }
        NewVrf->UniteNr = 1;
        NewVrf->UniteMc = NewVrf->UniteMc + set::of(Nonc->McNr, eos);
        NewVrf->NoncMc = NewVrf->NoncMc + Nonc->NoncMc;
      }
      Nonc = Nonc->Next;
    }
    if (WriteFile)  WriteVerified(FileName, Verified);
  }

  /*�������������������������� Kodowanie makromiejsc ���������������������������*/

  void CodesOfMcPlaces(FNameStr& FileName,
    boolean WriteFile,
    ptnet_places_t macronet_places,
    PConcurrency& Concurrency,
    PVerified Verified,
    PCodes& McCodes);


  /*==========================================================================*/

  static void CreateNewCode(PCodes& McCodes,
    ptnet_places_t macronet_places)
  {
    PCodes NewCode;
    byte Nr;

    McCodes = nil;
    for (Nr = 199; Nr >= 0; Nr--)
      if (macronet_places.has(Nr))
      {
        NewCode = new TCodes;
        NewCode->McNr = Nr;
        NewCode->PlcCode = nil;
        NewCode->Next = McCodes;
        McCodes = NewCode;
      }
  }



  /*==========================================================================*/

  static void FirstCode(PCodes McCodes)

  {
    PFlipFlop NewFF;

    while (McCodes != nil)
    {
      NewFF = new TFlipFlop;
      NewFF->FFNr = 1;
      NewFF->Ch = '-';
      NewFF->Next = nil;
      McCodes->PlcCode = NewFF;
      McCodes = McCodes->Next;
    }
  }

  static void IncidenceLevel(PVerified Verified,
    PVerified& MaxIncLev);


  /*------------------------------------------------------------------------*/

  static void MaxIncLevel(PVerified Verified,
    PVerified& MaxIncLev)
  {
    MaxIncLev = Verified;
    while (Verified != nil)
    {
      if (Verified->IncLev > MaxIncLev->IncLev)  MaxIncLev = Verified;
      Verified = Verified->Next;
    }
  }



  /*==========================================================================*/

  static void IncidenceLevel(PVerified Verified,
    PVerified& MaxIncLev)
  {
    PVerified SecVrf;
    byte IncL, i;

    /*------------------------------------------------------------------------*/


    SecVrf = Verified;
    while (SecVrf != nil)
    {
      IncL = 0;
      for (i = 0; i <= 199; i++)
        if (SecVrf->NoncMc.has(i))  IncL = succ(byte, IncL);
      SecVrf->IncLev = SecVrf->UniteNr * IncL;
      SecVrf = SecVrf->Next;
    }
    MaxIncLevel(Verified, MaxIncLev);
  }

  static void EncodProc(PVerified MaxIncLev,
    PCodes McCodes,
    PConcurrency Concurrency);


  /*------------------------------------------------------------------------*/

  static void FindFF(PFlipFlop& FFList,
    PCodes McCodes,
    byte Nr)
  {
    while (McCodes->McNr != Nr)  McCodes = McCodes->Next;
    FFList = McCodes->PlcCode;
    while (FFList->Next != nil)  FFList = FFList->Next;
  }

  static void ChangeToStar(PCodes McCodes,
    PConcurrency Concurrency);


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  static void CheckConcCode(PCodes McCodes,
    PConcurrency Concurrency,
    byte McNr,
    boolean& Change)
  {
    byte I;
    PFlipFlop SecFF;

    Change = false;
    while ((Concurrency->McNr != McNr) && (Concurrency != nil))
      Concurrency = Concurrency->Next;
    I = 0;
    while ((I < 200) && (Change == false))
    {
      if (Concurrency->ConcMc.has(I))
      {
        FindFF(SecFF, McCodes, I);
        if ((SecFF->Ch == '0') || (SecFF->Ch == '1'))  Change = true;
      }
      I = succ(byte, I);
    }
  }



  /*------------------------------------------------------------------------*/

  static void ChangeToStar(PCodes McCodes,
    PConcurrency Concurrency)
  {
    PFlipFlop FFList;
    byte McNr;
    boolean Change;
    PCodes SecCods;

    /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


    SecCods = McCodes;
    while (SecCods != nil)
    {
      FFList = SecCods->PlcCode;
      while (FFList->Next != nil)  FFList = FFList->Next;
      if (FFList->Ch == '-')
      {
        McNr = SecCods->McNr;
        CheckConcCode(McCodes, Concurrency, McNr, Change);
        if (Change == true)  FFList->Ch = '*';
      }
      SecCods = SecCods->Next;
    }
  }



  /*==========================================================================*/

  static void EncodProc(PVerified MaxIncLev,
    PCodes McCodes,
    PConcurrency Concurrency)
  {
    byte i, j;
    PFlipFlop FFList;
    PConcurrency SecConc;

    /*------------------------------------------------------------------------*/


    for (i = 0; i <= 199; i++)
    {
      SecConc = Concurrency;
      if (MaxIncLev->UniteMc.has(i))
      {
        FindFF(FFList, McCodes, i);
        FFList->Ch = '0';
        while ((SecConc->McNr != i) && (SecConc != nil))
          SecConc = SecConc->Next;
        if (SecConc != nil)
          for (j = 0; j <= 199; j++)
            if (SecConc->ConcMc.has(j))
            {
              FindFF(FFList, McCodes, j);
              if (FFList->Ch == '-')  FFList->Ch = '*';
            }
      }
      if (MaxIncLev->NoncMc.has(i))
      {
        FindFF(FFList, McCodes, i);
        FFList->Ch = '1';
      }
    }
    ChangeToStar(McCodes, Concurrency);
  }



  /*==========================================================================*/

  static void RemoveVrfItem(PVerified& Verified,
    PVerified MaxIncLev)
  {
    PVerified SecVrf;

    if (Verified == MaxIncLev)  Verified = MaxIncLev->Next;
    else
    {
      SecVrf = Verified;
      while (SecVrf->Next != MaxIncLev)  SecVrf = SecVrf->Next;
      SecVrf->Next = MaxIncLev->Next;
    }
  }



  /*==========================================================================*/

  static void RemoveMcItem(PVerified Verified,
    PVerified MaxIncLev)
  {
    PVerified SecVrf;
    byte I;

    for (I = 0; I <= 199; I++)
      if (MaxIncLev->UniteMc.has(I))
      {
        SecVrf = Verified;
        while (SecVrf != nil)
        {
          if (SecVrf->NoncMc.has(I))
            SecVrf->NoncMc = SecVrf->NoncMc - set::of(I, eos);
          SecVrf = SecVrf->Next;
        }
      }
  }



  /*==========================================================================*/

  static void IntroFF(PCodes McCodes)
  {
    PFlipFlop FFList, NewFF;

    while (McCodes != nil)
    {
      FFList = McCodes->PlcCode;
      while (FFList->Next != nil)  FFList = FFList->Next;
      NewFF = new TFlipFlop;
      NewFF->FFNr = succ(byte, FFList->FFNr);
      NewFF->Ch = '-';
      NewFF->Next = nil;
      FFList->Next = NewFF;
      McCodes = McCodes->Next;
    }
  }



  /*==========================================================================*/

  static void WriteCodesOfMcPlaces(FNameStr& FileName,
    PCodes McCodes)
  {
    text Plik;
    byte I;
    string Ch;
    PFlipFlop SecFF;
    string StrPlc;


    PTNetFileNameNew(FileName, "CMC");
    assign(Plik, FileName);
    rewrite(Plik);
    Plik << " Codes of macroplaces." << NL;
    Plik << "-----------------------" << NL;
    Plik << NL;
    while (McCodes != nil)
    {
      str(McCodes->McNr, StrPlc);
      StrPlc = concat("   MP", StrPlc);
      while (length(StrPlc) <= 8)  insert(" ", StrPlc, length(StrPlc) + 1);
      Plik << StrPlc << " = ";
      SecFF = McCodes->PlcCode;
      Ch = ' ';
      while (SecFF != nil)
      {
        if (set::of('0', '1', eos).has(SecFF->Ch))
        {
          Plik << Ch;
          Ch = " * ";
          if (SecFF->Ch == '0')  Plik << "/Q";
          else Plik << 'Q';
          switch (SecFF->FFNr) {
            case RANGE_10(0, 9): Plik << format(SecFF->FFNr, 1);
              break;
            case 10 ... 99: Plik << format(SecFF->FFNr, 2); break;
            case 100 ... 255: Plik << format(SecFF->FFNr, 3); break;
            default:
              Plik << format(SecFF->FFNr, 5);
          }
        }
        SecFF = SecFF->Next;
      }
      McCodes = McCodes->Next;
      Plik << NL;
    }
    close(Plik);
  }

  void CodesOfMcPlaces(FNameStr& FileName,
    boolean WriteFile,
    ptnet_places_t macronet_places,
    PConcurrency& Concurrency,
    PVerified Verified,
    PCodes& McCodes)
  {
    PVerified MaxIncLev;

    /*==========================================================================*/


    CreateNewCode(McCodes, macronet_places);
    IncidenceLevel(Verified, MaxIncLev);
    if (MaxIncLev->IncLev > 0)
    {
      FirstCode(McCodes);
      while ((MaxIncLev->IncLev > 0) && (Verified != nil))
      {
        EncodProc(MaxIncLev, McCodes, Concurrency);
        RemoveVrfItem(Verified, MaxIncLev);
        RemoveMcItem(Verified, MaxIncLev);
        IncidenceLevel(Verified, MaxIncLev);
        if (MaxIncLev != nil)
          if (MaxIncLev->IncLev > 0)  IntroFF(McCodes);
      }
    }
    if (WriteFile)  WriteCodesOfMcPlaces(FileName, McCodes);
  }

  /*������������������������ Kodowanie miejsc sieci ����������������������������*/

  void CodesOfPlaces(FNameStr& FileName,
    boolean WriteFile, boolean Rst,
    PConcurrency Concurrency,
    PPeak Peak,
    ptnet_places_t Marking, ptnet_places_t macronet_places,
    byte LastMacro,
    TContents contents,
    PCodes McCodes,
    PCodes& PlcCodes,
    PListCodes& ListCodes);


  /*==========================================================================*/

  static void ContentsOfMacro(TContents contents,
    byte LastMacro,
    ptnet_places_t macronet_places,
    PContMc& CntntsMc)
  {
    byte I;
    PContMc NewMc;

    CntntsMc = nil;
    for (I = LastMacro; I >= 101; I--)
    {
      NewMc = new TContMc;
      NewMc->McNr = I;
      NewMc->ContMc = contents[I];
      NewMc->Next = CntntsMc;
      CntntsMc = NewMc;
    }
    for (I = 99; I >= 0; I--)
      if (macronet_places.has(I))

      {
        NewMc = new TContMc;
        NewMc->McNr = I;
        NewMc->ContMc = set::of(I, eos);
        NewMc->Next = CntntsMc;
        CntntsMc = NewMc;
      }
  }



  /*==========================================================================*/

  static void McPrice(PContMc CntntsMc)
  {
    byte HMPlc, I;

    while (CntntsMc != nil)
    {
      HMPlc = 0;
      for (I = 0; I <= 99; I++)
        if (CntntsMc->ContMc.has(I))  HMPlc += 1;
      switch (HMPlc) {
        case 1: CntntsMc->Price = 0; break;
        case 2: CntntsMc->Price = 1; break;
        case RANGE_2(3, 4): CntntsMc->Price = 2;
          break;
        case RANGE_4(5, 8): CntntsMc->Price = 3;
          break;
        case RANGE_8(9, 16): CntntsMc->Price = 4;
          break;
        case RANGE_16(17, 32): CntntsMc->Price = 5;
          break;
        case RANGE_32(33, 64): CntntsMc->Price = 6;
          break;
        case 64 ... 99: CntntsMc->Price = 7; break;
      }
      CntntsMc = CntntsMc->Next;
    }
  }



  /*==========================================================================*/

  static void FindMcPrice(byte Nr,
    byte& Price,
    PContMc ContMc)
  {
    while ((ContMc->McNr != Nr) && (ContMc != nil))
      ContMc = ContMc->Next;
    Price = ContMc->Price;
  }



  /*==========================================================================*/

  static void MaxClkPrice(PPeak& MaxClk,
    PPeak Peak,
    PContMc ContMc)
  {
    byte Price, ClkPrice, MaxClkPrc, I;

    MaxClkPrc = 0;
    MaxClk = nil;
    while (Peak != nil)
    {
      ClkPrice = 0;
      for (I = 0; I <= 199; I++)
        if (Peak->PeakCont.has(I))
        {
          FindMcPrice(I, Price, ContMc);
          ClkPrice = ClkPrice + Price;
        }
      if (ClkPrice > MaxClkPrc)
      {
        MaxClkPrc = ClkPrice;
        MaxClk = Peak;
      }
      Peak = Peak->Next;
    }
  }

  static void AddFF(PPeak MaxClk,
    PContMc ContMc,
    PCodes McCodes,
    PConcurrency Concurrency);
  static void introff1(PCodes McCodes);


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  static void firstcode1(PCodes McCodes)
  {
    PFlipFlop NewFF;

    while (McCodes != nil)
    {
      NewFF = new TFlipFlop;
      NewFF->FFNr = 1;
      NewFF->Ch = '-';
      NewFF->Next = nil;
      McCodes->PlcCode = NewFF;
      McCodes = McCodes->Next;
    }
  }



  /*------------------------------------------------------------------------*/

  static void introff1(PCodes McCodes)
  {
    PFlipFlop FFList, NewFF;

    /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


    if (McCodes->PlcCode == nil)  firstcode1(McCodes);
    else
    {
      while (McCodes != nil)
      {
        FFList = McCodes->PlcCode;
        while (FFList->Next != nil)  FFList = FFList->Next;
        NewFF = new TFlipFlop;
        NewFF->FFNr = succ(byte, FFList->FFNr);
        NewFF->Ch = '-';
        NewFF->Next = nil;
        FFList->Next = NewFF;
        McCodes = McCodes->Next;
      }
    }
  }



  /*------------------------------------------------------------------------*/

  static void FindLastFF(PFlipFlop& FFList,
    PCodes McCodes,
    byte Nr)
  {
    while (McCodes->McNr != Nr)  McCodes = McCodes->Next;
    FFList = McCodes->PlcCode;
    while (FFList->Next != nil)  FFList = FFList->Next;
  }



  /*==========================================================================*/

  static void AddFF(PPeak MaxClk,
    PContMc ContMc,
    PCodes McCodes,
    PConcurrency Concurrency)
  {
    byte PrncMc, SecMc, FFNr, Price;
    PConcurrency SecConc;
    PFlipFlop FFList;

    /*------------------------------------------------------------------------*/


    for (PrncMc = 0; PrncMc <= 199; PrncMc++)
      if (MaxClk->PeakCont.has(PrncMc))
      {
        FFNr = 0;
        FindMcPrice(PrncMc, Price, ContMc);
        while (FFNr < Price)
        {
          introff1(McCodes);
          SecConc = Concurrency;
          while (SecConc->McNr != PrncMc)  SecConc = SecConc->Next;
          for (SecMc = 0; SecMc <= 199; SecMc++)
            if (SecConc->ConcMc.has(SecMc))
            {
              FindLastFF(FFList, McCodes, SecMc);
              FFList->Ch = '*';
            }
          FFNr = succ(byte, FFNr);
        }
      }
  }



  /*==========================================================================*/

  static void RemoveInfo(byte Nr,
    PConcurrency& Concurrency)
  {
    PConcurrency SecConc, W1;

    SecConc = Concurrency;
    if (SecConc->McNr == Nr)
    {
      Concurrency = Concurrency->Next;
      delete SecConc;
    }
    else
    {
      while (SecConc->Next->McNr != Nr)  SecConc = SecConc->Next;
      W1 = SecConc->Next;
      SecConc->Next = W1->Next;
      delete W1;
    }
    SecConc = Concurrency;
    while (SecConc != nil)
    {
      SecConc->ConcMc = SecConc->ConcMc - set::of(Nr, eos);
      SecConc = SecConc->Next;
    }
  }



  /*==========================================================================*/

  static void FindFreeFF(PFlipFlop& FFList,
    PCodes McCodes,
    byte Nr)
  {
    while (McCodes->McNr != Nr)  McCodes = McCodes->Next;
    FFList = McCodes->PlcCode;
    while ((FFList->Ch != '-') && (FFList != nil))  FFList = FFList->Next;
  }

  static void Block(PConcurrency& Concurrency,
    PPeak MaxClk,
    PContMc ContMc,
    PCodes McCodes);


  /*------------------------------------------------------------------------*/

  static void BlockFF(byte FFNr,
    PCodes McCodes,
    byte Nr)
  {
    PFlipFlop wprz;

    while (McCodes->McNr != Nr)  McCodes = McCodes->Next;
    wprz = McCodes->PlcCode;
    while (wprz->FFNr != FFNr)  wprz = wprz->Next;
    wprz->Ch = '*';
  }



  /*==========================================================================*/

  static void Block(PConcurrency& Concurrency,
    PPeak MaxClk,
    PContMc ContMc,
    PCodes McCodes)
  {
    byte I, J, Price;
    PFlipFlop FFList;

    /*------------------------------------------------------------------------*/


    for (I = 0; I <= 199; I++)
      if (MaxClk->PeakCont.has(I))  RemoveInfo(I, Concurrency);
    while (Concurrency != nil)
    {
      if (Concurrency->ConcMc != set::of(eos))
      {
        FindMcPrice(Concurrency->McNr, Price, ContMc);
        if (Price > 0)
        {
          FindFreeFF(FFList, McCodes, Concurrency->McNr);
          for (I = 1; I <= Price; I++)
          {
            for (J = 0; J <= 199; J++)
              if (Concurrency->ConcMc.has(J))
                BlockFF(FFList->FFNr, McCodes, J);
            FFList = FFList->Next;
            while (FFList->Ch != '-')  FFList = FFList->Next;
          }
        }
      }
      RemoveInfo(Concurrency->McNr, Concurrency);
    }
  }

  static void ListOfPlcCodes(PContMc ContMc,
    PCodes McCodes,
    PCodes& PlcCodes);


  /*------------------------------------------------------------------------*/

  static void CopyCodes(PFlipFlop McFF,
    PFlipFlop& PlcFF)
  {
    PFlipFlop FFList, NewFF;

    NewFF = new TFlipFlop;
    PlcFF = NewFF;
    NewFF->FFNr = McFF->FFNr;
    NewFF->Ch = McFF->Ch;
    NewFF->Next = nil;
    FFList = NewFF;
    McFF = McFF->Next;
    while (McFF != nil)
    {
      NewFF = new TFlipFlop;
      NewFF->FFNr = McFF->FFNr;
      NewFF->Ch = McFF->Ch;
      NewFF->Next = nil;
      FFList->Next = NewFF;
      FFList->Next = NewFF;
      FFList = NewFF;
      McFF = McFF->Next;
    }
  }



  /*==========================================================================*/

  static void ListOfPlcCodes(PContMc ContMc,
    PCodes McCodes,
    PCodes& PlcCodes)
  {
    byte miejsce;
    PCodes NewPlcCod, PlcCod, McCod;
    PFlipFlop PlcFF, McFF;

    /*------------------------------------------------------------------------*/


    PlcCodes = nil;
    while (ContMc != nil)
    {
      for (miejsce = 0; miejsce <= 199; miejsce++)
        if (ContMc->ContMc.has(miejsce))
        {
          NewPlcCod = new TCodes;
          NewPlcCod->PlcNr = miejsce;
          NewPlcCod->McNr = ContMc->McNr;
          NewPlcCod->PlcCode = nil;
          NewPlcCod->Next = PlcCodes;
          PlcCodes = NewPlcCod;
        }
      ContMc = ContMc->Next;
    }
    PlcCod = PlcCodes;
    while (PlcCod != nil)
    {
      McCod = McCodes;
      while (McCod->McNr != PlcCod->McNr)  McCod = McCod->Next;
      McFF = McCod->PlcCode;
      CopyCodes(McFF, PlcFF);
      PlcCod->PlcCode = PlcFF;
      PlcCod = PlcCod->Next;
    }
  }

  static void encodproc1(PCodes PlcCodes,
    PContMc ContMc);

  typedef struct TBit* PBit;
  struct TBit {
    unsigned char bit;
    PBit Next;
  };



  /*------------------------------------------------------------------------*/

  static void NewBit(PBit& BitList,
    integer NBit)
  {
    PBit NewB;

    NewB = new TBit;
    NewB->bit = NBit;
    NewB->Next = BitList;
    BitList = NewB;
  }



  /*------------------------------------------------------------------------*/

  static void DecBinar(integer LDec,
    PBit& BitList,
    byte& NrBit)
  {
    byte Rest;

    BitList = nil;
    NrBit = 0;
    while (LDec > 1)
    {
      Rest = LDec % 2;
      NewBit(BitList, Rest);
      NrBit = succ(byte, NrBit);
      LDec = LDec / 2;
    }
    NewBit(BitList, LDec);
    NrBit = succ(byte, NrBit);
  }



  /*------------------------------------------------------------------------*/

  static void ReWriteCod(PFlipFlop& FFList,
    PBit& BitList)
  {
    byte cyfra;

    while (BitList != nil)
    {
      while ((FFList != nil) && (FFList->Ch != '-'))  FFList = FFList->Next;
      cyfra = BitList->bit;
      if (cyfra == 0)  FFList->Ch = '0';
      else FFList->Ch = '1';
      BitList = BitList->Next;
    }
  }



  /*==========================================================================*/

  static void encodproc1(PCodes PlcCodes,
    PContMc ContMc)
  {
    byte McPlc, NrPlc, NrBit, I, zero;
    integer Diff;
    PBit BitList;
    PFlipFlop FFList;
    PContMc CntMc;

    /*------------------------------------------------------------------------*/


    zero = 0;
    NrPlc = 0;
    McPlc = PlcCodes->McNr;
    while (PlcCodes != nil)
    {
      if (PlcCodes->McNr != McPlc)
      {
        NrPlc = 0;
        McPlc = PlcCodes->McNr;
      }
      CntMc = CntMc;
      while ((CntMc->McNr != McPlc) && (CntMc != nil))  CntMc = CntMc->Next;
      if (CntMc->Price > 0)
      {
        DecBinar(NrPlc, BitList, NrBit);
        Diff = CntMc->Price - NrBit;
        if (Diff > 0)
        {
          if (Diff > 1)
            while (Diff > 0)
            {
              NewBit(BitList, zero);
              Diff = pred(integer, Diff);
            }
          else
            NewBit(BitList, zero);
        }
        FFList = PlcCodes->PlcCode;
        ReWriteCod(FFList, BitList);
      }
      NrPlc = succ(byte, NrPlc);
      PlcCodes = PlcCodes->Next;
    }
  }

  static void ReverseCodes(PCodes PlcCodes,
    ptnet_places_t Marking,
    char OldCod, char NewCod);


  /*------------------------------------------------------------------------*/

  static void FindActFF(PCodes W2,
    integer NrActQ,
    PFlipFlop& WActQ)
  {
    WActQ = W2->PlcCode;
    while (WActQ->FFNr != NrActQ)  WActQ = WActQ->Next;
  }



  /*------------------------------------------------------------------------*/

  static void ReversNext(PCodes W2,
    integer NrActQ,
    ptnet_places_t Marking,
    char OldCod, char NewCod)
  {
    PFlipFlop WActQ;

    while (W2 != nil)
    {
      FindActFF(W2, NrActQ, WActQ);
      if (WActQ->Ch == OldCod)  WActQ->Ch = NewCod;
      else
        if ((WActQ->Ch == NewCod) && (!(Marking.has(W2->PlcNr))))
          WActQ->Ch = OldCod;
      W2 = W2->Next;
    }
  }



  /*==========================================================================*/

  static void ReverseCodes(PCodes PlcCodes,
    ptnet_places_t Marking,
    char OldCod, char NewCod)
  {
    byte AktPlc;
    PCodes W1, W2;
    PFlipFlop PlcCod;

    /*------------------------------------------------------------------------*/


    for (AktPlc = 0; AktPlc <= 99; AktPlc++)
      if (Marking.has(AktPlc))
      {
        W1 = PlcCodes;
        while (W1->PlcNr != AktPlc)  W1 = W1->Next;
        PlcCod = W1->PlcCode;
        while (PlcCod != nil)
        {
          W2 = PlcCodes;
          if (PlcCod->Ch == OldCod)
          {
            PlcCod->Ch = NewCod;
            ReversNext(W2, PlcCod->FFNr, Marking, OldCod, NewCod);
          }
          PlcCod = PlcCod->Next;
        }
      }
  }

  static void RemoveFF(PCodes PlcCodes);
  static boolean SuperfluousFF(integer FFNr, PCodes PlcCodes);


  /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  static void findactff1(PCodes W2,
    integer NrActQ,
    PFlipFlop& WActQ)
  {
    WActQ = W2->PlcCode;
    while (WActQ->FFNr != NrActQ)  WActQ = WActQ->Next;
  }



  /*------------------------------------------------------------------------*/

  static boolean SuperfluousFF(integer FFNr, PCodes PlcCodes)
  {
    boolean Sprfls;
    PFlipFlop Act;

    /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


    boolean SuperfluousFF_result;
    Sprfls = true;
    while ((PlcCodes != nil) && (Sprfls == true))
    {
      findactff1(PlcCodes, FFNr, Act);
      if (set::of('0', '1', eos).has(Act->Ch))  Sprfls = false;
      PlcCodes = PlcCodes->Next;
    }
    SuperfluousFF_result = Sprfls;
    return SuperfluousFF_result;
  }



  /*------------------------------------------------------------------------*/

  static void RemoveSprfls(PCodes KMiejsca,
    integer ActNr)
  {
    PFlipFlop SecFF, Prv, Actl, Nxt;

    SecFF = KMiejsca->PlcCode;
    Prv = SecFF;
    Actl = Prv->Next;
    while (Actl->FFNr != ActNr)
    {
      Prv = Actl;
      Actl = Prv->Next;
    }
    if (Actl->Next == nil)
    {
      Prv->Next = nil;
      delete Actl;
    }
    else
    {
      Nxt = Actl->Next;
      while (Nxt != nil)
      {
        Actl->Ch = Nxt->Ch;
        if (Nxt->Next == nil)
        {
          Actl->Next = nil;
          delete Nxt;
        }
        else
          Actl = Nxt;
        Nxt = Actl->Next;
      }
    }
  }



  /*==========================================================================*/

  static void RemoveFF(PCodes PlcCodes)
  {
    PFlipFlop FlipFlops;
    PCodes KMiejsca;

    /*------------------------------------------------------------------------*/


    FlipFlops = PlcCodes->PlcCode;
    while (FlipFlops != nil)
    {
      if (SuperfluousFF(FlipFlops->FFNr, PlcCodes))
      {
        KMiejsca = PlcCodes;
        while (KMiejsca != nil)
        {
          RemoveSprfls(KMiejsca, FlipFlops->FFNr);
          KMiejsca = KMiejsca->Next;
        }
      }
      FlipFlops = FlipFlops->Next;
    }
  }



  /*==========================================================================*/

  static void PlacesCodes(PCodes PlcCodes,
    PListCodes& ListCodes)
  {
    PListCodes PlcCod, NewCod;
    PFlipFlop SecFF;
    char Ch;
    string Nmbr;

    ListCodes = nil;
    PlcCod = nil;
    while (PlcCodes != nil)
    {
      NewCod = new TListCodes;
      NewCod->PlcNr = PlcCodes->PlcNr;
      NewCod->StrCod = "";
      NewCod->Next = nil;
      SecFF = PlcCodes->PlcCode;
      Ch = ' ';
      while (SecFF != nil)
      {
        if (set::of('0', '1', eos).has(SecFF->Ch))
        {
          if (Ch == ' ')  NewCod->StrCod = NewCod->StrCod;
          else NewCod->StrCod = NewCod->StrCod + ' ' + Ch + ' ';
          Ch = '*';
          if (SecFF->Ch == '0')
            NewCod->StrCod = NewCod->StrCod + "/Q";
          else
            NewCod->StrCod = NewCod->StrCod + 'Q';
          str(SecFF->FFNr, Nmbr);
          NewCod->StrCod = NewCod->StrCod + Nmbr;
        }
        SecFF = SecFF->Next;
      }
      if (PlcCod != nil)  PlcCod->Next = NewCod;
      PlcCod = NewCod;
      if (ListCodes == nil)  ListCodes = NewCod;
      PlcCodes = PlcCodes->Next;
    }
    PlcCod = ListCodes;
  }



  /*==========================================================================*/

  static void WritePlcCodes(FNameStr& FileName,
    PListCodes ListCodes)

  {
    text Plik;
    string StrPlc;


    PTNetFileNameNew(FileName, "PLC");
    assign(Plik, FileName);
    rewrite(Plik);
    Plik << " Codes of places." << NL;
    Plik << "------------------" << NL;
    Plik << NL;
    while (ListCodes != nil)
    {
      str(ListCodes->PlcNr, StrPlc);
      StrPlc = Concat("   P", StrPlc);
      while (length(StrPlc) < 8)  insert(" ", StrPlc, length(StrPlc) + 1);
      Plik << StrPlc << "=  ";
      Plik << ListCodes->StrCod;
      Plik << NL;
      ListCodes = ListCodes->Next;
    }
    close(Plik);
  }

  void CodesOfPlaces(FNameStr& FileName,
    boolean WriteFile, boolean Rst,
    PConcurrency Concurrency,
    PPeak Peak,
    ptnet_places_t Marking, ptnet_places_t macronet_places,
    byte LastMacro,
    TContents contents,
    PCodes McCodes,
    PCodes& PlcCodes,
    PListCodes& ListCodes)
  {
    PPeak MaxClk;
    PContMc CntntsMc;

    /*==========================================================================*/


    ContentsOfMacro(contents, LastMacro, macronet_places, CntntsMc);
    McPrice(CntntsMc);
    MaxClkPrice(MaxClk, Peak, CntntsMc);
    AddFF(MaxClk, CntntsMc, McCodes, Concurrency);
    Block(Concurrency, MaxClk, CntntsMc, McCodes);
    ListOfPlcCodes(CntntsMc, McCodes, PlcCodes);
    encodproc1(PlcCodes, CntntsMc);
    if (Rst)  ReverseCodes(PlcCodes, Marking, '1', '0');
    else ReverseCodes(PlcCodes, Marking, '0', '1');
    RemoveFF(PlcCodes);
    PlacesCodes(PlcCodes, ListCodes);
    if (WriteFile)  WritePlcCodes(FileName, ListCodes);
  }

  /*�������������������� Wyswietlenie pelnej tablicy kodow ���������������������*/

  void FullTable(FNameStr& FileName,
    PCodes PlcCodes);


  /*==========================================================================*/

  static void SetOfPlaces(PCodes PlcCodes,
    ptnet_places_t& SetOfPlc)
  {
    SetOfPlc = set::of(eos);
    while (PlcCodes != nil)
    {
      SetOfPlc = SetOfPlc + set::of(PlcCodes->PlcNr, eos);
      PlcCodes = PlcCodes->Next;
    }
  }

  void FullTable(FNameStr& FileName,
    PCodes PlcCodes)
  {
    PFlipFlop FFList;
    text plik;
    PCodes W2;
    ptnet_places_t SetOfPlc;
    byte K;
    byte I;
    string StrPlc;

    /*==========================================================================*/


    PTNetFileNameNew(FileName, "FTB");
    assign(plik, FileName);
    rewrite(plik);
    plik << " Full table of codes." << NL;
    plik << "----------------------" << NL;
    plik << NL;
    plik << "    Q   : ";
    FFList = PlcCodes->PlcCode;
    while (FFList != nil)
    {
      plik << format(FFList->FFNr, 3);
      FFList = FFList->Next;
    }
    plik << NL;
    plik << NL;
    SetOfPlaces(PlcCodes, SetOfPlc);
    for (K = 0; K <= 99; K++)
      if (SetOfPlc.has(K))
      {
        W2 = PlcCodes;
        while ((W2->PlcNr != K) && (W2 != nil))  W2 = W2->Next;
        str(W2->PlcNr, StrPlc);
        StrPlc = Concat("   P", StrPlc);
        while (length(StrPlc) < 8)  insert(" ", StrPlc, length(StrPlc) + 1);
        plik << StrPlc << ": ";
        FFList = W2->PlcCode;
        while (FFList != nil)
        {
          plik << format(FFList->Ch, 3);
          FFList = FFList->Next;
        }
        plik << NL;
      }
    close(plik);
  }

  /*����������������������������������������������������������������������������*/

  void WriteSeq(FNameStr& FileName,
    ptnet_transitions_list_t Net,
    ptnet_registers_list_t Reg,
    ptnet_input_data_t input_data,
    ptnet_places_t Marking,
    PListCodes ListCodes);

  static text Plik;


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
    ptnet_places_t Marking,
    text& Plik)
  {
    integer I;
    boolean Start;

    WrtInData(input_data.Plc, "PLACE ", 'P');
    WrtInData(input_data.inp_, "INPUT ", 'X');
    WrtInData(input_data.Reg, "REG_OUT ", 'Y');
    WrtInData(input_data.Com, "COMB_OUT ", 'Y');
    WrtInData(Marking, "MARKING ", 'P');
    Plik << NL;
  }



  /*==========================================================================*/

  static void WriteTrn(PListCodes ListCodes,
    ptnet_places_t Plc, ptnet_places_t Sgnl, ptnet_places_t Sgnl1,
    boolean Left)
  {
    integer I;
    boolean Start;
    PListCodes SecLC;
    string SeqLine;
    string SL;

    Start = true;
    I = 0;
    SeqLine = "";
    while (Plc != set::of(eos))
    {
      if (Plc.has(I))
      {
        SecLC = ListCodes;
        while ((SecLC->PlcNr != I) && (SecLC != nil))
          SecLC = SecLC->Next;
        if (Start)
        {
          insert(SecLC->StrCod, SeqLine, length(SeqLine) + 1);
          Start = false;
        }
        else
        {
          SL = concat(" * ", SecLC->StrCod);
          insert(SL, SeqLine, length(SeqLine) + 1);
        }
        Plc = Plc - set::of(I, eos);
      }
      I += 1;
    }
    Start = true;
    while (SeqLine != "")
    {
      while (pos(" ", SeqLine) != 0)  Delete(SeqLine, pos(" ", SeqLine), 1);
      if (pos("*", SeqLine) != 0)
      {
        SL = copy(SeqLine, 1, pos("*", SeqLine) - 1);
        Delete(SeqLine, 1, pos("*", SeqLine));
        while (pos(SL, SeqLine) != 0)
          if (((pos("/", SL) == 0) && (SeqLine[pos(SL, SeqLine) - 1] != '/')) ||
            ((pos("/", SL) != 0) && (SeqLine[pos(SL, SeqLine)] == '/')))
          {
            if (SeqLine[pos(SL, SeqLine) + length(SL)] == '*')
              Delete(SeqLine, pos(SL, SeqLine) + length(SL), 1);
            Delete(SeqLine, pos(SL, SeqLine), length(SL));
          }
      }
      else
      {
        if (SeqLine != "") SL = SeqLine;
        Delete(SeqLine, pos(SL, SeqLine), length(SL));
      }
      if (Start)  Plik << SL;
      else Plik << " * " << SL;
      Start = false;
    }
    I = 0;
    while (Sgnl != set::of(eos))
    {
      while (!(Sgnl.has(I)))  I += 1;
      if (Left)
        if (Sgnl1.has(I))  Plik << " * /X" << I;
        else Plik << " * X" << I;
      else
        if (Sgnl1.has(I))  Plik << " * RY" << I;
        else Plik << " * SY" << I;
      Sgnl = Sgnl - set::of(I, eos);
    }
  }

  void WriteSeq(FNameStr& FileName,
    ptnet_transitions_list_t Net,
    ptnet_registers_list_t Reg,
    ptnet_input_data_t input_data,
    ptnet_places_t Marking,
    PListCodes ListCodes)
  {
    boolean Start;
    PListCodes SecLC;
    ptnet_transitions_list_t Trn;
    byte I;
    ptnet_places_t P;

    /*==========================================================================*/


    PTNetFileNameNew(FileName, "COD");
    assign(Plik, FileName);
    rewrite(Plik);
    Plik << " Encoded sequents." << NL;
    Plik << "-------------------" << NL;
    Plik << NL;
    GetInput(input_data, Marking, Plik);
    Plik << "TRANSITION" << NL;
    Trn = Net;
    while (Trn != nil)
    {
      Plik << 'T' << Trn->Nr << ": ";
      WriteTrn(ListCodes, Trn->InpPlc, Trn->inp_, Trn->InpN, true);
      Plik << " |- ";
      WriteTrn(ListCodes, Trn->OutPlc, Trn->Out, Trn->OutR, false);
      Plik << ';' << NL;
      Trn = Trn->NextTrn;
    }
    Plik << NL;
    Start = true;
    Plik << "OUTPUTS" << NL;
    for (I = 1; I <= Reg[0].Plc; I++)
    {
      SecLC = ListCodes;
      while ((SecLC->PlcNr != Reg[I].Plc) && (SecLC != nil))
        SecLC = SecLC->Next;
      Plik << SecLC->StrCod << " |- Y" << Reg[I].Out << ';';
      Plik << NL;
    }
    close(Plik);
  }

  /*����������������������������������������������������������������������������*/

}
