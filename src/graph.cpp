/******************************************************************************/
/*                     Department of Computer Engineering                     */
/*                Higher College of Engineering in Zielona Gora               */
/*                                                                            */
/*                Decomposition and Place Encoding of Petri Net.              */
/*             Master's thesis, Supervisor Prof. Marcin Adamski.              */
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

  /* Place name as written in the macronet structure, e.g. "P14" or "MP101". */
  static std::string ptnetPlaceName(int place)
  {
    return std::string(place > ptnet_places_max ? "MP" : "P") + std::to_string(place);
  }

  static void ptnetWriteMacronetStructure(std::ofstream& file, const PetriNet& petrinet)
  {
    file << " Macronet structure." << endl;
    file << "---------------------" << endl;
    file << endl;
    file << "TRANSITIONS" << endl;
    for (const ptnet_transition_t& transition : petrinet.macronet)
    {
      const char* separator = "";

      file << 'T' << transition.number << ": ";
      for (int place : transition.input_places)
      {
        file << separator << ptnetPlaceName(place);
        separator = " * ";
      }
      file << " |- ";
      separator = "";
      for (int place : transition.output_places)
      {
        file << separator << '@' << ptnetPlaceName(place);
        separator = " * ";
      }
      file << ';' << endl;
    }
    file << endl;
    for (int macroplace = ptnet_macroplace_first; macroplace <= petrinet.last_macroplace; macroplace++)
    {
      const ptnet_places_t& contents = petrinet.macroplace_contents[macroplace];
      const char* separator = "";

      file << "MACROPLACE " << macroplace << endl;
      file << "    Internal places      : ";
      for (int place : contents)
      {
        /* As in the original, place 100 is not listed. */
        if (place < ptnet_places_max)
        {
          file << separator << 'P' << place;
          separator = ",";
        }
      }
      file << endl;
      file << "    Internal transitions : ";
      separator = "";
      for (const ptnet_transition_t& transfer : petrinet.transfers)
      {
        for (int place : transfer.input_places * contents)
        {
          if (place <= ptnet_places_max)
          {
            file << separator << 'T' << transfer.number;
            separator = ",";
          }
        }
      }
      file << endl;
      file << "    Border transitions   : ";
      separator = "";
      for (const ptnet_transition_t& transition : petrinet.macronet)
      {
        if (transition.input_places.has(macroplace) || transition.output_places.has(macroplace))
        {
          file << separator << 'T' << transition.number;
          separator = ",";
        }
      }
      file << endl;
      file << endl;
    }
  }

  void PTNetWriteMacronet(PetriNet& petrinet)
  {
    std::string out_name = PTNetFileNameNew(petrinet.file_name, "mcn");
    std::ofstream file(out_name);

    std::cout << "Macro file name: " << out_name << endl;
    if (file.is_open())
    {
      ptnetWriteMacronetStructure(file, petrinet);
      file.close();
    }
  }

  /* Transfer transition: exactly one input place and one output place. */
  static bool ptnetIsTransfer(const ptnet_transition_t& transition)
  {
    return (transition.input_places.size() == 1) && (transition.output_places.size() == 1);
  }

  /*
   * Replace place "replaced" with "substitute" in all transitions. Both places
   * becoming inputs (or outputs) of the same transition makes the net unsafe.
   */
  static void ptnetReplacePlace(ptnet_transitions_list_t& transitions, int replaced, int substitute, bool& dead, bool& unsafe)
  {
    if (replaced != substitute)
    {
      for (ptnet_transition_t& transition : transitions)
      {
        if (transition.input_places.has(replaced))
        {
          if (!transition.input_places.has(substitute) || (transition.input_places == transition.output_places))
          {
            transition.input_places.erase(replaced);
            transition.input_places.insert(substitute);
          }
          else
          {
            dead = true;
            unsafe = true;
          }
        }
        if (transition.output_places.has(replaced))
        {
          if (!transition.output_places.has(substitute) || (transition.input_places == transition.output_places))
          {
            transition.output_places.erase(replaced);
            transition.output_places.insert(substitute);
          }
          else
          {
            dead = true;
            unsafe = true;
          }
        }
      }
    }
  }

  /* Merge the input and output place of a transfer transition into a macroplace. */
  static void ptnetAddMacroplace(PetriNet& petrinet, const ptnet_transition_t& transfer, bool& dead, bool& unsafe)
  {
    ptnet_macroplace_contents_t& contents = petrinet.macroplace_contents;
    ptnet_places_t& marking = petrinet.macronet_marking;
    int input = *transfer.input_places.begin();
    int output = *transfer.output_places.begin();

    /* As in the original, place 100 is not treated as an ordinary place here. */
    if ((input < ptnet_places_max) && (output < ptnet_places_max))
    {
      /* Two ordinary places make a new macroplace. */
      int macroplace = ++petrinet.last_macroplace;

      contents[macroplace] = ptnet_places_t{ input, output };
      if (marking.has(input) || marking.has(output))
      {
        marking -= ptnet_places_t{ input, output };
        marking.insert(macroplace);
      }
      ptnetReplacePlace(petrinet.macronet, input, macroplace, dead, unsafe);
      ptnetReplacePlace(petrinet.macronet, output, macroplace, dead, unsafe);
    }
    else if (input != output)
    {
      if ((input > ptnet_places_max) && (output > ptnet_places_max))
      {
        /* Two macroplaces are merged into the lower one and the ones above are renumbered. */
        int kept = std::min(input, output);
        int removed = std::max(input, output);

        ptnetReplacePlace(petrinet.macronet, removed, kept, dead, unsafe);
        contents[kept] += contents[removed];
        if (marking.has(removed))
        {
          marking.erase(removed);
          marking.insert(kept);
        }
        /* As in the original, the marking is not renumbered. */
        for (int macroplace = removed + 1; macroplace <= petrinet.last_macroplace; macroplace++)
        {
          ptnetReplacePlace(petrinet.macronet, macroplace, macroplace - 1, dead, unsafe);
        }
        for (int macroplace = removed + 1; macroplace <= petrinet.last_macroplace; macroplace++)
        {
          contents[macroplace - 1] = contents[macroplace];
        }
        contents[petrinet.last_macroplace].clear();
        petrinet.last_macroplace--;
      }
      else
      {
        /* An ordinary place joins a macroplace. */
        int macroplace = std::max(input, output);
        int place = std::min(input, output);

        contents[macroplace].insert(place);
        if (marking.has(place))
        {
          marking.erase(place);
          marking.insert(macroplace);
        }
        ptnetReplacePlace(petrinet.macronet, place, macroplace, dead, unsafe);
      }
    }
  }

  /* Store a removed transfer transition with its original places, prepended as in the original. */
  static void ptnetAddTransfer(PetriNet& petrinet, ptnet_transition_t transfer)
  {
    for (const ptnet_transition_t& transition : petrinet.net)
    {
      if (transition.number == transfer.number)
      {
        transfer.input_places = transition.input_places;
        transfer.output_places = transition.output_places;
        break;
      }
    }
    petrinet.transfers.insert(petrinet.transfers.begin(), transfer);
  }

  /* Remove the transfer transition at "index" from the macronet and merge its places. */
  static void ptnetRemoveTransfer(PetriNet& petrinet, size_t index, bool& dead, bool& unsafe)
  {
    ptnet_transition_t transfer = petrinet.macronet[index];

    petrinet.macronet.erase(petrinet.macronet.begin() + index);
    ptnetAddMacroplace(petrinet, transfer, dead, unsafe);
    ptnetAddTransfer(petrinet, transfer);
  }

  void PTNetCreateMacronet(PetriNet& petrinet, bool& dead, bool& unsafe)
  {
    dead = false;
    unsafe = false;

    /* The macronet starts as a copy of the net places and marking. */
    petrinet.macronet.clear();
    for (const ptnet_transition_t& transition : petrinet.net)
    {
      ptnet_transition_t copy;

      copy.number = transition.number;
      copy.input_places = transition.input_places;
      copy.output_places = transition.output_places;
      petrinet.macronet.push_back(copy);
    }
    petrinet.macronet_marking = petrinet.net_marking;
    petrinet.transfers.clear();
    for (ptnet_places_t& contents : petrinet.macroplace_contents)
    {
      contents.clear();
    }
    petrinet.last_macroplace = ptnet_macroplace_first - 1;

    if (!petrinet.macronet.empty())
    {
      /* Transfers at the beginning of the list, the last remaining transition is kept. */
      if (ptnetIsTransfer(petrinet.macronet.front()))
      {
        do
        {
          ptnetRemoveTransfer(petrinet, 0, dead, unsafe);
        } while (!petrinet.macronet.empty() && ptnetIsTransfer(petrinet.macronet.front()) && (petrinet.macronet.size() > 1) && !dead);
      }
      /* Transfers in the rest of the list, the first transition is not checked again. */
      size_t index = 1;
      while ((index < petrinet.macronet.size()) && !dead)
      {
        if (ptnetIsTransfer(petrinet.macronet[index]))
        {
          ptnetRemoveTransfer(petrinet, index, dead, unsafe);
        }
        else
        {
          index++;
        }
      }
    }

    petrinet.macronet_places.clear();
    for (const ptnet_transition_t& transition : petrinet.macronet)
    {
      petrinet.macronet_places += transition.input_places;
    }

    if (petrinet.save_intermediate_results)
    {
      PTNetWriteMacronet(petrinet);
    }
  }
}
