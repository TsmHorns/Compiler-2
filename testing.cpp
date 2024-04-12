#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <string>
#include <algorithm>

std::map<std::string, int> variables;

void processAssignment(const std::string& line) {
    std::istringstream stream(line);
    std::string identifier;
    char equals;
    int value;
    stream >> identifier >> equals >> value;
    variables[identifier] = value;
}

void processPrint(const std::string& line) {
    size_t start = line.find('(') + 1;
    size_t end = line.find_last_of(')');
    std::string content = line.substr(start, end - start);
    std::istringstream stream(content);
    std::string text, comma, identifier;
    stream >> text >> comma >> identifier;
    text.pop_back(); // Remove the trailing comma
    std::cout << text << " " << variables[identifier] << std::endl;
}

void processLine(const std::string& line) {
    if (line.find("print") != std::string::npos) {
        processPrint(line);
    } else {
        processAssignment(line);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input.py>\n";
        return 1;
    }

    std::string input_file = argv[1];
    std::ifstream input(input_file);
    if (!input) {
        std::cerr << "Could not open input file: " << input_file << "\n";
        return 1;
    }

    std::string line;
    while (std::getline(input, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') continue;
        processLine(line);
    }

    return 0;
}
