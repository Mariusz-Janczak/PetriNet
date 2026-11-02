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
  static void ptnetOverwriteText(std::string& line, size_t position, const std::string& text)
  {
    line.replace(std::min(position - 1, line.length()), text.length(), text);
  }

  static void ptnetWritePlaces(const ptnet_places_t& places, std::string& line, size_t& position)
  {
    size_t left = places.size();

    for (int place : places)
    {
      std::string number = std::to_string(place);

      ptnetOverwriteText(line, position, number);
      position += number.length();
      if (--left > 0)
      {
        ptnetOverwriteText(line, position, ",");
        position += 1;
      }
    }
  }

  /* Horizontal table line with columns of the same width as in the original output. */
  static std::string ptnetTableRule(const char* left, const char* fill, const char* cross, const char* right)
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
  static void ptnetWriteTableRow(std::ofstream& file, const std::string& line)
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

  static void ptnetWriteGraphTable(std::ofstream& file, const ptnet_branches_list_t& branches, const ptnet_transitions_list_t& transitions, bool macro)
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
      file << ptnetTableRule("╒", "═", "╤", "╕") << endl;
      ptnetWriteTableRow(file, "|        MARKED PLACES         | TRANSITIONS |       NEW MARKED PLACES       |");
      file << ptnetTableRule("╞", "═", "╪", "╡") << endl;
    }
    else
    {
      file << "Could not create graph." << endl;
    }
    for (size_t b = 0; b < branches.size(); b++)
    {
      std::string line = clean;
      size_t position = 3;

      ptnetWritePlaces(branches[b].marking, line, position);
      position = (position < 39) ? 39 : position + 1;
      for (const ptnet_arc_t& arc : branches[b].arcs)
      {
        ptnetOverwriteText(line, position, std::to_string(transitions[arc.transition].number));
        position = 48;
        ptnetWritePlaces(branches[arc.branch].marking, line, position);
        position = 39;
        ptnetWriteTableRow(file, line);
        line = clean;
      }
      if (b + 1 == branches.size())
      {
        file << ptnetTableRule("╘", "═", "╧", "╛") << endl;
      }
      else
      {
        file << ptnetTableRule("├", "─", "┼", "┤") << endl;
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
      ptnetWriteGraphTable(file, macro ? petrinet.macronet_branches : petrinet.branches, macro ? petrinet.macronet : petrinet.net, macro);
      file.close();
    }
  }

  static void ptnetFindEnabledTransitions(const ptnet_transitions_list_t& transitions, ptnet_branch_t& branch)
  {
    /* The original prepends enabled transitions to the list, so they are collected in reverse order. */
    for (size_t t = transitions.size(); t-- > 0; )
    {
      if (transitions[t].input_places <= branch.marking)
      {
        branch.arcs.push_back({ t, 0 });
      }
    }
  }

  static void ptnetFireTransition(const ptnet_transition_t& transition, const ptnet_places_t& old_marking, ptnet_places_t& new_marking, bool& unsafe, bool& dead)
  {
    new_marking = old_marking - transition.input_places;
    unsafe = false;
    for (int place : transition.output_places)
    {
      if (new_marking.has(place))
      {
        unsafe = true;
        dead = true;
      }
      else
      {
        /* As in the original, a following output place clears the unsafe flag again. */
        unsafe = false;
        new_marking.insert(place);
      }
    }
  }

  static bool ptnetFindBranch(const ptnet_branches_list_t& branches, const ptnet_places_t& marking, size_t& found)
  {
    for (size_t b = 0; b < branches.size(); b++)
    {
      if (branches[b].marking == marking)
      {
        found = b;
        return true;
      }
    }
    return false;
  }

  /*
   * Depth-first construction of the graph from the node "current". The
   * branches vector grows during recursion, so nodes are accessed by index only.
   */
  static void ptnetBuildGraph(const ptnet_transitions_list_t& transitions, ptnet_branches_list_t& branches, size_t current, bool& proceed, bool& dead)
  {
    if (proceed)
    {
      ptnetFindEnabledTransitions(transitions, branches[current]);
      if (branches[current].arcs.empty())
      {
        proceed = false;
        dead = true;
      }
      else
      {
        const ptnet_places_t old_marking = branches[current].marking;
        size_t a = 0;

        for (; (a < branches[current].arcs.size()) && proceed; a++)
        {
          ptnet_places_t new_marking;
          bool unsafe = false;
          size_t found = 0;

          ptnetFireTransition(transitions[branches[current].arcs[a].transition], old_marking, new_marking, unsafe, dead);
          if (unsafe)  proceed = false;
          if (ptnetFindBranch(branches, new_marking, found))
          {
            branches[current].arcs[a].branch = found;
          }
          else
          {
            ptnet_branch_t branch;

            branch.marking = new_marking;
            branches.push_back(branch);
            branches[current].arcs[a].branch = branches.size() - 1;
            ptnetBuildGraph(transitions, branches, branches.size() - 1, proceed, dead);
          }
        }
        /* Drop arcs not processed after the construction was stopped (their targets are unknown). */
        branches[current].arcs.resize(a);
      }
    }
  }

  /* Markings of all graph nodes, in reverse order of creation as the original prepends them. */
  static void ptnetCollectMarkings(const ptnet_branches_list_t& branches, ptnet_markings_list_t& markings)
  {
    markings.clear();
    for (auto it = branches.rbegin(); it != branches.rend(); it++)
    {
      markings.push_back(it->marking);
    }
  }

  void PTNetCreateGraph(PetriNet& petrinet, bool macro, bool& dead)
  {
    const ptnet_transitions_list_t& transitions = macro ? petrinet.macronet : petrinet.net;
    ptnet_branches_list_t& branches = macro ? petrinet.macronet_branches : petrinet.branches;
    ptnet_branch_t root;
    bool proceed = true;

    root.marking = macro ? petrinet.macronet_marking : petrinet.net_marking;
    branches.clear();
    branches.push_back(root);
    dead = false;

    ptnetBuildGraph(transitions, branches, 0, proceed, dead);
    ptnetCollectMarkings(branches, petrinet.reachable_markings);
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
      Plik << 'T' << DMcNet->number << ": ";
      Start = true;
      for (I = 0; I <= ptnet_macroplace_last; I++)
        if (DMcNet->input_places.has(I))
        {
          if (!Start)  Plik << " * ";
          else Start = false;
          if (I > ptnet_places_max)  Plik << 'M';
          Plik << 'P' << I;
        }
      Plik << " |- ";
      Start = true;
      for (I = 0; I <= ptnet_macroplace_last; I++)
        if (DMcNet->output_places.has(I))
        {
          if (!Start)  Plik << " * ";
          else Start = false;
          if (I > ptnet_places_max)  Plik << "@M";
          if (I <= ptnet_places_max)  Plik << '@';
          Plik << 'P' << I;
        }
      Plik << ';' << NL;
      DMcNet = DMcNet->NextTrn;
    }
    Plik << NL;
    for (I = ptnet_macroplace_first; I <= LastMacro; I++)
    {
      Plik << "MACROPLACE " << I << NL;
      Plik << "    Internal places      : ";
      Start = true;
      /* As in the original, place 100 is not listed. */
      for (J = 0; J < ptnet_places_max; J++)
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
        for (J = 0; J <= ptnet_places_max; J++)
          if ((DTransfer->input_places.has(J)) && (contents[I].has(J)))
            if (!Start)  Plik << ",T" << DTransfer->number;
            else
            {
              Plik << 'T' << DTransfer->number;
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
        if ((DMcNet->input_places.has(I)) || (DMcNet->output_places.has(I)))
          if (!Start)  Plik << ",T" << DMcNet->number;
          else
          {
            Plik << 'T' << DMcNet->number;
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
    std::cout << "Checking transition nr=" << trans.number << ", inputs=" << trans.input_places.size() << ", outputs=" << (trans.output_places.size() == 1) << endl;
    return ((trans.input_places.size() == 1) && (trans.output_places.size() == 1));
  }
#if 0
  static void AddMcPlc(ptnet_transitions_list_t macronet, ptnet_transitions_list_t transfers, ptnet_places_t& macronet_marking, integer& LastMacro, TContents& contents, boolean& dead, boolean& Danger);

  static void Replace(ptnet_transitions_list_t macronet, byte Repl, byte Subs, boolean& dead, boolean& Danger)
  {
    char Ch;

    if (Repl != Subs)
      while (macronet != nil)
      {
        if (macronet->input_places.has(Repl))
          if ((!(macronet->input_places.has(Subs))) ||
            (macronet->input_places == macronet->output_places))
            macronet->input_places = macronet->input_places - set::of(Repl, eos) + set::of(Subs, eos);
          else
          {
            dead = true;
            Danger = true;
          }
        if (macronet->output_places.has(Repl))
          if ((!(macronet->output_places.has(Subs))) ||
            (macronet->input_places == macronet->output_places))
            macronet->output_places = macronet->output_places - set::of(Repl, eos) + set::of(Subs, eos);
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

    for (I = 1; I <= ptnet_places_t::max; I++)
    {
      if (transfers->input_places.has(I))  inp_ = I;
      if (transfers->output_places.has(I))  Out = I;
    }
    /* As in the original, place 100 is not treated as an ordinary place here. */
    if ((inp_ < ptnet_places_max) && (Out < ptnet_places_max))
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
        if ((inp_ > ptnet_places_max) && (Out > ptnet_places_max))
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
    while ((Net->number != Trns->number) && (Net != nil))
      Net = Net->NextTrn;
    Trns->input_places = Net->input_places;
    Trns->output_places = Net->output_places;
    Trns->NextTrn = transfers;
    transfers = Trns;
  }

  static void PlcOfMacro(ptnet_transitions_list_t Trns, ptnet_places_t& macronet_places)
  {
    macronet_places = set::of(eos);
    while (Trns != nil)
    {
      macronet_places = macronet_places + Trns->input_places;
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
      trans.number = it->number;
      trans.input_places = it->input_places;
      trans.output_places = it->output_places;
      petrinet.macronet.push_back(trans);
    }

    /* ... and a copy of net markings */
    petrinet.macronet_marking = petrinet.net_marking;

    //for (I = ptnet_macroplace_first; I <= ptnet_macroplace_last; I++) contents[I] = set::of(eos);
    //LastMacro = ptnet_macroplace_first - 1;
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
