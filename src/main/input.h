#ifndef INPUT_H
#define INPUT_H

#include "board.h"
#include "tile.h"
#include <string>
#include <vector>

class Input {
public:
    Board inputFromFile(std::string fileName);
    void testOutput();
    Board mergePrecomputedOutput(Board baseBoard, const std::string &outputFile);

private:
    size_t rows;
    size_t columns;
    size_t numTrees = 0;
    std::vector<size_t> rowTents;
    std::vector<size_t> columnTents;
    std::vector<std::vector<Tile>> boardTiles;
};

#endif
