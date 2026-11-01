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

#include <algorithm>
#include "graph.h"
#include "tools.h"

namespace graph {
  /* Overwrite text at a 1-based position, like Pascal delete() followed by insert(). */
  static void WriteAt(std::string& line, size_t position, const std::string& text)
  {
    line.replace(std::min(position - 1, line.length()), text.length(), text);
  }

  static void WritePlc(const ptnet_places_t& places, std::string& line, size_t& position)
  {
    size_t left = places.size();

    for (int place : places)
    {
      std::string number = std::to_string(place);

      WriteAt(line, position, number);
      position += number.length();
      if (--left > 0)
      {
        WriteAt(line, position, ",");
        position += 1;
      }
    }
  }

  /* Horizontal table line with columns of the same width as in the original output. */
  static std::string Rule(const char* left, const char* fill, const char* cross, const char* right)
  {
    const size_t widths[] = { 30, 13, 31 };
    std::string rule(left);

    for (size_t i = 0; i < items(widths); i++)
    {
      for (size_t j = 0; j < widths[i]; j++)
      {
        rule += fill;
      }
      rule += (i + 1 < items(widths)) ? cross : right;
    }
    return rule;
  }

  /* Table rows are built with '|' as a placeholder, as box drawing characters take more than one byte. */
  static void WriteRow(std::ofstream& file, const std::string& line)
  {
    for (char ch : line)
    {
      if (ch == '|')
      {
        file << "│";
      }
      else
      {
        file << ch;
      }
    }
    file << endl;
  }

  static void GetGraf(std::ofstream& file, const ptnet_branches_list_t& branches, const ptnet_transitions_list_t& transitions, bool macro)
  {
    const std::string clean = "|                              |             |                               |";

    if (!branches.empty())
    {
      if (!macro)
      {
        file << " Reachability graph." << endl;
        file << "---------------------" << endl;
      }
      else
      {
        file << " Macronet reachability graph." << endl;
        file << "------------------------------" << endl;
      }
      file << endl;
      file << Rule("╒", "═", "╤", "╕") << endl;
      WriteRow(file, "|        MARKED PLACES         | TRANSITIONS |       NEW MARKED PLACES       |");
      file << Rule("╞", "═", "╪", "╡") << endl;
    }
    else
    {
      file << "Could not create graph." << endl;
    }
    for (size_t b = 0; b < branches.size(); b++)
    {
      std::string line = clean;
      size_t position = 3;

      WritePlc(branches[b].MrkPlc, line, position);
      position = (position < 39) ? 39 : position + 1;
      for (const ptnet_arc_t& arc : branches[b].RdyTrn)
      {
        WriteAt(line, position, std::to_string(transitions[arc.transition].Nr));
        position = 48;
        WritePlc(branches[arc.branch].MrkPlc, line, position);
        position = 39;
        WriteRow(file, line);
        line = clean;
      }
      if (b + 1 == branches.size())
      {
        file << Rule("╘", "═", "╧", "╛") << endl;
      }
      else
      {
        file << Rule("├", "─", "┼", "┤") << endl;
      }
    }
  }

  void PTNetWriteGraph(PetriNet& petrinet, bool macro)
  {
    std::string out_name = PTNetFileNameNew(petrinet.file_name, macro ? "mcg" : "grf");
    std::ofstream file(out_name);

    std::cout << "Graph file name: " << out_name << endl;
    if (file.is_open())
    {
      GetGraf(file, macro ? petrinet.macronet_branches : petrinet.branches, macro ? petrinet.macronet : petrinet.net, macro);
      file.close();
    }
  }

  static void ReadyTrn(const ptnet_transitions_list_t& transitions, ptnet_branch_t& branch)
  {
    /* The original prepends ready transitions to the list, so they are collected in reverse order. */
    for (size_t t = transitions.size(); t-- > 0; )
    {
      if (transitions[t].InpPlc <= branch.MrkPlc)
      {
        branch.RdyTrn.push_back({ t, 0 });
      }
    }
  }

  static void Light(const ptnet_transition_t& transition, const ptnet_places_t& OldMarkPlc, ptnet_places_t& NewMarkPlc, bool& Danger, bool& dead)
  {
    NewMarkPlc = OldMarkPlc - transition.InpPlc;
    Danger = false;
    for (int place : transition.OutPlc)
    {
      if (NewMarkPlc.has(place))
      {
        Danger = true;
        dead = true;
      }
      else
      {
        /* As in the original, a following output place clears Danger again. */
        Danger = false;
        NewMarkPlc.insert(place);
      }
    }
  }

  static bool FindBranch(const ptnet_branches_list_t& branches, const ptnet_places_t& NewMarkPlc, size_t& like)
  {
    for (size_t b = 0; b < branches.size(); b++)
    {
      if (branches[b].MrkPlc == NewMarkPlc)
      {
        like = b;
        return true;
      }
    }
    return false;
  }

  /*
   * Depth-first construction of the graph from the node "current". The
   * branches vector grows during recursion, so nodes are accessed by index only.
   */
  static void MarkGraf(const ptnet_transitions_list_t& transitions, ptnet_branches_list_t& branches, size_t current, bool& Start, bool& dead)
  {
    if (Start)
    {
      ReadyTrn(transitions, branches[current]);
      if (branches[current].RdyTrn.empty())
      {
        Start = false;
        dead = true;
      }
      else
      {
        const ptnet_places_t OldMarkPlc = branches[current].MrkPlc;
        size_t a = 0;

        for (; (a < branches[current].RdyTrn.size()) && Start; a++)
        {
          ptnet_places_t NewMarkPlc;
          bool Danger = false;
          size_t like = 0;

          Light(transitions[branches[current].RdyTrn[a].transition], OldMarkPlc, NewMarkPlc, Danger, dead);
          if (Danger)  Start = false;
          if (FindBranch(branches, NewMarkPlc, like))
          {
            branches[current].RdyTrn[a].branch = like;
          }
          else
          {
            ptnet_branch_t branch;

            branch.MrkPlc = NewMarkPlc;
            branches.push_back(branch);
            branches[current].RdyTrn[a].branch = branches.size() - 1;
            MarkGraf(transitions, branches, branches.size() - 1, Start, dead);
          }
        }
        /* Drop arcs not processed after the construction was stopped (their targets are unknown). */
        branches[current].RdyTrn.resize(a);
      }
    }
  }
#if 0
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
    const ptnet_transitions_list_t& transitions = macro ? petrinet.macronet : petrinet.net;
    ptnet_branches_list_t& branches = macro ? petrinet.macronet_branches : petrinet.branches;
    ptnet_branch_t root;
    bool Start = true;

    root.MrkPlc = macro ? petrinet.macronet_marking : petrinet.net_marking;
    branches.clear();
    branches.push_back(root);
    dead = false;

    MarkGraf(transitions, branches, 0, Start, dead);
    //GraphPeak(branches, Peak);
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
