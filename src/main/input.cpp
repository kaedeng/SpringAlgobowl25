#include "input.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

void Input::inputFromFile(std::string fileName) {
    std::ifstream file(fileName);

    if (!file) {
        throw std::runtime_error("Invalid input: rows and columns must be positive integers.");
    }

    std::string line;
    std::vector<std::string> lines;

    while (std::getline(file, line)) {
        lines.push_back(line);
    }

    // Rows and Columns
    std::istringstream iss1(lines[0]);
    // Try to read two numbers from the stream.
    if (!(iss1 >> rows >> columns)) {
        throw std::runtime_error("Invalid input: R and C must be positive integers.");
    }

    // Ensure that there are no extra characters (other than whitespace) after the numbers.
    std::string extra1;
    if (iss1 >> extra1) {
        throw std::runtime_error("Invalid input: extra characters detected after R and C.");
    }

    // Optionally, if you consider zero not a positive integer, check for that as well.
    if (rows == 0 || columns == 0) {
        throw std::runtime_error("Invalid input: R and C must be greater than zero.");
    }

    // Check if R * C is greater than 10^5, which would be invalid.
    if (rows * columns > 100000) {
        throw std::runtime_error("Invalid input: R * C exceeds 10^5.");
    }

    // rowTents
    /*
    std::istringstream iss2(lines[1]);
    size_t tempNum;
    while (iss2 >> tempNum) {
        rowTents.push_back(tempNum);
    }
    */
    std::istringstream iss2(lines[1]);
    size_t tempNum;

    for (size_t i = 0; i < rows; ++i) {
        if (!(iss2 >> tempNum)) {
            throw std::runtime_error("Invalid input: r_i value must be a non-negative integer.");
        }

        if (tempNum > columns) {
            throw std::runtime_error("Invalid input: r_i is greater than C");
        }

        rowTents.push_back(tempNum);
    }

    // columnTents
    std::istringstream iss3(lines[2]);
    for (size_t i = 0; i < columns; ++i) {
        if (!(iss3 >> tempNum)) {
            throw std::runtime_error("Invalid input: c_i value must be a non-negative integer.");
        }

        if (tempNum > rows) {
            throw std::runtime_error("Invalid input: c_i is greater than R");
        }

        columnTents.push_back(tempNum);
    }

    for (size_t i = 3; i < rows + 3; ++i) {
        std::vector<char> tempVector;
        for (int j = 0; j < columns; ++j) {
            tempVector.push_back(lines[i][j]);
        }

        gridRows.push_back(tempVector);
    }

    file.close();
    return;
}

void Input::testOutput() {
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
}
