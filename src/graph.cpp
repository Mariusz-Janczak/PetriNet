/******************************************************************************/
/*                     Department of Computer Engineering                     */
/*                Higher College of Engineering in Zielona Gora               */
/*                                                                            */
/*                Decomposition and Place Encoding of Petri Net.              */
/*                 Master's thesis, Supervisor Prof. M.Adamski.               */
/*                                                                            */
/*                                            Author: Mariusz Janczak (1993)  */
/******************************************************************************/
/*  Module for creating a network and macronetworks marking graphs.           */
/******************************************************************************/

#include "graph.h"
#include "tools.h"

namespace graph {
#if 0
  static void WritePlc(ptnet_places_t Prtn, TCiag& Str1, byte& Position)
  {
    varying_string <10> Str2;
    integer I;

    I = 0;
    while (Prtn != set::of(eos))
    {
      do {
        I += 1;
      } while (!(Prtn.has(I)));
      str(I, Str2);
      Delete(Str1, Position, length(Str2));
      insert(Str2, Str1, Position);
      Position = Position + length(Str2);
      Prtn = Prtn - set::of(I, eos);
      if (Prtn != set::of(eos))
      {
        Delete(Str1, Position, 1);
        insert(",", Str1, Position);
        Position += 1;
      }
    }
  }

  void GetGraf(ptnet_branches_list_t branches, text& Plik, integer What)
  {
    TCiag Str1, Clean;
    varying_string<6> NrTrn;
    byte Position;
    PTree Trees;
    boolean Consent;

    Consent = true;
    if (branches != nil)
    {
      if (What == 1)
      {
        Plik << " Reachability graph." << NL;
        Plik << "---------------------" << NL;
      }
      else
      {
        Plik << " Macronet reachability graph." << NL;
        Plik << "------------------------------" << NL;
      }
      Plik << NL;
      Plik << "����������������������������������������������������������������������������͸" << NL;
      Plik << "�        MARKED PLACES         � TRANSITIONS �       NEW MARKED PLACES       �" << NL;
      Plik << "����������������������������������������������������������������������������͵" << NL;
    }
    else
      Plik << "Could not create graph." << NL;
    Clean = "�                              �             �                               �";
    while ((branches != nil) && Consent)
    {
      Str1 = Clean;
      Position = 3;
      WritePlc(branches->Addr->MrkPlc, Str1, Position);
      if (Position < 39)  Position = 39;
      else Position = Position + 1;
      Trees = branches->Addr->RdyTrn;
      while ((Trees != nil) && Consent)
      {
        str(Trees->TranNr->Nr, NrTrn);
        Delete(Str1, Position, length(NrTrn));
        insert(NrTrn, Str1, Position);
        Position = 48;
        WritePlc(Trees->Branch->MrkPlc, Str1, Position);
        Position = 39;
        Trees = Trees->NextTree;
        Plik << Str1 << NL;
        Str1 = Clean;
      }
      branches = branches->Next;
      if (branches == nil)
        Plik << "����������������������������������������������������������������������������;" << NL;
      else
        Plik << "����������������������������������������������������������������������������Ĵ" << NL;
    }
  }
#endif
  void PTNetWriteGraph(PetriNet& petrinet, bool macro)
  {
    std::string out_name = PTNetFileNameNew(petrinet.file_name, macro ? "mcg" : "grf");
    std::ofstream file(out_name);

    std::cout << "Graph file name: " << out_name << endl;
    if (file.is_open())
    {
      //GetGraf(branches, Plik, What);
      file.close();
    }
  }
#if 0
  static void ReadyTrn(PetriNet& petrinet, PBranch& TestingBranch)
  {
    PTree PNew;

    for (ptnet_transitions_list_t::iterator it = petrinet.net.begin(); it != petrinet.net.end(); it++)
    {
      if ((it->InpPlc) <= (TestingBranch->MrkPlc))
      {
        PNew = new ptnet_tree_t;
        PNew->NextTree = TestingBranch->RdyTrn;
        PNew->TranNr = PTrnNr;
        TestingBranch->RdyTrn = PNew;
      }
    }
  }

  static void Light(ptnet_transitions_list_t Transition, ptnet_places_t OldMarkPlc, ptnet_places_t& NewMarkPlc, boolean& Danger, boolean& dead)
  {
    integer I;
    ptnet_places_t OutPlcs;
    char Ch;

    NewMarkPlc = OldMarkPlc - Transition->InpPlc;
    I = 1;
    OutPlcs = Transition->OutPlc;
    while ((I < 256) && (OutPlcs != set::of(eos)))
    {
      if (OutPlcs.has(I))
      {
        if (NewMarkPlc.has(I))
        {
          Danger = true;
          dead = true;
        }
        else
        {
          Danger = false;
          NewMarkPlc = NewMarkPlc + set::of(I, eos);
          OutPlcs = OutPlcs - set::of(I, eos);
        }
      }
      I += 1;
    }
  }

  static void FindBranch(ptnet_branches_list_t branches, ptnet_places_t NewMarkPlc, PBranch& Like)
  {
    Like = nil;
    while (branches != nil)
    {
      if (branches->Addr->MrkPlc == NewMarkPlc)  Like = branches->Addr;
      branches = branches->Next;
    }
  }

  static void MarkGraf(PetriNet& petrinet, bool& Start, bool& dead)
  {
    PTree Transition;
    ptnet_places_t OldMarkPlc, NewMarkPlc;
    PBranch Like, PNew;
    ptnet_branches_list_t NewAddress;
    boolean Danger;
    char Ch;

    if (Start)
    {
      ReadyTrn(petrinet, Prev->Addr);
      if (Prev->Addr->RdyTrn == nil)
      {
        Start = false;
        dead = true;
      }
      else
      {
        Transition = Prev->Addr->RdyTrn;
        OldMarkPlc = Prev->Addr->MrkPlc;
        while ((Transition != nil) && Start)
        {
          Light(Transition->TranNr, OldMarkPlc, NewMarkPlc, Danger, dead);
          if (Danger)  Start = false;
          FindBranch(branches, NewMarkPlc, Like);
          if (Like != nil)  Transition->Branch = Like;
          else
          {
            PNew = new ptnet_branch_t;
            PNew->MrkPlc = NewMarkPlc;
            PNew->RdyTrn = nil;
            Transition->Branch = PNew;
            NewAddress = new TAddress;
            NewAddress->Next = nil;
            NewAddress->Addr = PNew;
            Prev->Next = NewAddress;
            Prev = NewAddress;
            MarkGraf(petrinet, Prev, branches, Start, dead);
          }
          Transition = Transition->NextTree;
        }
      }
    }
  }

  static void GraphPeak(ptnet_branches_list_t branches, PPeak& Peak)
  {
    byte I;
    PPeak NewPeak;

    Peak = nil;
    while (branches != nil)
    {
      NewPeak = new TPeak;
      NewPeak->PeakCont = set::of(eos);
      NewPeak->Next = Peak;
      Peak = NewPeak;
      for (I = 0; I <= 199; I++)
        if (branches->Addr->MrkPlc.has(I))
          NewPeak->PeakCont = NewPeak->PeakCont + set::of(I, eos);
      branches = branches->Next;
    }
  }
#endif
  void PTNetCreateGraph(PetriNet& petrinet, bool macro, PPeak& /*Peak*/, bool& dead)
  {
    ptnet_branch_t branch;
    branch.MrkPlc = petrinet.net_marking;
    petrinet.branches.push_back(branch);

    dead = false;

    //bool Start = !dead;

    //Prev = branches;
    //MarkGraf(petrinet, Start, dead);
    //GraphPeak(petrinet.branches, Peak);
    if (petrinet.save_intermediate_results)
    {
      PTNetWriteGraph(petrinet, macro);
    }
  }

#if 0
  void GetMacro(ptnet_transitions_list_t macronet, ptnet_transitions_list_t transfers, TContents contents, integer LastMacro, text& Plik)
  {
    ptnet_transitions_list_t DMcNet, DTransfer;
    byte I, J, N;
    boolean Start;
    char Ch;

    N = 0;
    Plik << " Macronet structure." << NL;
    Plik << "---------------------" << NL;
    Plik << NL;
    DMcNet = macronet;
    Plik << "TRANSITIONS" << NL;
    while (DMcNet != nil)
    {
      Plik << 'T' << DMcNet->Nr << ": ";
      Start = true;
      for (I = 0; I <= 199; I++)
        if (DMcNet->InpPlc.has(I))
        {
          if (!Start)  Plik << " * ";
          else Start = false;
          if (I > 100)  Plik << 'M';
          Plik << 'P' << I;
        }
      Plik << " |- ";
      Start = true;
      for (I = 0; I <= 199; I++)
        if (DMcNet->OutPlc.has(I))
        {
          if (!Start)  Plik << " * ";
          else Start = false;
          if (I > 100)  Plik << "@M";
          if (I <= 100)  Plik << '@';
          Plik << 'P' << I;
        }
      Plik << ';' << NL;
      DMcNet = DMcNet->NextTrn;
    }
    Plik << NL;
    for (I = 101; I <= LastMacro; I++)
    {
      Plik << "MACROPLACE " << I << NL;
      Plik << "    Internal places      : ";
      Start = true;
      for (J = 0; J <= 99; J++)
        if (contents[I].has(J))
          if (!Start)  Plik << ",P" << J;
          else
          {
            Plik << 'P' << J;
            Start = false;
          }
      Plik << NL;
      Plik << "    Internal transitions : ";
      Start = true;
      DTransfer = transfers;
      while (DTransfer != nil)
      {
        for (J = 0; J <= 100; J++)
          if ((DTransfer->InpPlc.has(J)) && (contents[I].has(J)))
            if (!Start)  Plik << ",T" << DTransfer->Nr;
            else
            {
              Plik << 'T' << DTransfer->Nr;
              Start = false;
            }
        DTransfer = DTransfer->NextTrn;
      }
      Plik << NL;
      Plik << "    Border transitions   : ";
      Start = true;
      DMcNet = macronet;
      while (DMcNet != nil)
      {
        if ((DMcNet->InpPlc.has(I)) || (DMcNet->OutPlc.has(I)))
          if (!Start)  Plik << ",T" << DMcNet->Nr;
          else
          {
            Plik << 'T' << DMcNet->Nr;
            Start = false;
          }
        DMcNet = DMcNet->NextTrn;
      }
      Plik << NL;
      Plik << NL;
    }
  }
#endif
  void PTNetWriteMacronet(PetriNet& petrinet, int /*LastMacro*/)
  {
    std::string out_name = PTNetFileNameNew(petrinet.file_name, "mcn");
    std::ofstream file(out_name);

    std::cout << "Macro file name: " << out_name << endl;
    if (file.is_open())
    {
      //GetMacro(macronet, transfers, contents, LastMacro, Plik);
      file.close();
    }
  }

  static bool IsTransfer(ptnet_transition_t& trans)
  {
    std::cout << "Checking transition nr=" << trans.Nr << ", inputs=" << trans.InpPlc.size() << ", outputs=" << (trans.OutPlc.size() == 1) << endl;
    return ((trans.InpPlc.size() == 1) && (trans.OutPlc.size() == 1));
  }
#if 0
  static void AddMcPlc(ptnet_transitions_list_t macronet, ptnet_transitions_list_t transfers, ptnet_places_t& macronet_marking, integer& LastMacro, TContents& contents, boolean& dead, boolean& Danger);

  static void Replace(ptnet_transitions_list_t macronet, byte Repl, byte Subs, boolean& dead, boolean& Danger)
  {
    char Ch;

    if (Repl != Subs)
      while (macronet != nil)
      {
        if (macronet->InpPlc.has(Repl))
          if ((!(macronet->InpPlc.has(Subs))) ||
            (macronet->InpPlc == macronet->OutPlc))
            macronet->InpPlc = macronet->InpPlc - set::of(Repl, eos) + set::of(Subs, eos);
          else
          {
            dead = true;
            Danger = true;
          }
        if (macronet->OutPlc.has(Repl))
          if ((!(macronet->OutPlc.has(Subs))) ||
            (macronet->InpPlc == macronet->OutPlc))
            macronet->OutPlc = macronet->OutPlc - set::of(Repl, eos) + set::of(Subs, eos);
          else
          {
            dead = true;
            Danger = true;
          }
        macronet = macronet->NextTrn;
      }
  }

  static void AddMcPlc(ptnet_transitions_list_t macronet, ptnet_transitions_list_t transfers, ptnet_places_t& macronet_marking, integer& LastMacro, TContents& contents, boolean& dead, boolean& Danger)
  {
    integer I, inp_, Out, Leav, Cln;
    ptnet_transitions_list_t DTrn;

    for (I = 1; I <= 255; I++)
    {
      if (transfers->InpPlc.has(I))  inp_ = I;
      if (transfers->OutPlc.has(I))  Out = I;
    }
    if ((inp_ < 100) && (Out < 100))
    {
      LastMacro += 1;
      contents[LastMacro] = set::of(inp_, eos) + set::of(Out, eos);
      if ((macronet_marking.has(inp_)) || (macronet_marking.has(Out)))
        macronet_marking = macronet_marking - set::of(inp_, Out, eos) + set::of(LastMacro, eos);
      Replace(macronet, inp_, LastMacro, dead, Danger);
      Replace(macronet, Out, LastMacro, dead, Danger);
    }
    else
      if (inp_ != Out)
      {
        if ((inp_ > 100) && (Out > 100))
        {
          if (inp_ > Out)
          {
            Leav = Out;
            Cln = inp_;
          }
          else
          {
            Leav = inp_;
            Cln = Out;
          }
          Replace(macronet, Cln, Leav, dead, Danger);
          contents[Leav] = contents[Leav] + contents[Cln];
          if (macronet_marking.has(Cln))
            macronet_marking = macronet_marking - set::of(Cln, eos) + set::of(Leav, eos);
          for (I = Cln + 1; I <= LastMacro; I++)
            Replace(macronet, I, I - 1, dead, Danger);
          for (I = Cln + 1; I <= LastMacro; I++)
            contents[I - 1] = contents[I];
          LastMacro -= 1;
          contents[I] = set::of(eos);
        }
        else
        {
          if (inp_ > Out)
          {
            Leav = inp_;
            Cln = Out;
          }
          else
          {
            Leav = Out;
            Cln = inp_;
          }
          contents[Leav] = contents[Leav] + set::of(Cln, eos);
          if (macronet_marking.has(Cln))
            macronet_marking = macronet_marking - set::of(Cln, eos) + set::of(Leav, eos);
          Replace(macronet, Cln, Leav, dead, Danger);
        }
      }
  }

  static void AddTrn(ptnet_transitions_list_t Trns, ptnet_transitions_list_t Net, ptnet_transitions_list_t& transfers)
  {
    while ((Net->Nr != Trns->Nr) && (Net != nil))
      Net = Net->NextTrn;
    Trns->InpPlc = Net->InpPlc;
    Trns->OutPlc = Net->OutPlc;
    Trns->NextTrn = transfers;
    transfers = Trns;
  }

  static void PlcOfMacro(ptnet_transitions_list_t Trns, ptnet_places_t& macronet_places)
  {
    macronet_places = set::of(eos);
    while (Trns != nil)
    {
      macronet_places = macronet_places + Trns->InpPlc;
      Trns = Trns->NextTrn;
    }
  }
#endif

  void PTNetCreateMacronet(PetriNet& petrinet, int& LastMacro, bool& dead, bool& /*Danger*/)
  {
    /*ptnet_transitions_list_t ClrTrn, Prev, Actual;*/
    /*integer I;*/

    dead = false;

    /* Make a copy of net places ... */
    for (ptnet_transitions_list_t::iterator it = petrinet.net.begin(); it != petrinet.net.end(); it++)
    {
      ptnet_transition_t trans;
      trans.Nr = it->Nr;
      trans.InpPlc = it->InpPlc;
      trans.OutPlc = it->OutPlc;
      petrinet.macronet.push_back(trans);
    }

    /* ... and a copy of net markings */
    petrinet.macronet_marking = petrinet.net_marking;

    //for (I = 101; I <= 199; I++) contents[I] = set::of(eos);
    //LastMacro = 100;
    for (ptnet_transitions_list_t::iterator it = petrinet.macronet.begin(); it != petrinet.macronet.end(); it++)
    {
      if (IsTransfer(*it))
      {
        std::cout << "Transition has transfer" << endl;
#if 0
        do {
          AddMcPlc(macronet->NextTrn, macronet, macronet_marking, LastMacro, contents, dead, Danger);
          ClrTrn = macronet;
          macronet = macronet->NextTrn;
          AddTrn(ClrTrn, Net, transfers);
        } while (!((!IsTransfer(macronet)) || (macronet->NextTrn == nil) || dead));
#endif
      }
    }
#if 0
    Prev = macronet;
    Actual = macronet->NextTrn;
    while ((Actual != nil) && (!dead))
    {
      if (IsTransfer(Actual))
      {
        Prev->NextTrn = Actual->NextTrn;
        AddMcPlc(macronet, Actual, macronet_marking, LastMacro, contents, dead, Danger);
        AddTrn(Actual, Net, transfers);
        Actual = Prev->NextTrn;
      }
      else
      {
        Prev = Actual;
        Actual = Actual->NextTrn;
      }
    }
    PlcOfMacro(macronet, macronet_places);
#endif
    if (petrinet.save_intermediate_results)
    {
      PTNetWriteMacronet(petrinet, LastMacro);
    }
  }
}
