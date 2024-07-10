#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cctype>
#include <cstdlib>
#include <map>

using namespace std;

void send_error(int line_number, string error_message)
{
  cerr << "Erro na linha: " << line_number << ". " << endl;
  cerr << error_message << endl;
  exit(1);
}

bool validate_number(string word, int line_number)
{
  if (word[0] == '0' && word[1] == 'x')
  {
    string substr = word.substr(2);
    for (char &c : substr)
    {
      if (not(c >= '0' && c <= '9' || c >= 'A' && c <= 'F'))
      {
        send_error(line_number, "'" + word + "' nao e um hexadecimal valido.");
        return false;
      }
    }
    size_t unconverted_characters;
    int converted_number = stoi(word.substr(2), &unconverted_characters, 16);

    if (unconverted_characters != word.substr(2).length())
    {
      send_error(line_number, "'" + word + "' nao e um hexadecimal valido.");
    }
  }
  else if (word[0] == '-')
  {
    string substr = word.substr(1, word.length());
    for (char &c : substr)
    {
      if (not(c >= '0' && c <= '9'))
      {
        send_error(line_number, "'" + word + "' nao e um numero valido.");
        return false;
      }
    }
  }
  else
  {
    for (char &c : word)
    {
      if (not(c >= '0' && c <= '9'))
      {
        send_error(line_number, "'" + word + "' nao e um numero valido.");
        return false;
      }
    }
  }
  return true;
}

int convert_string_to_int(string number, int line_number)
{
  if (validate_number(number, line_number))
  {
    if (number[0] == '0' && number[1] == 'x')
    {
      return stoi(number, nullptr, 16);
    }
    else if (number[0] == '-')
    {
      return stoi(number.substr(1)) * (-1);
    }
    else
    {
      return stoi(number);
    }
  }
}

void pre_processor(string &input_file_name, string output_file_name)
{
  bool is_constant = false;
  bool has_argument = false;
  bool is_label = false;
  bool can_write = false;
  bool is_equ = false;
  bool is_copy = false;
  bool is_if = false;
  bool skip_next_line = false;
  int line_number = 0;
  string label;
  string line_to_read = "";
  string line_to_write = "";
  map<string, string> dictionary;

  ofstream output_file(output_file_name);

  if (!output_file.is_open())
  {
    cerr << "Erro ao abrir o arquivo: " << output_file_name << endl;
    exit(1);
  }

  ifstream input_file(input_file_name);

  if (!input_file.is_open())
  {
    cerr << "Erro ao abrir o arquivo: " << input_file_name << endl;
    exit(1);
  }

  // Lê o arquivo linha por linha
  while (getline(input_file, line_to_read))
  {
    line_number++;
    istringstream line_readed(line_to_read);

    string word;

    while (line_readed >> word)
    {
      if (word.empty() || word[0] == ';')
      {
        break;
      }

      if (skip_next_line)
      {
        skip_next_line = false;
        break;
      }

      if (word[word.length() - 1] == ':')
      {
        is_label = true;
      }

      /* Transformar tudo para maiusculo */
      if (!is_constant && !is_label)
      {
        for (char &c : word)
        {
          c = toupper(static_cast<unsigned char>(c));
        }
      }

      if (is_equ)
      {
        dictionary[label.substr(0, label.length() - 1)] = word;
        is_equ = false;
      }
      else
      {
        if (word == "EQU")
        {
          is_equ = true;
          line_to_write = "";
        }
        else if (word == "STOP" || word == "SPACE")
        {
          line_to_write = word;
          can_write = true;
        }
        else if (is_constant)
        {
          if (dictionary.find(word) != dictionary.end())
          {
            if (validate_number(dictionary[word], line_number))
            {
              line_to_write += dictionary[word];
            }
          }
          else
          {
            if (validate_number(word, line_number))
            {
              line_to_write += word;
            }
          }
          can_write = true;
          is_constant = false;
        }
        else if (word == "CONST")
        {
          line_to_write += word + " ";
          is_constant = true;
        }
        else if (is_label)
        {
          line_to_write = word + " ";
          label = word;
          is_label = false;
        }
        else if (has_argument)
        {
          line_to_write += word;
          has_argument = false;
          can_write = true;
        }
        else if (is_copy)
        {
          size_t indice_da_virgula = word.find(',');

          if (indice_da_virgula != std::string::npos && indice_da_virgula + 1 < word.length() && indice_da_virgula > 0)
          {
            line_to_write += word;
            can_write = true;
            is_copy = false;
          }
          else
          {
            send_error(line_number, "'COPY' possui argumentos invalidos: " + word);
          }
        }
        else if (word == "COPY")
        {
          is_copy = true;
          line_to_write += word + " ";
        }
        else if (is_if)
        {
          if (dictionary.find(word) != dictionary.end())
          {
            if (convert_string_to_int(dictionary[word], line_number) == 0)
            {
              skip_next_line = true;
            }
          }
          else
          {
            if (convert_string_to_int(word, line_number) == 0)
            {
              skip_next_line = true;
            }
          }
          is_if = false;
        }
        else if (word == "IF")
        {
          is_if = true;
        }
        else
        {
          line_to_write += word + " ";
          has_argument = true;
          can_write = false;
        }

        if (can_write)
        {
          output_file << line_to_write << endl;
          can_write = false;
          line_to_write = "";
        }
      }
    }
  }
  input_file.close();
  output_file.close();
}

void assembler(string &input_file_name, string output_file_name)
{
  bool can_write = false;
  bool is_label = false;
  int line_number = 0;
  string line_to_read = "";
  string line_to_write = "";

  ofstream output_file(output_file_name);

  if (!output_file.is_open())
  {
    cerr << "Erro ao abrir o arquivo: " << output_file_name << endl;
    exit(1);
  }

  ifstream input_file(input_file_name);

  if (!input_file.is_open())
  {
    cerr << "Erro ao abrir o arquivo: " << input_file_name << endl;
    exit(1);
  }

  // Lê o arquivo linha por linha
  while (getline(input_file, line_to_read))
  {
    line_number++;
    istringstream line_readed(line_to_read);

    string word;

    while (line_readed >> word)
    {
      if (word.empty())
      {
        break;
      }

      if (word[word.length() - 1] == ':')
      {
        is_label = true;
      }

      if (can_write)
      {
        output_file << line_to_write << endl;
        can_write = false;
        line_to_write = "";
      }
    }
  }
  input_file.close();
  output_file.close();
}

int main(int argc, char *argv[])
{
  string output_file_name;

  if (argc != 3)
  {
    cerr << "Modo de execucao incorreto, tente: " << endl;
    cerr << "<arquivo.cpp>" << " -p <arquivo.asm>" << endl;
    cerr << "OU:" << endl;
    cerr << "<arquivo.cpp>" << " -o <arquivo.pre>" << endl;
    return 1;
  }

  string argument_1 = argv[0];
  string argument_2 = argv[1];
  string argument_3 = argv[2];

  if (argument_2 != "-p" && argument_2 != "-o")
  {
    cerr << "argumento: '" + argument_2 + "' nao reconhecido" << endl;
    return 1;
  }

  if (argument_3.length() - 4 <= 0)
  {
    cerr << "Arquivo '" + argument_3 + "' nao aceito" << endl;
    return 1;
  }

  if (argument_3.substr(argument_3.length() - 4) == ".asm")
  {
    if (argument_2 == "-o")
    {
      cerr << "Argumento '" + argument_2 + "' incorreto para arquivo com final '" + argument_3.substr(argument_3.length() - 4) << endl;
      return 1;
    }
  }
  else if (argument_3.substr(argument_3.length() - 4) == ".pre")
  {
    if (argument_2 == "-p")
    {
      cerr << "Argumento '" + argument_2 + "' incorreto para arquivo com final '" + argument_3.substr(argument_3.length() - 4) << endl;
      return 1;
    }
  }
  else
  {
    cerr << "Arquivo '" + argument_3 + "' nao e aceito." << endl;
    return 1;
  }

  if (argument_2 == "-p")
  {
    output_file_name = "myfile.pre";
    pre_processor(argument_3, output_file_name);
  }
  else
  {
    output_file_name = "myfile.obj";
    pre_processor(argument_3, output_file_name);
  }

  return 0;
}

/* Terminar o montador (assembler) */
/* Começar o ligador */