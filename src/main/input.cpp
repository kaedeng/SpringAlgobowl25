#include "input.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

bool Input::inputFromFile(std::string fileName) {
    std::ifstream file(fileName);

    if (!file) {
        std::cerr << "u dum dum, no file >:(" << '\n';
        return false;
    }

    std::string line;
    std::vector<std::string> lines;

    while (std::getline(file, line)) {
        lines.push_back(line);
    }

    // Rows and Columns
    std::istringstream iss1(lines[0]);
    iss1 >> rows;
    iss1 >> columns;

    // rowTents
    std::istringstream iss2(lines[1]);
    int tempNum;
    while (iss2 >> tempNum) {
        rowTents.push_back(tempNum);
    }

    // columnTents
    std::istringstream iss3(lines[2]);
    while (iss3 >> tempNum) {
        columnTents.push_back(tempNum);
    }

    for (int i = 3; i < rows + 3; ++i) {
        std::vector<char> tempVector;
        for (int j = 0; j < columns; ++j) {
            tempVector.push_back(lines[i][j]);
        }

        gridRows.push_back(tempVector);
    }

    std::cout << rows << ' ' << columns << '\n';

    for (int i : rowTents) {
        std::cout << i << ' ';
    }
    std::cout << '\n';

    for (int i : columnTents) {
        std::cout << i << ' ';
    }
    std::cout << '\n';

    for (int i = 0; i < gridRows.size(); ++i) {
        for (int j = 0; j < gridRows[i].size(); ++j) {
            std::cout << gridRows[i][j];
        }
        std::cout << '\n';
    }

    file.close();
    return true;
}
