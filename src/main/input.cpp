#include "input.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>

Board Input::inputFromFile(std::string fileName) {
    
    rowTents.clear();
    columnTents.clear();
    boardTiles.clear();

    std::ifstream file(fileName);

    if (!file) {
        throw std::runtime_error("Invalid input: rows and columns must be positive integers");
    }

    std::string line;
    std::vector<std::string> lines;

    while (std::getline(file, line)) {
        lines.push_back(line);
    }

    std::istringstream iss1(lines[0]);

    // Try to read two numbers from the stream.
    if (!(iss1 >> rows >> columns)) {
        throw std::runtime_error("Invalid input: R and C must be positive integers");
    }

    // Ensure that there are no extra characters (other than whitespace) after the numbers.
    std::string extra1;
    if (iss1 >> extra1) {
        throw std::runtime_error("Invalid input: extra characters detected after R and C");
    }

    // Check if R and C are not equal to zero.
    if (rows == 0 || columns == 0) {
        throw std::runtime_error("Invalid input: R and C must be greater than zero");
    }

    // Check if R * C is greater than 10^5, which would be invalid.
    if (rows * columns > 100000) {
        throw std::runtime_error("Invalid input: R * C exceeds 10^5");
    }

    // rowTents
    std::istringstream iss2(lines[1]);
    size_t tempNum;

    for (size_t i = 0; i < rows; ++i) {
        if (!(iss2 >> tempNum)) {
            throw std::runtime_error("Invalid input: r_i value must be a non-negative integer");
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
            throw std::runtime_error("Invalid input: c_i value must be a non-negative integer");
        }

        if (tempNum > rows) {
            throw std::runtime_error("Invalid input: c_i is greater than R");
        }

        columnTents.push_back(tempNum);
    }

    for (size_t i = 0; i < rows; ++i) {
        std::vector<Tile> tempVector;
        for (size_t j = 0; j < columns; ++j) {
            if (lines[i+3][j] == '.') {
                tempVector.push_back(Tile(Type::NONE, i, j));
            }
            else if (lines[i+3][j] == 'T') {
                tempVector.push_back(Tile(Type::TREE, i, j));
                ++numTrees;
            }
            else {
                throw std::runtime_error("Invalid input: Tile is not '.' or 'T'");
            }
        }
        boardTiles.push_back(tempVector);
    }

    file.close();

    return Board(rows, columns, rowTents, columnTents, boardTiles, numTrees);
}


void Input::testOutput() {

    std::cout << rows << ' ' << columns << '\n';

    for (size_t i : rowTents) {
        std::cout << i << ' ';
    }
    std::cout << '\n';

    for (size_t i : columnTents) {
        std::cout << i << ' ';
    }
    std::cout << '\n';

    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < columns; ++j) {
            std::cout << (int)(boardTiles[i][j].getType());
        }
        std::cout << '\n';
    }
}
