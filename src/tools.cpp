/******************************************************************************/
/*                     Department of Computer Engineering                     */
/*                Higher College of Engineering in Zielona Gora               */
/*                                                                            */
/*                Decomposition and Place Encoding of Petri Net.              */
/*             Master's thesis, Supervisor Prof. Marcin Adamski.              */
/*                                                                            */
/*                                            Author: Mariusz Janczak (1993)  */
/******************************************************************************/

#include <fstream>
#include <sstream>
#include <algorithm>
#include "tools.h"

namespace tools {

  std::string PTNetFileNameNew(const std::string& file_name, const std::string& file_ext)
  {
    std::string new_name(file_name);
    size_t length = new_name.length();
    size_t found = new_name.rfind('.');

    if (found != string::npos)
    {
      new_name.replace(found + 1, length - found - 1, file_ext);
    }
    return new_name;
  }

  static std::string& ptnetTrimLine(std::string& line, const std::string& whitespace = " \r\n\t\v\f")
  {
    line.erase(0UL, line.find_first_not_of(whitespace));
    line.erase(line.find_last_not_of(whitespace) + 1);
    return line;
  }

  static std::string& ptnetRemoveSpaces(std::string& line)
  {
    line.erase(std::remove(line.begin(), line.end(), ' '), line.end());
    return line;
  }

  static void ptnetToUpperLine(std::string& line)
  {
    std::transform(line.begin(), line.end(), line.begin(), ::toupper);
  }

  static ptnet_file_error_t ptnetGetNumber(std::string& data, std::size_t pos, int& number)
  {
    ptnet_file_error_t error = DATA_FILE_OK;
    size_t size = 0;

    number = std::stoi(data.substr(pos), &size);
    if ((size == 0) || (number < 0) || (number > ptnet_places_t::max))
    {
      std::cerr << "Place missformat: " << data << endl;
      error = DATA_FILE_PARSE_ERROR;
    }
    return error;
  }

  static ptnet_file_error_t ptnetParseNumber(std::string& data, char prefix, int& number)
  {
    ptnet_file_error_t error = DATA_FILE_OK;

    if (data[0] == prefix)
    {
      error = ptnetGetNumber(data, 1UL, number);
    }
    else
    {
      std::cerr << "Unexpected prefix: " << data[0] << "(" << prefix << ")" << endl;
      error = DATA_FILE_PARSE_ERROR;
    }
    return error;
  }

  static ptnet_file_error_t ptnetParseTransitionInput(std::string& data, ptnet_transition_t& trans)
  {
    ptnet_file_error_t error = DATA_FILE_OK;
    int number = 0;

    if (data[0] == 'P')
    {
      error = ptnetGetNumber(data, 1UL, number);
      if (!error)
      {
        trans.input_places.insert(number);
      }
    }
    else if (data[0] == 'X')
    {
      error = ptnetGetNumber(data, 1UL, number);
      if (!error)
      {
        trans.input_signals.insert(number);
      }
    }
    else if ((data[0] == '/') && (data[1] == 'X'))
    {
      error = ptnetGetNumber(data, 2UL, number);
      if (!error)
      {
        trans.input_signals.insert(number);
        trans.negated_inputs.insert(number);
      }
    }
    return error;
  }

  static ptnet_file_error_t ptnetParseTransitionOutput(std::string& data, ptnet_transition_t& trans)
  {
    ptnet_file_error_t error = DATA_FILE_OK;
    int number = 0;

    if ((data[0] == '@') && (data[1] == 'P'))
    {
      error = ptnetGetNumber(data, 2UL, number);
      if (!error)
      {
        trans.output_places.insert(number);
      }
    }
    else if ((data[0] == 'S') && (data[1] == 'Y'))
    {
      error = ptnetGetNumber(data, 2UL, number);
      if (!error)
      {
        trans.output_signals.insert(number);
      }
    }
    else if ((data[0] == 'R') && (data[1] == 'Y'))
    {
      error = ptnetGetNumber(data, 2UL, number);
      if (!error)
      {
        trans.output_signals.insert(number);
        trans.reset_outputs.insert(number);
      }
    }
    return error;
  }

  static ptnet_file_error_t ptnetParseTransitionInputsOutputs(std::string& data, ptnet_transition_t& trans, bool input)
  {
    ptnet_file_error_t error = DATA_FILE_OK;
    std::stringstream ss(data);
    std::string item;

    while (std::getline(ss, item, '*') && !error)
    {
      if (input)
      {
        error = ptnetParseTransitionInput(item, trans);
      }
      else
      {
        error = ptnetParseTransitionOutput(item, trans);
      }
    }
    return error;
  }

  static ptnet_file_error_t ptnetParseTransitionSequence(std::string& data, ptnet_transition_t& trans)
  {
    ptnet_file_error_t error = DATA_FILE_OK;
    size_t pos = data.find("|-");

    if (pos != std::string::npos)
    {
      std::string inputs = data.substr(0UL, pos);
      std::string outputs = data.substr(pos + 2);

      error = ptnetParseTransitionInputsOutputs(inputs, trans, true);
      if (!error)
      {
        error = ptnetParseTransitionInputsOutputs(outputs, trans, false);
      }
    }
    return error;
  }

  static ptnet_file_error_t ptnetParseTransition(std::string& data, int number, ptnet_transitions_list_t& Net)
  {
    ptnet_file_error_t error = DATA_FILE_OK;
    ptnet_transition_t trans;

    trans.number = number;
    error = ptnetParseTransitionSequence(ptnetRemoveSpaces(data), trans);
    if (!error)
    {
      Net.push_back(trans);
    }
    return error;
  }

  static ptnet_file_error_t ptnetParseOutputPlace(std::string& data, ptnet_output_t& output)
  {
    ptnet_file_error_t error = DATA_FILE_OK;
    int number = 0;

    if (data[0] == 'P')
    {
      error = ptnetGetNumber(data, 1UL, number);
      if (!error)
      {
        output.places.insert(number);
      }
    }
    return error;
  }

  static ptnet_file_error_t ptnetParseOutputSignal(std::string& data, ptnet_output_t& output)
  {
    ptnet_file_error_t error = DATA_FILE_OK;
    int number = 0;

    if (data[0] == 'Y')
    {
      error = ptnetGetNumber(data, 1UL, number);
      if (!error)
      {
        output.outputs.insert(number);
      }
    }
    return error;
  }

  static ptnet_file_error_t ptnetParseOutputPlacesSignals(std::string& data, ptnet_output_t& output, bool input)
  {
    ptnet_file_error_t error = DATA_FILE_OK;
    std::stringstream ss(data);
    std::string item;

    while (std::getline(ss, item, '*') && !error)
    {
      if (input)
      {
        error = ptnetParseOutputPlace(item, output);
      }
      else
      {
        error = ptnetParseOutputSignal(item, output);
      }
    }
    return error;
  }

  static ptnet_file_error_t ptnetParseOutputSequence(std::string& data, ptnet_output_t& output)
  {
    ptnet_file_error_t error = DATA_FILE_OK;
    size_t pos = data.find("|-");

    if (pos != std::string::npos)
    {
      std::string inputs = data.substr(0UL, pos);
      std::string outputs = data.substr(pos + 2);

      error = ptnetParseOutputPlacesSignals(inputs, output, true);
      if (!error)
      {
        error = ptnetParseOutputPlacesSignals(outputs, output, false);
      }
    }
    return error;
  }

  static ptnet_file_error_t ptnetParseOutput(std::string& data, ptnet_outputs_list_t& outputs)
  {
    ptnet_file_error_t error = DATA_FILE_OK;
    ptnet_output_t output;

    error = ptnetParseOutputSequence(ptnetRemoveSpaces(data), output);
    if (!error)
    {
      outputs.push_back(output);
    }
    return error;
  }

  static ptnet_file_error_t ptnetParsePlaces(std::string& data, char prefix, ptnet_places_t& places)
  {
    ptnet_file_error_t error = DATA_FILE_OK;
    std::stringstream ss(ptnetRemoveSpaces(data));
    std::string item;

    while (std::getline(ss, item, ',') && !error)
    {
      int number = 0;

      error = ptnetParseNumber(item, prefix, number);
      if (!error)
      {
        places.insert(number);
      }
    }
    return error;
  }

  static ptnet_file_error_t ptnetVerifyNetPlaces(PetriNet& petrinet)
  {
    ptnet_file_error_t error = DATA_FILE_OK;
    ptnet_places_t all_places, all_inputs, all_registered_outputs, all_combinational_outputs;

    for (ptnet_transitions_list_t::iterator it = petrinet.net.begin(); it != petrinet.net.end(); it++)
    {
      all_places += it->input_places + it->output_places;
      all_inputs += it->input_signals;
      all_registered_outputs += it->output_signals;
    }
    for (ptnet_outputs_list_t::iterator it = petrinet.outputs.begin(); it != petrinet.outputs.end(); it++)
    {
      all_combinational_outputs += it->outputs;
    }
    if ((all_places != petrinet.input_data.places) || (all_inputs != petrinet.input_data.inputs) || (all_registered_outputs != petrinet.input_data.registered_outputs) || (all_combinational_outputs != petrinet.input_data.combinational_outputs))
    {
      std::cerr << "Inconsistent data found" << endl;
      error = DATA_INCONSISTENT;
    }
    return error;
  }

  ptnet_file_error_t PTNetReadData(PetriNet& petrinet)
  {
    ptnet_file_error_t error = DATA_FILE_OK;
    std::ifstream file(petrinet.file_name);

    if (file.is_open() == false)
    {
      error = DATA_FILE_OPEN_ERROR;
    }
    if (!error)
    {
      int section_data = 0;
      std::string line;

      while (!error && std::getline(file, line))
      {
        ptnetToUpperLine(ptnetTrimLine(line));
        if (line[0] == '#')
        {
          continue; /* Skip comments */
        }
        else if (line.empty())
        {
          section_data = 0; /* Exit from section */
        }
        else if (line == "TRANSITIONS")
        {
          section_data = 1; /* Proceed transitions */
        }
        else if (section_data == 1)
        {
          size_t pos = line.find(':');
          if (pos != std::string::npos)
          {
            std::string trans = line.substr(0UL, pos);
            std::string data = line.substr(pos + 1);
            int number = 0;

            error = ptnetParseNumber(trans, 'T', number);
            if (!error)
            {
              error = ptnetParseTransition(data, number, petrinet.net);
            }
          }
        }
        else if (line == "OUTPUTS")
        {
          section_data = 2; /* Proceed outputs */
        }
        else if (section_data == 2)
        {
          error = ptnetParseOutput(line, petrinet.outputs);
        }
        else /* Single line defines */
        {
          size_t pos = line.find(' ');

          if (pos != std::string::npos)
          {
            std::string key = line.substr(0UL, pos);
            std::string data = line.substr(pos + 1);

            if (key == "PLACES")
            {
              error = ptnetParsePlaces(data, 'P', petrinet.input_data.places);
            }
            else if (key == "INPUTS")
            {
              error = ptnetParsePlaces(data, 'X', petrinet.input_data.inputs);
            }
            else if (key == "REG_OUTS")
            {
              error = ptnetParsePlaces(data, 'Y', petrinet.input_data.registered_outputs);
            }
            else if (key == "COMB_OUTS")
            {
              error = ptnetParsePlaces(data, 'Y', petrinet.input_data.combinational_outputs);
            }
            else if (key == "MARKINGS")
            {
              error = ptnetParsePlaces(data, 'P', petrinet.net_marking);
            }
          }
        }
      }
      file.close();
    }
    if (!error)
    {
      error = ptnetVerifyNetPlaces(petrinet);
    }
    return error;
  }
}
