#include "board.h"
#include <cmath>
#include <unordered_set>
#include <cstdlib>
#include <ctime>

Board::Board(size_t rowCount, size_t colCount, std::vector<size_t> rowTentNum, std::vector<size_t> colTentNum, std::vector<std::vector<Tile>> board, size_t numTrees){
    srand(time(NULL));
    this->rowCount = rowCount;
    this->colCount = colCount;
    this->rowTentNum = rowTentNum;
    this->colTentNum = colTentNum;
    this->board = board;
    this->numTrees = numTrees;
}

bool Board::placeTent() {
    // Calculate the number of available spaces (excluding trees and already placed tents)
    int availableSpaces = (rowCount * colCount) - numTrees - tents.size();

    // Iterate through the board
    for (auto& row : board) { // Use reference (&) to modify board in-place
        for (auto& tile : row) { // Also use reference for tiles

            // Check if there are available spaces and if the tile is empty (NONE)
            if (availableSpaces > 0 && tile.getType() == Type::NONE) {
                
                // Probability check: 1 / availableSpaces chance to place a tent
                if ((std::rand() % availableSpaces) == 0) {
                    
                    tile.setType(Type::TENT); // Place the tent

                    int tileCol = tile.getCoord().getCol();
                    int tileRow = tile.getCoord().getRow();
                    std::vector<Tile> treeTiles; // Store nearby trees

                    // Check for adjacent trees in 4 directions (Left, Right, Up, Down)
                    if (tileCol - 1 >= 0 && board[tileRow][tileCol - 1].getType() == Type::TREE) {
                        treeTiles.push_back(board[tileRow][tileCol - 1]);
                    }
                    if (tileCol + 1 < colCount && board[tileRow][tileCol + 1].getType() == Type::TREE) {
                        treeTiles.push_back(board[tileRow][tileCol + 1]);
                    }
                    if (tileRow - 1 >= 0 && board[tileRow - 1][tileCol].getType() == Type::TREE) {
                        treeTiles.push_back(board[tileRow - 1][tileCol]);
                    }
                    if (tileRow + 1 < rowCount && board[tileRow + 1][tileCol].getType() == Type::TREE) {
                        treeTiles.push_back(board[tileRow + 1][tileCol]);
                    }

                    // Choose the best tree (one with the least tents around it)
                    Tile bestTree = treeTiles.empty() ? tile : treeTiles[0];

                    for (Tile& tree : treeTiles) {
                        if (tree.getNumTent() < bestTree.getNumTent()) {
                            bestTree = tree;
                        }
                    }

                    // Set tent direction based on the best tree found
                    if (treeTiles.empty()) {
                        tile.setDir('X'); // No tree found, mark with 'X'
                    } else {
                        if (bestTree.getCoord() == Coord(tileRow - 1, tileCol)) tile.setDir('L'); // Left
                        if (bestTree.getCoord() == Coord(tileRow + 1, tileCol)) tile.setDir('R'); // Right
                        if (bestTree.getCoord() == Coord(tileRow, tileCol - 1)) tile.setDir('U'); // Up
                        if (bestTree.getCoord() == Coord(tileRow, tileCol + 1)) tile.setDir('D'); // Down
                    }

                    // Add the tent to the tent set
                    tents.insert(Coord(tileRow, tileCol));
                    
                    checkViolations(); // Get updated violation count

                    return true; // Exit after placing one tent
                }

                availableSpaces--; // Reduce available spaces count
            }
        }
    }

    return false; // No tent placed
}


bool Board::removeTent() {
    // Check if there are any tents to remove
    if (tents.empty()) {
        return false; // No tents available to remove
    }

    // Select a random tent to remove
    int randCoord = std::rand() % tents.size();
    auto it = tents.begin();
    std::advance(it, randCoord);

    // Get tent coordinates
    int currentRow = it->getRow();
    int currentCol = it->getCol();

    // Get tree direction associated with this tent
    char treeDir = board[currentRow][currentCol].getDir();

    // Reduce the tent count for the associated tree (if any)
    switch (treeDir) {
        case 'U': // Tent was linked to a tree above
            if (currentRow - 1 >= 0) {
                board[currentRow - 1][currentCol].setNumTent(board[currentRow - 1][currentCol].getNumTent() - 1);
            }
            break;
        case 'D': // Tent was linked to a tree below
            if (currentRow + 1 < rowCount) {
                board[currentRow + 1][currentCol].setNumTent(board[currentRow + 1][currentCol].getNumTent() - 1);
            }
            break;
        case 'L': // Tent was linked to a tree on the left
            if (currentCol - 1 >= 0) {
                board[currentRow][currentCol - 1].setNumTent(board[currentRow][currentCol - 1].getNumTent() - 1);
            }
            break;
        case 'R': // Tent was linked to a tree on the right
            if (currentCol + 1 < colCount) {
                board[currentRow][currentCol + 1].setNumTent(board[currentRow][currentCol + 1].getNumTent() - 1);
            }
            break;
        case 'X': // Tent was not linked to any tree
        default:
            break;
    }

    // Remove the tent from the board
    board[currentRow][currentCol].setType(Type::NONE);

    // Remove tent from the tent set
    tents.erase(it);

    // Recalculate board violations
    checkViolations();

    return true;
}

bool Board::moveTent(){}

double Board::fitnessFunction(double averageViolations){
    return averageViolations - (double)violations;
}

size_t Board::countRowColViolations(){
    size_t violations = 0;

    for(size_t i = 0; i < rowCount; i++){
        // Get the total amount of tents in the current row
        size_t sum = 0;
        for(auto it = tents.begin(); it != tents.end(); it++){
            // Add to sum of tents in row if it equals the current row.
            sum += ((*it).getRow() == i) ? 1 : 0;
        }
        violations += abs(rowTentNum[i] - sum);
    }

    for(size_t i = 0; i < colCount; i++){
        // Get the total amount of tents in the current row
        size_t sum = 0;
        for(auto it = tents.begin(); it != tents.end(); it++){
            sum += ((*it).getCol() == i) ? 1 : 0;
        }
        violations += abs(colTentNum[i] - sum);
    }

    return violations;
}

size_t Board::countTentViolations(){
    const Coord DIRS[8] = {
        Coord(-1, -1), Coord(-1, 0), Coord(-1, 1),
        Coord(0, -1) ,                Coord(0, 1),
        Coord(1, -1) ,  Coord(1, 0),  Coord(1, 1)
    };

    size_t violations = 0;
    for(auto it = tents.begin(); it != tents.end(); it++){
        for(const Coord& dir : DIRS){
            Coord adjacent = Coord((*it).getRow() + dir.getRow(), (*it).getCol() + dir.getCol());
            if(tents.find(adjacent) != tents.end()){
                violations++;
                break;
            }
        }
    }

    return violations;
}

size_t Board::countTreeViolations(){
    // 0 indexed
    std::unordered_map<Coord, int> trees;
    for(auto it = tents.begin(); it != tents.end(); it++){
        int currentRow = (*it).getRow(); int currentCol = (*it).getCol();
        Tile currentTile = board[currentRow][currentCol];
        char dir = currentTile.getDir();
        if(dir == 'X') {
            continue;
        }
        Coord tentCoord = Coord(currentRow, currentCol);
        Coord treeCoord;
        switch(dir){
            case 'U':
                treeCoord.setRow(tentCoord.getRow() - 1);
                treeCoord.setCol(tentCoord.getCol());
                break;
            case 'D':
                treeCoord.setRow(tentCoord.getRow() + 1);
                treeCoord.setCol(tentCoord.getCol());
                break;
            case 'L':
                treeCoord.setRow(tentCoord.getRow());   
                treeCoord.setCol(tentCoord.getCol() - 1);
                break;
            case 'R':
                treeCoord.setRow(tentCoord.getRow());
                treeCoord.setCol(tentCoord.getCol() + 1);
                break;
            default:
                return 0;
        }
        trees[treeCoord] = 1 + trees[treeCoord];
    }
    size_t violations = 0;
    for(size_t i = 0; i < rowCount; i++){
        for(size_t j = 0; j < colCount; j++){
            if(board[i][j].getType() == Type::TREE){
                int tree_count = trees[Coord(i, j)];
                if(tree_count != 1){
                    violations++;
                }
            }
        }
    }
    return violations;

}

size_t Board::checkViolations(){
    size_t violations = 0;

    violations += countRowColViolations();
    violations += countTentViolations();
    violations += countTreeViolations();

    for(auto it = tents.begin(); it != tents.end(); it++){
        if(board[(*it).getRow()][(*it).getCol()].getDir() == 'X'){
            violations++;
        }
    }

    this->violations = violations;
    return violations;
}
