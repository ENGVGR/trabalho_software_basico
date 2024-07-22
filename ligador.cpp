#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>


// Step 1: Define a structure to hold name-value pairs
struct NameValuePair {
    std::string name;
    int value;
};

// Define an enum with more than two states
enum FileState {
    INITIAL,
    USO,
    DEF,
    REAL,
    CODE
};

int main(int argc, char* argv[])
{
    std::vector<NameValuePair> tu;
    std::vector<NameValuePair> tgd;
    std::string line;
    std::vector<int> codigo;
    std::vector<int> relatives;
    std::vector<int> fator;
    // Check if the correct number of arguments (file names) are passed
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <objFile1> <objFile2>" << std::endl;
        return 1;
    }
    int contador = 0;
    // Process each .obj file
    for (int i = 1; i <= 2; ++i) {
        std::ifstream objFile(argv[i]);

        if (!objFile.is_open()) {
            std::cerr << "Error opening file: " << argv[i] << std::endl;
            return 1;
        }

        // Here, you can process the file as needed
        std::string line;
        fator.push_back(codigo.size());
        FileState state = INITIAL;
        while (getline(objFile, line)) {
            if (line.empty()) {
                continue;
            }

            // Determine the state of the file
            if (line == "USO") {
                state = USO; // Set the state to USO
                continue;
            } else if (line == "DEF"){
                state = DEF; // Set the state to DEF
                continue;
            } else if (line == "REAL"){
                state = REAL; // Set the state to REAL
                continue;
            }
            

            if(state == USO){
                std::stringstream ss(line);
                std::string name;
                int value;
                ss >> name >> value;
                tu.push_back({name, value + fator[contador]});
            } else if(state == DEF){
                std::stringstream ss(line);
                std::string name;
                int value;
                ss >> name >> value;
                tgd.push_back({name, value + fator[contador]});
            } else if(state == REAL){
                for(char ch : line) {
                    int number = ch - '0'; // Convert character to integer
                    relatives.push_back(number); // Add the number to the vector
                }
                state = CODE;
            } else if(state == CODE){
                std::stringstream ss(line);
                int number;
                    while (ss >> number) {
                    codigo.push_back(number); // Add the number to the vector
                }
            }
            
        }
        contador++;
        objFile.close();

    }
    for(int i = 0; i < tu.size(); i++){
        std::string variable = tu[i].name; 
        for(int j = 0; j< tgd.size(); j++){
            if(variable == tgd[j].name){
                codigo[tu[i].value] += tgd[j].value;
                relatives[tu[i].value] = 0;
            }
        } }
    for(int i = 0; i < relatives.size(); i++){
        if(relatives[i] == 1){
            if (i<=fator[1]){
                codigo[i] += fator[0];
            } else {
            codigo[i] += fator[1];
            }
        }
    }

    std::ofstream objFile("output.e");
    std::string saida;
    for(int i = 0; i < codigo.size(); i++){
        if(codigo[i]<=9 && codigo[i]>=0){
            saida = "0" + std::to_string(codigo[i]);
        } else {
            saida = std::to_string(codigo[i]);
        }
        objFile << saida << " ";
    }
    objFile.close();
    return 0;
}