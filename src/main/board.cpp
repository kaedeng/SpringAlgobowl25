#include "board.h"
#include <cmath>
#include <unordered_set>
#include <cstdlib>
#include <ctime>

Board::Board(size_t rowCount, size_t colCount, std::vector<size_t> rowTentNum, std::vector<size_t> colTentNum, std::vector<std::vector<Tile>> board){
    srand(time(NULL));
    this->rowCount = rowCount;
    this->colCount = colCount;
    this->rowTentNum = rowTentNum;
    this->colTentNum = colTentNum;
    this->board = board;
}

bool Board::placeTent(){}

bool Board::removeTent(){
    int randCoord = std::rand() % tents.size();
    auto it = tents.begin();
    std::advance(it, randCoord);
    int currentRow = (*it).getRow(); int currentCol = (*it).getCol();
    board[currentRow][currentCol].setType(Type::NONE);
    tents.erase(it);
}

bool Board::moveTent(){}

double Board::fitnessFunction(double averageViolations){
    return averageViolations - (double)volations;
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

    return violations;
}
