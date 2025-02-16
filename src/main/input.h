#ifndef INPUT_H
#define INPUT_H

#include <string>
#include <vector>
class Input {
public:
    bool inputFromFile(std::string fileName);

private:
    int rows;
    int columns;
    std::vector<int> rowTents;
    std::vector<int> columnTents;
    std::vector<std::vector<char>> gridRows;
};

#endif
