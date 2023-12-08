/******************************************************************************/
/*                     Department of Computer Engineering                     */
/*                Higher College of Engineering in Zielona Gora               */
/*                                                                            */
/*                Decomposition and Place Encoding of Petri Net.              */
/*                 Master's thesis, Supervisor Prof. M.Adamski.               */
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

  static std::string& trimLine(std::string& line, const std::string& whitespace = " \r\n\t\v\f")
  {
    line.erase(0UL, line.find_first_not_of(whitespace));
    line.erase(line.find_last_not_of(whitespace) + 1);
    return line;
  }

  static std::string& removeSpaces(std::string& line)
  {
    line.erase(std::remove(line.begin(), line.end(), ' '), line.end());
    return line;
  }

  static void toUpperLine(std::string& line)
  {
    std::transform(line.begin(), line.end(), line.begin(), ::toupper);
  }

  static ptnet_file_error_t getNumber(std::string& data, std::size_t pos, int& number)
  {
    ptnet_file_error_t error = DATA_FILE_OK;
    size_t size = 0;

    number = std::stoi(data.substr(pos), &size);
    if (size == 0)
    {
      std::cerr << "Place missformat: " << data << endl;
      error = DATA_FILE_PARSE_ERROR;
    }
    return error;
  }

  static ptnet_file_error_t parseNumber(std::string& data, char prefix, int& number)
  {
    ptnet_file_error_t error = DATA_FILE_OK;

    if (data[0] == prefix)
    {
      error = getNumber(data, 1UL, number);
    }
    else
    {
      std::cerr << "Unexpected prefix: " << data[0] << "(" << prefix << ")" << endl;
      error = DATA_FILE_PARSE_ERROR;
    }
    return error;
  }

  static ptnet_file_error_t parseTransitionInput(std::string& data, ptnet_transition_t& trans)
  {
    ptnet_file_error_t error = DATA_FILE_OK;
    int number = 0;

    if (data[0] == 'P')
    {
      error = getNumber(data, 1UL, number);
      if (!error)
      {
        trans.InpPlc.insert(number);
      }
    }
    else if (data[0] == 'X')
    {
      error = getNumber(data, 1UL, number);
      if (!error)
      {
        trans.Inp.insert(number);
      }
    }
    else if ((data[0] == '/') && (data[1] == 'X'))
    {
      error = getNumber(data, 2UL, number);
      if (!error)
      {
        trans.Inp.insert(number);
        trans.InpN.insert(number);
      }
    }
    return error;
  }

  static ptnet_file_error_t parseTransitionOutput(std::string& data, ptnet_transition_t& trans)
  {
    ptnet_file_error_t error = DATA_FILE_OK;
    int number = 0;

    if ((data[0] == '@') && (data[1] == 'P'))
    {
      error = getNumber(data, 2UL, number);
      if (!error)
      {
        trans.OutPlc.insert(number);
      }
    }
    else if ((data[0] == 'S') && (data[1] == 'Y'))
    {
      error = getNumber(data, 2UL, number);
      if (!error)
      {
        trans.Out.insert(number);
      }
    }
    else if ((data[0] == 'R') && (data[1] == 'Y'))
    {
      error = getNumber(data, 2UL, number);
      if (!error)
      {
        trans.Out.insert(number);
        trans.OutR.insert(number);
      }
    }
    return error;
  }

  static ptnet_file_error_t parseTransitionInputsOutputs(std::string& data, ptnet_transition_t& trans, bool input)
  {
    ptnet_file_error_t error = DATA_FILE_OK;
    std::stringstream ss(data);
    std::string item;

    while (std::getline(ss, item, '*') && !error)
    {
      if (input)
      {
        error = parseTransitionInput(item, trans);
      }
      else
      {
        error = parseTransitionOutput(item, trans);
      }
    }
    return error;
  }

  static ptnet_file_error_t parseTransitionSequence(std::string& data, ptnet_transition_t& trans)
  {
    ptnet_file_error_t error = DATA_FILE_OK;
    size_t pos = data.find("|-");

    if (pos != std::string::npos)
    {
      std::string inputs = data.substr(0UL, pos);
      std::string outputs = data.substr(pos + 2);

      error = parseTransitionInputsOutputs(inputs, trans, true);
      if (!error)
      {
        error = parseTransitionInputsOutputs(outputs, trans, false);
      }
    }
    return error;
  }

  static ptnet_file_error_t parseTransition(std::string& data, int number, ptnet_transitions_list_t& Net)
  {
    ptnet_file_error_t error = DATA_FILE_OK;
    ptnet_transition_t trans;

    trans.Nr = number;
    error = parseTransitionSequence(removeSpaces(data), trans);
    if (!error)
    {
      Net.push_back(trans);
    }
    return error;
  }

  static ptnet_file_error_t parseRegisterInput(std::string& data, ptnet_register_t& regis)
  {
    ptnet_file_error_t error = DATA_FILE_OK;
    int number = 0;

    if (data[0] == 'P')
    {
      error = getNumber(data, 1UL, number);
      if (!error)
      {
        regis.Plc.insert(number);
      }
    }
    return error;
  }

  static ptnet_file_error_t parseRegisterOutput(std::string& data, ptnet_register_t& regis)
  {
    ptnet_file_error_t error = DATA_FILE_OK;
    int number = 0;

    if (data[0] == 'Y')
    {
      error = getNumber(data, 1UL, number);
      if (!error)
      {
        regis.Out.insert(number);
      }
    }
    return error;
  }

  static ptnet_file_error_t parseRegisterInputsOutputs(std::string& data, ptnet_register_t& regis, bool input)
  {
    ptnet_file_error_t error = DATA_FILE_OK;
    std::stringstream ss(data);
    std::string item;

    while (std::getline(ss, item, '*') && !error)
    {
      if (input)
      {
        error = parseRegisterInput(item, regis);
      }
      else
      {
        error = parseRegisterOutput(item, regis);
      }
    }
    return error;
  }

  static ptnet_file_error_t parseOutputSequence(std::string& data, ptnet_register_t& regis)
  {
    ptnet_file_error_t error = DATA_FILE_OK;
    size_t pos = data.find("|-");

    if (pos != std::string::npos)
    {
      std::string inputs = data.substr(0UL, pos);
      std::string outputs = data.substr(pos + 2);

      error = parseRegisterInputsOutputs(inputs, regis, true);
      if (!error)
      {
        error = parseRegisterInputsOutputs(outputs, regis, false);
      }
    }
    return error;
  }

  static ptnet_file_error_t parseOutput(std::string& data, ptnet_registers_list_t& Reg)
  {
    ptnet_file_error_t error = DATA_FILE_OK;
    ptnet_register_t regis;

    error = parseOutputSequence(removeSpaces(data), regis);
    if (!error)
    {
      Reg.push_back(regis);
    }
    return error;
  }

  static ptnet_file_error_t parsePlaces(std::string& data, char prefix, ptnet_places_t& places)
  {
    ptnet_file_error_t error = DATA_FILE_OK;
    std::stringstream ss(removeSpaces(data));
    std::string item;

    while (std::getline(ss, item, ',') && !error)
    {
      int number = 0;

      error = parseNumber(item, prefix, number);
      if (!error)
      {
        places.insert(number);
      }
    }
    return error;
  }

  static ptnet_file_error_t verityNetPlaces(PetriNet& petrinet)
  {
    ptnet_file_error_t error = DATA_FILE_OK;
    ptnet_places_t AllPlc, AllInp, AllReg, AllCom;

    for (ptnet_transitions_list_t::iterator it = petrinet.net.begin(); it != petrinet.net.end(); it++)
    {
      AllPlc.insert(it->InpPlc.begin(), it->InpPlc.end());
      AllPlc.insert(it->OutPlc.begin(), it->OutPlc.end());
      AllInp.insert(it->Inp.begin(), it->Inp.end());
      AllReg.insert(it->Out.begin(), it->Out.end());
    }
    for (ptnet_registers_list_t::iterator it = petrinet.reg.begin(); it != petrinet.reg.end(); it++)
    {
      AllCom.insert(it->Out.begin(), it->Out.end());
    }
    if ((AllPlc != petrinet.input_data.Plc) || (AllInp != petrinet.input_data.Inp) || (AllReg != petrinet.input_data.Reg) || (AllCom != petrinet.input_data.Com))
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
        toUpperLine(trimLine(line));
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

            error = parseNumber(trans, 'T', number);
            if (!error)
            {
              error = parseTransition(data, number, petrinet.net);
            }
          }
        }
        else if (line == "OUTPUTS")
        {
          section_data = 2; /* Proceed outputs */
        }
        else if (section_data == 2)
        {
          error = parseOutput(line, petrinet.reg);
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
              error = parsePlaces(data, 'P', petrinet.input_data.Plc);
            }
            else if (key == "INPUTS")
            {
              error = parsePlaces(data, 'X', petrinet.input_data.Inp);
            }
            else if (key == "REG_OUTS")
            {
              error = parsePlaces(data, 'Y', petrinet.input_data.Reg);
            }
            else if (key == "COMB_OUTS")
            {
              error = parsePlaces(data, 'Y', petrinet.input_data.Com);
            }
            else if (key == "MARKINGS")
            {
              error = parsePlaces(data, 'P', petrinet.net_marking);
            }
          }
        }
      }
      file.close();
    }
    if (!error)
    {
      error = verityNetPlaces(petrinet);
    }
    return error;
  }
}
