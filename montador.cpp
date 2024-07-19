#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cctype>
#include <cstdlib>
#include <map>
#include <vector>

using namespace std;

struct info_symbol_table
{
  bool defined;
  int value;
  vector<int> addresses_to_correct;
};

map<string, string> instructions = {{"ADD", "01"}, {"SUB", "02"}, {"MUL", "03"}, {"DIV", "04"}, {"JMP", "05"}, {"JMPN", "06"}, {"JMPP", "07"}, {"JMPZ", "08"}, {"COPY", "09"}, {"LOAD", "10"}, {"STORE", "11"}, {"INPUT", "12"}, {"OUTPUT", "13"}, {"STOP", "14"}};

void send_error(int line_number, string error_message, string file_name)
{
  cerr << "Foi encontrado um erro no arquivo: " << file_name;
  cerr << " na linha: " << line_number << ". " << endl;
  cerr << error_message << endl;
  exit(1);
}

void send_symbol_error(string error_message)
{
  cerr << "Foi encontrado um erro na tabela de simbolos: " << endl;
  cerr << error_message << endl;
  exit(1);
}

bool validate_number(string word, int line_number, string file_name, bool send_error_if_false = true)
{
  if (word[0] == '0' && word[1] == 'x')
  {
    string substr = word.substr(2);
    for (char &c : substr)
    {
      if (not(c >= '0' && c <= '9' || c >= 'A' && c <= 'F'))
      {
        if (send_error_if_false)
        {
          send_error(line_number, "'" + word + "' nao e um hexadecimal valido.", file_name);
        }
        return false;
      }
    }
    size_t unconverted_characters;
    int converted_number = stoi(word.substr(2), &unconverted_characters, 16);

    if (unconverted_characters != word.substr(2).length())
    {
      if (send_error_if_false)
      {
        send_error(line_number, "'" + word + "' nao e um hexadecimal valido.", file_name);
      }
      return false;
    }
  }
  else if (word[0] == '-')
  {
    string substr = word.substr(1, word.length());
    for (char &c : substr)
    {
      if (not(c >= '0' && c <= '9'))
      {
        if (send_error_if_false)
        {
          send_error(line_number, "'" + word + "' nao e um numero valido.", file_name);
        }
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
        if (send_error_if_false)
        {
          send_error(line_number, "'" + word + "' nao e um numero valido.", file_name);
        }
        return false;
      }
    }
  }
  return true;
}

int convert_string_to_int(string number, int line_number, string file_name)
{
  if (validate_number(number, line_number, file_name))
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

void label_validation(string label, int line_number, string file_name)
{
  if (label[0] >= '0' && label[0] <= '9')
  {
    send_error(line_number, "Rotulo nao pode comecar com numero.", file_name);
  }

  for (char l : label)
  {
    if (!((l >= 'a' && l <= 'z') || (l >= 'A' && l <= 'Z') || (l >= '0' && l <= '9') || (l == '_')))
    {
      send_error(line_number, "Rotulo com caracter invalido.", file_name);
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
  bool new_line = false;

  int line_number = 0;
  int next_line = 0;

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

    if (line_to_write.size() != 0)
    {
      new_line = true;
    }

    if (next_line != line_number)
    {
      skip_next_line = false;
    }

    istringstream line_readed(line_to_read);

    string word;

    while (line_readed >> word)
    {
      if (word.empty() || word[0] == ';')
      {
        break;
      }

      if (skip_next_line && line_number == next_line)
      {
        break;
      }

      /* cout << "Word: " << word << endl;
      cout << "Line to write: " << line_to_write << endl;
      cout << "Numero da linha: " << line_number << endl;
      cout << "can write e new_line: " << can_write << " " << new_line << endl; */

      if (can_write && new_line)
      {
        output_file << line_to_write << endl;
        can_write = false;
        line_to_write = "";
        new_line = false;
        has_argument = false;
      }

      if (word[word.length() - 1] == ':')
      {
        is_label = true;
      }

      /* Transformar tudo para maiusculo */
      if (!is_constant && !is_label && !is_if)
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
          line_to_write += word + " ";
          can_write = true;
        }
        else if (is_constant)
        {
          if (dictionary.find(word) != dictionary.end())
          {
            if (validate_number(dictionary[word], line_number, input_file_name))
            {
              line_to_write += dictionary[word] + " ";
            }
          }
          else
          {
            if (validate_number(word, line_number, input_file_name))
            {
              line_to_write += word + " ";
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
          line_to_write += word + " ";
          label = word;
          is_label = false;
        }
        else if (has_argument)
        {
          if (dictionary.find(word) != dictionary.end())
          {
            line_to_write += dictionary[word] + " ";
          }
          else
          {
            line_to_write += word + " ";
          }

          has_argument = false;
          can_write = true;
        }
        else if (is_copy)
        {
          size_t comma_pos = word.find(',');

          if (comma_pos != string::npos && comma_pos + 1 < word.length() && comma_pos > 0)
          {
            string before_comma;
            string after_comma;

            before_comma = word.substr(0, comma_pos);
            after_comma = word.substr(comma_pos + 1);

            if (dictionary.find(before_comma) != dictionary.end())
            {
              line_to_write += dictionary[before_comma] + ",";
            }
            else
            {
              line_to_write += before_comma + ",";
            }

            if (dictionary.find(after_comma) != dictionary.end())
            {
              line_to_write += dictionary[after_comma] + " ";
            }
            else
            {
              line_to_write += after_comma + " ";
            }
            can_write = true;
            is_copy = false;
          }
          else
          {
            send_error(line_number, "'COPY' possui argumentos invalidos: " + word, input_file_name);
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
            if (convert_string_to_int(dictionary[word], line_number, input_file_name) == 0)
            {
              skip_next_line = true;
              next_line = line_number + 1;
            }
          }
          else
          {
            if (convert_string_to_int(word, line_number, input_file_name) == 0)
            {
              skip_next_line = true;
              next_line = line_number + 1;
            }
          }
          is_if = false;
          line_to_write = "";
        }
        else if (word == "IF")
        {
          is_if = true;
        }
        else if (word == "BEGIN" || word == "END")
        {
          line_to_write += word + " ";
          can_write = true;
        }
        else
        {

          line_to_write += word + " ";
          has_argument = true;
        }
      }
    }
  }
  if (line_to_write.size() > 0)
  {
    output_file << line_to_write << endl;
  }
  input_file.close();
  output_file.close();
}

void assembler(string input_file_name, string output_file_name)
{
  bool can_write = false;
  bool is_label = false;
  bool has_argument = false;
  bool have_label_in_this_line = false;
  bool have_begin_end = false;
  bool last_was_begin_or_and = false;
  bool last_was_const = false;
  bool last_was_copy = false;

  int new_line = 0;
  int line_number = 0;
  int arguments_counter = 2;
  int address = 0;

  string line_to_read = "";
  string line_to_write = "";

  map<string, info_symbol_table> symbol_table;

  vector<string> source_code;

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

    if (arguments_counter > 0 && line_to_write.size() > 0)
    {
      send_error(line_number - 1, "Falta argumento.", input_file_name);
    }

    if (line_to_write.size() != 0)
    {
      new_line = true;
      have_label_in_this_line = false;
    }

    if (last_was_begin_or_and)
    {
      line_to_write.clear();
      can_write = false;
      new_line = false;
      arguments_counter = 2;
      has_argument = false;
      last_was_begin_or_and = false;
    }

    istringstream line_readed(line_to_read);

    string word;

    while (line_readed >> word)
    {
      if (word.empty())
      {
        break;
      }

      cout << "Word: " << word << endl;
      cout << "Line to write: " << line_to_write << endl;
      cout << "Numero da linha: " << line_number << endl;
      cout << "can write e new_line: " << can_write << " " << new_line << endl;
      cout << "Source code: " << endl;

      for (auto s : source_code)
      {
        cout << "   " << s << endl;
      }

      cout << "Symbol table: " << endl;

      for (map<string, info_symbol_table>::iterator it = symbol_table.begin(); it != symbol_table.end(); ++it)
      {
        cout << "   Chave: " << it->first << ", Valor: " << it->second.value << " definido: " << it->second.defined << " enderecos para corrigir: " << endl;
        for (auto s : it->second.addresses_to_correct)
        {
          cout << "     " << s << endl;
        }
      }

      /* Inserir uma nova linha */
      if (can_write && new_line)
      {
        if (!has_argument)
        {
          send_error(line_number--, "Falta argumento.", input_file_name);
        }

        size_t end = line_to_write.find_last_not_of(" ");
        if (end != string::npos && end + 1 < line_to_write.size())
        {
          line_to_write.erase(end + 1);
        }

        source_code.push_back(line_to_write);
        line_to_write.clear();
        can_write = false;
        new_line = false;
        arguments_counter = 2;
        has_argument = false;
      }

      /* Verificar se é label] */
      if (word[word.length() - 1] == ':')
      {
        is_label = true;
      }

      if (is_label)
      {
        label_validation(word.substr(0, word.length() - 1), line_number, input_file_name);

        if (have_label_in_this_line)
        {
          send_error(line_number, "Rotulo dobrado na mesma linha.", input_file_name);
        }

        if (symbol_table.find(word.substr(0, word.length() - 1)) != symbol_table.end())
        {
          if (symbol_table[word.substr(0, word.length() - 1)].defined)
          {
            send_error(line_number, "Rotulo redefinido.", input_file_name);
          }
          symbol_table[word.substr(0, word.length() - 1)].defined = true;
          symbol_table[word.substr(0, word.length() - 1)].value = address;
        }
        else
        {
          symbol_table[word.substr(0, word.length() - 1)] = {true, address, {}};
        }

        have_label_in_this_line = true;
        is_label = false;
      }
      else if (instructions.find(word) != instructions.end() || word == "CONST")
      {
        if (arguments_counter == 1)
        {
          if (new_line)
          {
            send_error(line_number--, "Falta argumento.", input_file_name);
          }
          else
          {
            send_error(line_number, "Falta argumento.", input_file_name);
          }
        }

        if (word == "COPY")
        {
          last_was_copy = true;
        }

        if (word != "CONST")
        {
          line_to_write += instructions[word] + " ";
        }
        else
        {
          last_was_const = true;
        }

        arguments_counter--;

        if (word == "STOP")
        {
          arguments_counter = 0;
          has_argument = true;
        }
      }
      else if (word == "SPACE")
      {
        line_to_write += "00 ";
        arguments_counter -= 2;
        has_argument = true;
      }
      else if (word == "BEGIN" || word == "END")
      {
        line_to_write.clear();
        arguments_counter = 0;
        has_argument = false;
        last_was_begin_or_and = true;
        have_begin_end = true;
      }
      else if (last_was_copy)
      {
        string before_comma;
        string after_comma;

        size_t comma_pos = word.find(',');

        if (comma_pos != string::npos)
        {
          before_comma = word.substr(0, comma_pos);
          after_comma = word.substr(comma_pos + 1);
        }
        else
        {
          send_error(line_number, "Argumentos do copy em formato incorreto.", input_file_name);
        }

        if (validate_number(before_comma, line_number, input_file_name, false))
        {
          line_to_write += to_string(convert_string_to_int(before_comma, line_number, input_file_name)) + " ";
        }
        else
        {
          if (symbol_table.find(before_comma) != symbol_table.end())
          {
            if (symbol_table[before_comma].defined)
            {
              line_to_write += symbol_table[before_comma].value + " ";
            }
            else
            {
              symbol_table[before_comma].addresses_to_correct.push_back(address);
              line_to_write += "-1 ";
            }
          }
          else
          {
            symbol_table[before_comma] = {false, -1, {address}};
            line_to_write += "-1 ";
          }
        }

        address++;

        if (validate_number(after_comma, line_number, input_file_name, false))
        {
          line_to_write += to_string(convert_string_to_int(after_comma, line_number, input_file_name)) + " ";
        }
        else
        {
          if (symbol_table.find(after_comma) != symbol_table.end())
          {
            if (symbol_table[after_comma].defined)
            {
              line_to_write += symbol_table[after_comma].value + " ";
            }
            else
            {
              symbol_table[after_comma].addresses_to_correct.push_back(address);
              line_to_write += "-1 ";
            }
          }
          else
          {
            symbol_table[after_comma] = {false, -1, {address}};
            line_to_write += "-1 ";
          }
        }

        last_was_copy = false;
        has_argument = true;
        arguments_counter--;
      }
      else if ((arguments_counter > 0 && line_to_write.size() > 0) || last_was_const)
      {
        if (validate_number(word, line_number, input_file_name, false))
        {
          line_to_write += to_string(convert_string_to_int(word, line_number, input_file_name)) + " ";
        }
        else
        {
          if (symbol_table.find(word) != symbol_table.end())
          {
            if (symbol_table[word].defined)
            {
              line_to_write += symbol_table[word].value + " ";
            }
            else
            {
              symbol_table[word].addresses_to_correct.push_back(address);
              line_to_write += "-1 ";
            }
          }
          else
          {
            symbol_table[word] = {false, -1, {address}};
            line_to_write += "-1 ";
          }
        }
        has_argument = true;
        arguments_counter--;
        last_was_const = false;
      }
      else
      {
        send_error(line_number, "Instrucao ou diretiva invalida.", input_file_name);
      }

      if (arguments_counter == 0 && has_argument)
      {
        can_write = true;
      }
      else if (arguments_counter < 0)
      {
        send_error(line_number, "Numero de argumentos incorreto.", input_file_name);
      }

      cout << "Address: " << address << endl;

      /* Se não for label e const e begin e end, soma o endereço */
      if (word[word.length() - 1] != ':' && word != "CONST" && word != "BEGIN" && word != "END")
      {
        address++;
      }
    }
  }

  if (line_to_write.size() > 0)
  {
    if (arguments_counter > 0 && line_to_write.size() > 0)
    {
      send_error(line_number, "Falta argumento.", input_file_name);
    }

    if (can_write)
    {
      if (!has_argument)
      {
        send_error(line_number, "Falta argumento.", input_file_name);
      }

      source_code.push_back(line_to_write);
    }
    else
    {
      send_error(line_number, "Falta argumento.", input_file_name);
    }
  }

  /* Corrigir simbolos indefinidos */
  for (map<string, info_symbol_table>::iterator it = symbol_table.begin(); it != symbol_table.end(); ++it)
  {

    cout << it->first << endl;

    if (it->second.defined)
    {
      for (int s : it->second.addresses_to_correct)
      {
        int contagem = -1;

        for (int i = 0; i < source_code.size(); i++)
        {
          string line = source_code[i];

          istringstream stream(line);
          string word_source_file;

          while (stream >> word_source_file)
          {
            contagem++;
          }

          if (contagem == s)
          {
            size_t position_last_space = line.find_last_of(" ");

            if (position_last_space != string::npos)
            {
              source_code[i] = line.substr(0, position_last_space + 1) + to_string(it->second.value);
            }
            break;
          }
          else if (contagem > s) /* Caso seja copy */
          {
            size_t position_last_space = line.find_last_of(" ");
            size_t position_first_space = line.find_first_of(" ");

            if (position_last_space != string::npos)
            {
              source_code[i] = line.substr(0, position_first_space + 1) + to_string(it->second.value) + line.substr(position_last_space);
            }
            break;
          }
        }
      }
    }
    else
    {
      send_symbol_error("Simbolo nao definido: " + it->first);
    }
  }

  for (string s : source_code)
  {
    output_file << s << " ";
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

  if (argument_3.substr(argument_3.length() - 4) != ".asm")
  {
    cerr << "Arquivo '" + argument_3 + "' nao e aceito." << endl;
    return 1;
  }

  if (argument_2 != "-o" && argument_2 != "-p")
  {
    cerr << "Argumento '" + argument_2 + "' nao aceito. '" + argument_3.substr(argument_3.length() - 4) << endl;
    return 1;
  }

  if (argument_2 == "-p")
  {
    output_file_name = "myfile.pre";
    pre_processor(argument_3, output_file_name);
  }
  else
  {
    output_file_name = "myfile.pre";
    pre_processor(argument_3, output_file_name);

    output_file_name = "myfile.obj";
    assembler("myfile.pre", output_file_name);
  }

  return 0;
}

/* A fazer:
  - Gerar tabela de usos e definição
  - Gerar a tabela de relativos
  - Diferenciar quando temm begin ou não
  - Testar */