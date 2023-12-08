/******************************************************************************/
/*                     Department of Computer Engineering                     */
/*                Higher College of Engineering in Zielona Gora               */
/*                                                                            */
/*                Decomposition and Place Encoding of Petri Net.              */
/*                 Master's thesis, Supervisor Prof. M.Adamski.               */
/*                                                                            */
/*                                            Author: Mariusz Janczak (1993)  */
/******************************************************************************/

#include <getopt.h>
#include "petri.h"
#include "tools.h"
//#include "encoding.h"
//#include "automat.h"
//#include "colored.h"
#include "graph.h"

PPeak Peak;
PConcurrency Concurrency;
PNonconcurrency NonConcurrency;
PVerified Verified;
PCodes McCodes, PlcCodes;
PListCodes ListCodes;

TPalete Color, McColor;

bool dead, Finish, Danger, Rst;

int LastMacro, ColorsNr, What;

void PetriNet::About(void)
{
  std::cout
    << "Department of Computer Engineering" << endl
    << "Higher College of Engineering in Zielona Gora" << endl << endl
    << "Decomposition and Place Encoding of Petri Net." << endl
    << "Master of science thesis, Supervisor Prof. Marcin Adamski." << endl << endl
    << "(c) Mariusz Janczak (1993-2023)" << endl;
}

static std::string ptnetGetMessage(int message_id)
{
  std::string message;
  switch (message_id)
  {
    case 21:
    case 22: message = "Failed to read data file."; break;
    case 23:
    case 24: message = "Input data syntax error."; break;
    case 25:
    case 26: message = "The net is not save."; break;
    case 27:
    case 28: message = "The net is not live."; break;
  }
  return message;
}

static void ptnetShowMessage(std::string message)
{
  std::cout << message << endl;
}

/*
 * PetriNet class constructor.
 */
PetriNet::PetriNet(const std::string& fname) : file_name(fname)
{
  save_intermediate_results = true;
  file_status = DATA_FILE_UNKNOWN;

  if (file_name.length())
  {
    std::cout << "Input file name: " << file_name << endl;
    file_status = PTNetReadData(*this);
    if (file_status)
    {
      ptnetShowMessage(ptnetGetMessage(file_status == DATA_FILE_OPEN_ERROR ? 21 : 23));
    }
  }
}

/*
 * PetriNet class destructor.
 */
PetriNet::~PetriNet()
{
  if (file_name.length())
  {
    std::cout << "Releasing file " << file_name << " data." << endl;
  }
}

/*
 * Creating reachability graph of net/macronet.
 */
void PetriNet::ReachabilityGraph(bool macro)
{
  if (file_status == DATA_FILE_OK)
  {
    if (!macro)
    {
      PTNetCreateGraph(*this, macro, Peak, dead);
    }
    else
    {
#if 0
      PTNetCreateMacronet(fname, false, Net, Marking, macronet, transfers, macronet_marking, macronet_places, contents, LastMacro, dead, Danger);
      if ((!dead) && (!Danger))
      {
        PTNetCreateGraph(fname, true, macro, macronet, macronet_marking, branches, Peak, dead);
      }
      else
      {
        if (Danger)
        {
          ptnetShowMessage(ptnetGetMessage(25));
        }
      }
#endif
    }
#if 0
    if (dead)
    {
      ptnetShowMessage(ptnetGetMessage(27));
    }
#endif
  }
}

/*
 * Creating macronet.
 */
void PetriNet::CreateMacronet(void)
{
  if (file_status == DATA_FILE_OK)
  {
#if 0
    PTNetCreateMacronet(fname, true, Net, Marking, macronet, transfers, macronet_marking, macronet_places, contents, LastMacro, dead, Danger);
    if ((!dead) && (!Danger))
    {
      OpenEditor(fname, true);
    }
    else
    {
      if (dead)
      {
        ptnetShowMessage(ptnetGetMessage(27));
      }
      else
      {
        ptnetShowMessage(ptnetGetMessage(25));
      }
    }
#endif
  }
}

/*
 * Coloring net or macronet.
 */
void PetriNet::Coloring(bool /*macro*/)
{
  if (file_status == DATA_FILE_OK)
  {
#if 0
    dead = false;
    PTNetCreateMacronet(fname, false, Net, Marking, macronet, transfers, macronet_marking, macronet_places, contents, LastMacro, dead, Danger);
    if ((!dead) && (!Danger))
    {
      PTNetCreateGraph(fname, false, 2, macronet, macronet_marking, macronet_branches, Peak, dead);
      if (dead) {
        ptnetShowMessage(ptnetGetMessage(27));
      }
      else
      {
        Coloring(macronet_branches, McColor, ColorsNr);
        if (What == 1) /* Kolorowanie sieci */
        {
          WhichColor(McColor, ColorsNr, contents, LastMacro, Color);
          WriteColored(fname, Net, Color, ColorsNr, What, Reg, false);
        }
        else /* Kolorowanie makrosieci */
        {
          WriteColored(fname, macronet, McColor, ColorsNr, What, Reg, true);
        }
        OpenEditor(fname, true);
      }
    }
    else
    {
      if (dead)
      {
        ptnetShowMessage(ptnetGetMessage(27));
      }
      else
      {
        ptnetShowMessage(ptnetGetMessage(25));
      }
    }
#endif
  }
}

static void CreateChoseDecompositionDialog(int& What)
{
  ptnet_flip_flop_type_t Data = flip_flop_d;

  if (Data == flip_flop_d)
  {
    What = 1;
  }
  else
  {
    What = 2;
  }
}

/*
 * Dekompozycja na skladowe automatowe
 */
void PetriNet::Decomposition(void)
{
  //ptnet_places_t Place;

  CreateChoseDecompositionDialog(What);
  if (file_status == DATA_FILE_OK)
  {
#if 0
    PTNetCreateMacronet(fname, false, Net, Marking, macronet, transfers, macronet_marking, macronet_places, contents, LastMacro, dead, Danger);
    if ((!dead) && (!Danger))
    {
      PTNetCreateGraph(fname, false, 2, macronet, macronet_marking, macronet_branches, Peak, dead);
      if (!dead)
      {
        Coloring(macronet_branches, McColor, ColorsNr);
        WhichColor(McColor, ColorsNr, contents, LastMacro, Color);
        Automats(fname, input_data, Net, Reg, Marking, Color, ColorsNr, What);
        OpenEditor(fname, true);
      }
      else
      {
        ptnetShowMessage(ptnetGetMessage(27));
      }
    }
    else
    {
      if (dead)
      {
        ptnetShowMessage(ptnetGetMessage(27));
      }
      else
      {
        ptnetShowMessage(ptnetGetMessage(25));
      }
    }
#endif
  }
}

static void CreateChoseEncodingDialog(int& What)
{
  What = encoded_sequents;
}

static void CreateChangeInitialDialog(bool& Rst)
{
  ptnet_encoding_init_t What = initial_code_reset;

  Rst = (What == initial_code_reset);
}
#if 0
static void WriteResultFile(unsigned int value, int Bit, boolean& WrResF)
{
  WrResF = (value & (1 << Bit)) ? true : false;
}
#endif
/*
 * Kodowanie miejsc sieci Petriego
 */
void PetriNet::Encoding(void)
{
  //bool WrResF;
  int ByteStr;

  CreateChoseEncodingDialog(ByteStr);
  if (ByteStr != 0)
  {
    CreateChangeInitialDialog(Rst);
    if (file_status == DATA_FILE_OK)
    {
      PTNetCreateMacronet(*this, LastMacro, dead, Danger);
      if ((!dead) && (!Danger))
      {
#if 0
        PTNetCreateGraph(fname, false, 2, macronet, macronet_marking, macronet_branches, Peak, dead);
        if (!dead)
        {
          WriteResultFile(ByteStr, 1, WrResF);
          /*poprawi�*/ListOfConcurrency(fname, WrResF, macronet_places, Peak, Concurrency);
          if (WrResF)
          {
            OpenEditor(fname, true);
          }
          WriteResultFile(ByteStr, 2, WrResF);
          ListOfNonConcurrency(fname, WrResF, Concurrency, macronet_places, NonConcurrency);
          if (WrResF)
          {
            OpenEditor(fname, true);
          }
          WriteResultFile(ByteStr, 3, WrResF);
          VerifiedList(fname, WrResF, NonConcurrency, Verified);
          if (WrResF)
          {
            OpenEditor(fname, true);
          }
          WriteResultFile(ByteStr, 4, WrResF);
          CodesOfMcPlaces(fname, WrResF, macronet_places, Concurrency, Verified, McCodes);
          if (WrResF)
          {
            OpenEditor(fname, true);
          }
          WriteResultFile(ByteStr, 5, WrResF);
          CodesOfPlaces(fname, WrResF, Rst, Concurrency, Peak, Marking, macronet_places, LastMacro, contents, McCodes, PlcCodes, ListCodes);
          if (WrResF)
          {
            OpenEditor(fname, true);
          }
          WriteResultFile(ByteStr, 6, WrResF);
          if (WrResF)
          {
            FullTable(fname, PlcCodes);
            OpenEditor(fname, true);
          }
          WriteResultFile(ByteStr, 7, WrResF);
          if (WrResF)
          {
            WriteSeq(fname, Net, Reg, input_data, Marking, ListCodes);
            OpenEditor(fname, true);
          }
        }
        else
        {
          ptnetShowMessage(ptnetGetMessage(27));
        }
      }
      else
      {
        if (dead)
        {
          ptnetShowMessage(ptnetGetMessage(27));
        }
        else
        {
          ptnetShowMessage(ptnetGetMessage(25));
        }
#endif
      }
    }
  }
}

static void ptnetHelp(void)
{
  std::cout
    << "Decomposition and Place Encoding of Petri Net." << endl << endl
    << "Usage: PetriNet [options]" << endl << endl
    << "Options:" << endl
    << "-i, --input <filename>      an input file to be processed" << endl
    << "-c, --command <name>        command to be executed" << endl
    << "--help                      display this help and exit" << endl << endl
    << "Commands:" << endl
    << "about                       display information about the program" << endl
    << "reachability-graph          create reachability graph of the net" << endl
    << "coloring                    color the net" << endl
    << "macronet                    create macronet" << endl
    << "reachability-graph-macronet create reachability graph of the macronet" << endl
    << "coloring-macronet           color the macronet" << endl
    << "decomposition               decompose the net into automatons" << endl
    << "encoding                    encode the net places" << endl;
}

typedef enum ptnet_command_e {
  cmd_about,
  cmd_net_reachability_graph,
  cmd_net_coloring,
  cmd_macronet_create,
  cmd_macronet_reachability_graph,
  cmd_macronet_coloring,
  cmd_net_decomposition,
  cmd_net_encoding
} ptnet_command_t;

static void ptnetCommand(const std::string& command, const std::string& fname)
{
  if (command == "help")
  {
    ptnetHelp();
  }
  else {
    PetriNet petrinet(fname);
    if (command == "about")
    {
      petrinet.About();
    }
    else if (command == "reachability-graph")
    {
      petrinet.ReachabilityGraph(false);
    }
    else if (command == "coloring")
    {
      petrinet.Coloring(false);
    }
    else if (command == "macronet")
    {
      petrinet.CreateMacronet();
    }
    else if (command == "reachability-graph-macronet")
    {
      petrinet.ReachabilityGraph(true);
    }
    else if (command == "coloring-macronet")
    {
      petrinet.Coloring(true);
    }
    else if (command == "decomposition")
    {
      petrinet.Decomposition();
    }
    else if (command == "encoding")
    {
      petrinet.Encoding();
    }
  }
}

int main(int argc, char* argv[])
{
  std::string fname = "";
  std::string command = "help";

  if (argc < 2)
  {
    cerr << "error: missing input parameter(s)" << endl << endl;
    ptnetCommand(command, fname);
  }
  else
  {
    extern char* optarg;
    extern int optopt;

    struct option long_options[] = {
        {"help", no_argument, NULL, 'h'},
        {"input", required_argument, NULL, 'i'},
        {"command", required_argument, NULL, 'c'},
        {NULL, 0, NULL, 0}
    };

    while (1)
    {
      int option_index = 0;
      int opt = getopt_long(argc, argv, "hi:c:", long_options, &option_index);

      if (opt == -1)
      {
        break;
      }
      switch (opt)
      {
        case 0:
        {
          cerr << "error: unknown option " << long_options[option_index].name << endl;
          break;
        }
        case '?':
        {
          cerr << "error: unknown option " << optopt << endl;
          break;
        }
        case ':':
        {
          cerr << "error: option " << optopt << " requires an argument" << endl;
          break;
        }
        case 'c':
        {
          command = optarg;
          break;
        }
        case 'i':
        {
          fname = optarg;
          break;
        }
        case 'h':
        default:
        {
          break;
        }
      }
    }
    ptnetCommand(command, fname);
  }
  return 0;
}