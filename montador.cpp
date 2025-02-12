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
  bool is_extern = false;
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
      if (!(c >= '0' && c <= '9' || c >= 'A' && c <= 'F'))
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
      if (!(c >= '0' && c <= '9'))
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
      if (!(c >= '0' && c <= '9'))
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

  map<string, int> labels;

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

      if (line_to_write.size() != 0)
      {
        new_line = true;
      }

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
      for (char &c : word)
      {
        c = toupper(static_cast<unsigned char>(c));
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
          labels[word.substr(0, word.length() - 1)] = 0;
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
        else if (word == "BEGIN" || word == "END" || word == "EXTERN")
        {
          line_to_write += word + " ";
          can_write = true;
        }
        else if (word == "PUBLIC")
        {
          line_to_write += word + " ";
          has_argument = true;
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
  bool last_was_extern = false;
  bool last_was_public = false;
  bool cant_sum_address = false;
  bool have_instruction_in_this_line = false;

  int new_line = 0;
  int line_number = 0;
  int arguments_counter = 2;
  int address = 0;

  string line_to_read = "";
  string line_to_write = "";
  string last_label = "";
  string real = "";

  map<string, info_symbol_table> symbol_table;
  vector<pair<string, int>> uses_table;
  vector<string> definition_table;

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
      send_error(line_number--, "Falta argumento.", input_file_name);
    }

    if (line_to_write.size() != 0)
    {
      new_line = true;
      have_label_in_this_line = false;
    }

    if (last_was_extern)
    {
      last_was_extern = false;
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
      have_label_in_this_line = false;
    }

    istringstream line_readed(line_to_read);

    string word;

    have_instruction_in_this_line = false;

    while (line_readed >> word)
    {
      if (word.empty())
      {
        break;
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
        last_label = word.substr(0, word.length() - 1);
      }
      else if (!have_instruction_in_this_line && (instructions.find(word) != instructions.end() || word == "CONST"))
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
          line_to_write += instructions[word] + " ";
          real += "011";
        }
        else if (word == "STOP")
        {
          line_to_write += instructions[word] + " ";
          real += "0";
        }
        else if (word != "CONST")
        {
          line_to_write += instructions[word] + " ";
          real += "01";
        }
        else
        {
          last_was_const = true;
          real += "0";
        }

        arguments_counter--;

        if (word == "STOP")
        {
          arguments_counter = 0;
          has_argument = true;
        }

        have_instruction_in_this_line = true;
      }
      else if (word == "SPACE")
      {
        line_to_write += "00 ";
        arguments_counter -= 2;
        has_argument = true;
        real += "0";
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
            if (symbol_table[before_comma].defined && !symbol_table[before_comma].is_extern)
            {
              line_to_write += to_string(symbol_table[before_comma].value) + " ";
            }
            else if (symbol_table[before_comma].is_extern)
            {
              uses_table.push_back(make_pair(before_comma, address));
              symbol_table[before_comma].addresses_to_correct.push_back(address);
              line_to_write += "00 ";
            }
            else
            {
              symbol_table[before_comma].addresses_to_correct.push_back(address);
              line_to_write += "00 ";
            }
          }
          else
          {
            symbol_table[before_comma] = {false, -1, {address}};
            line_to_write += "00 ";
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
            if (symbol_table[after_comma].defined && !symbol_table[after_comma].is_extern)
            {
              line_to_write += to_string(symbol_table[after_comma].value) + " ";
            }
            else if (symbol_table[after_comma].is_extern)
            {
              uses_table.push_back(make_pair(after_comma, address));
              symbol_table[after_comma].addresses_to_correct.push_back(address);
              line_to_write += "00 ";
            }
            else
            {
              symbol_table[after_comma].addresses_to_correct.push_back(address);
              line_to_write += "00 ";
            }
          }
          else
          {
            symbol_table[after_comma] = {false, -1, {address}};
            line_to_write += "00 ";
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
            if (symbol_table[word].is_extern)
            {
              line_to_write += "00 ";
            }
            else
            {
              if (symbol_table[word].defined)
              {
                line_to_write += to_string(symbol_table[word].value) + " ";
              }
              else
              {
                symbol_table[word].addresses_to_correct.push_back(address);
                line_to_write += "0 ";
              }
            }
          }
          else
          {
            symbol_table[word] = {false, -1, {address}};
            line_to_write += "0 ";
          }
        }
        has_argument = true;
        arguments_counter--;
        last_was_const = false;
      }
      else if (word == "EXTERN")
      {
        symbol_table[last_label].is_extern = true;

        last_was_extern = true;
        line_to_write.clear();
      }
      else if (word == "PUBLIC")
      {
        last_was_public = true;
        line_to_write.clear();
      }
      else if (last_was_public)
      {
        definition_table.push_back(word);
        last_was_public = false;
        cant_sum_address = true;
        line_to_write.clear();
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

      if (symbol_table.find(word) != symbol_table.end() && symbol_table[word].is_extern)
      {
        uses_table.push_back(make_pair(word, address));
      }

      /* Se não for label e const e begin e end, soma o endereço */
      if (word[word.length() - 1] != ':' && word != "CONST" && word != "BEGIN" && word != "END" && word != "EXTERN" && word != "PUBLIC" && !cant_sum_address)
      {
        address++;
      }

      if (cant_sum_address)
      {
        cant_sum_address = false;
      }
    }
  }

  /* Escrever a última linha */
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

    if (it->second.defined)
    {
      if (!it->second.is_extern)
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
    }
    else
    {
      send_symbol_error("Simbolo nao definido: " + it->first);
    }
  }

  if (have_begin_end)
  {
    /* Escrever tabela de usos */
    output_file << "USO" << endl;

    for (int i = 0; i < uses_table.size(); i++)
    {
      output_file << uses_table[i].first << " " << uses_table[i].second << endl;
    }

    /* Escrever tabela de diretivas */
    output_file << "DEF" << endl;

    for (int i = 0; i < definition_table.size(); i++)
    {
      if (symbol_table.find(definition_table[i]) != symbol_table.end() && symbol_table[definition_table[i]].defined)
      {
        output_file << definition_table[i] << " " << symbol_table[definition_table[i]].value << endl;
      }
      else
      {
        send_symbol_error(definition_table[i] + " nao esta definido.");
      }
    }

    output_file << endl;

    /* Escrever tabela de diretivas */
    output_file << "REAL" << endl;
    output_file << real << endl;
    output_file << endl;
  }


  /* Escrever código fonte */
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

  if (argument_3.substr(argument_3.length() - 4) != ".asm" && argument_3.substr(argument_3.length() - 4) != ".pre")
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
    output_file_name = argument_3.substr(0, argument_3.length() - 4) + ".pre";
    pre_processor(argument_3, output_file_name);
  }
  else
  {
    output_file_name = argument_3.substr(0, argument_3.length() - 4) + ".obj";
    assembler(argument_3, output_file_name);
  }

  return 0;
}
