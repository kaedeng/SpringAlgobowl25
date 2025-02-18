#ifndef INPUT_H
#define INPUT_H

#include "tile.h"
#include <string>
#include <vector>

class Input {
public:
    void inputFromFile(std::string fileName);
    void testOutput();

private:
    size_t rows;
    size_t columns;
    std::vector<size_t> rowTents;
    std::vector<size_t> columnTents;
    std::vector<std::vector<Tile>> boardTiles;
};

#endif
