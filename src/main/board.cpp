#include "board.h"
#include <cmath>
#include <unordered_set>
#include <cstdlib>
#include <ctime>
#include <iostream>

/*
/////////////////////////////////////////////////////////////////////////////
Helper Functions (Add more if needed)
/////////////////////////////////////////////////////////////////////////////
*/

bool Board::hasAdjacentTent(const Coord &coord) {
    const Coord DIRS[8] = {
        Coord(-1, -1), Coord(-1, 0), Coord(-1, 1),
        Coord(0, -1),                Coord(0, 1),
        Coord(1, -1),  Coord(1, 0),  Coord(1, 1)
    };
    
    for (auto &dir : DIRS) {
        Coord adjacent(coord.getRow() + dir.getRow(), coord.getCol() + dir.getCol());
        if (tents.find(adjacent) != tents.end())
            return true;
    }
    return false;
}

void Board::updateTentAdjacencyForCoord(const Coord &coord) {
    // Update the current tent's violation status.
    bool prevStatus = tentAdjViolation[coord];
    bool newStatus = hasAdjacentTent(coord);
    if (prevStatus != newStatus) {
        tentViolations += (newStatus ? 1 : -1);
        tentAdjViolation[coord] = newStatus;
    }

    // Define all 8 adjacent directions.
    const Coord DIRS[8] = {
        Coord(-1, -1), Coord(-1, 0), Coord(-1, 1),
        Coord(0, -1),                Coord(0, 1),
        Coord(1, -1),  Coord(1, 0),  Coord(1, 1)
    };

    // Update each neighboring tent's violation status.
    for (auto &dir : DIRS) {
        Coord adjacent(coord.getRow() + dir.getRow(), coord.getCol() + dir.getCol());
        if (tents.find(adjacent) != tents.end()) {
            bool prevAdjStatus = tentAdjViolation[adjacent];
            bool newAdjStatus = hasAdjacentTent(adjacent);
            if (prevAdjStatus != newAdjStatus) {
                tentViolations += (newAdjStatus ? 1 : -1);
                tentAdjViolation[adjacent] = newAdjStatus;
            }
        }
    }
}

/*
/////////////////////////////////////////////////////////////////////////////
Actual functions
/////////////////////////////////////////////////////////////////////////////
*/

Board::Board(
    size_t rowCount, 
    size_t colCount,
    std::vector<size_t> rowTentNum,
    std::vector<size_t> colTentNum,
    std::vector<std::vector<Tile>> board,
    size_t numTrees
    ){

    srand(time(NULL));
    this->rowCount = rowCount;
    this->colCount = colCount;
    this->rowTentNum = rowTentNum;
    this->colTentNum = colTentNum;
    this->board = board;
    this->numTrees = numTrees;

    currentRowTents.assign(rowCount, 0);
    currentColTents.assign(colCount, 0);

    rowViolations = 0;
    colViolations = 0;
    tentViolations = 0;
    treeViolations = 0;
    invalidTentViolations = 0;
    violations = 0;
    
    // parse through the whole board
    for(size_t i = 0; i < rowCount; i++){
        for(size_t j = 0; j < colCount; j++){
            // If the current tile is a tree
            if (board[i][j].getType() == Type::TREE) {
                treeTentCount[Coord(i, j)] = 0;
                // Tree starts at 0 tents, will be lonely for valentines...
                treeViolations++;
            }

            // If the current tile is a tent
            if(board[i][j].getType() == Type::TENT){
                Coord coord = board[i][j].getCoord();
                if (tents.find(coord) == tents.end()) {
                    tents.insert(coord);
                    currentRowTents[i]++;
                    currentColTents[j]++;

                    char dir = board[i][j].getDir(); // Check the direction of the tent
                    if (dir != 'X') {
                        // Update the associated tree’s count.
                        Coord treeCoord = coord; // should be a shallow copy, but we aren't using the old coord anymore, so its fine.
                        switch (dir) {
                            case 'U': treeCoord.setRow(i - 1); break;
                            case 'D': treeCoord.setRow(i + 1); break;
                            case 'L': treeCoord.setCol(j - 1); break;
                            case 'R': treeCoord.setCol(j + 1); break;
                            default: break;
                        }

                        int oldCount = treeTentCount[treeCoord];
                        treeTentCount[treeCoord] = oldCount + 1;
                        if (oldCount == 0)
                            treeViolations--;    // Tree now has exactly one tent (resolved violation)
                        else if (oldCount == 1)
                            treeViolations++;    // Tree now has too many tents
                    } else {
                        invalidTentViolations++; // Lonely tent (womp)
                    }

                    // Check if this tent has any adjacent tent already.
                    updateTentAdjacencyForCoord(coord);
                }
            }
        }
    }

    // Compute row and column violations based on initial counts.
    for (size_t i = 0; i < rowCount; i++) {
        rowViolations += std::abs((int)rowTentNum[i] - (int)currentRowTents[i]);
    }
    for (size_t j = 0; j < colCount; j++) {
        colViolations += std::abs((int)colTentNum[j] - (int)currentColTents[j]);
    }
    violations = rowViolations + colViolations + tentViolations + treeViolations + invalidTentViolations;

}


bool Board::placeTent() {
      // Calculate available spaces (total tiles minus trees and already placed tents)
      int availableSpaces = (rowCount * colCount) - numTrees - (int)tents.size();

      for (auto &row : board) {
          for (auto &tile : row) {
              if (availableSpaces > 0 && tile.getType() == Type::NONE) {
                  // With probability 1/availableSpaces, choose this tile.
                  if ((std::rand() % availableSpaces) == 0) {
                      int r = tile.getCoord().getRow();
                      int c = tile.getCoord().getCol();
                      // Place tent
                      tile.setType(Type::TENT);
  
                      // Update row counts and corresponding violation difference.
                      int oldRowCount = currentRowTents[r];
                      currentRowTents[r]++;
                      int oldRowViol = abs((int)(rowTentNum[r] - oldRowCount));
                      int newRowViol = abs((int)(rowTentNum[r] - currentRowTents[r]));
                      rowViolations += (newRowViol - oldRowViol);
  
                      // Update column counts.
                      int oldColCount = currentColTents[c];
                      currentColTents[c]++;
                      int oldColViol = abs((int)(colTentNum[c] - oldColCount));
                      int newColViol = abs((int)(colTentNum[c] - currentColTents[c]));
                      colViolations += (newColViol - oldColViol);
  
                      Coord newCoord(r, c);
                      tents.insert(newCoord);
  
                      // Check for adjacent tent violations.
                      bool newTentAdj = hasAdjacentTent(newCoord);
                      tentAdjViolation[newCoord] = newTentAdj;
                      if (newTentAdj)
                          tentViolations++;
  
                      // Update neighbors’ adjacency status.
                      const Coord DIRS[8] = {
                          Coord(-1, -1), Coord(-1, 0), Coord(-1, 1),
                          Coord(0, -1),              Coord(0, 1),
                          Coord(1, -1),  Coord(1, 0),  Coord(1, 1)
                      };
                      for (auto &dir : DIRS) {
                          Coord neighbor(r + dir.getRow(), c + dir.getCol());
                          if (tents.find(neighbor) != tents.end())
                              updateTentAdjacencyForCoord(neighbor);
                      }
  
                      // Choose an associated tree.
                      std::vector<Tile> treeTiles;
                      if (c - 1 >= 0 && board[r][c - 1].getType() == Type::TREE)
                          treeTiles.push_back(board[r][c - 1]);
                      if (c + 1 < colCount && board[r][c + 1].getType() == Type::TREE)
                          treeTiles.push_back(board[r][c + 1]);
                      if (r - 1 >= 0 && board[r - 1][c].getType() == Type::TREE)
                          treeTiles.push_back(board[r - 1][c]);
                      if (r + 1 < rowCount && board[r + 1][c].getType() == Type::TREE)
                          treeTiles.push_back(board[r + 1][c]);
  
                      Tile bestTree = treeTiles.empty() ? tile : treeTiles[0];
                      for (Tile &tree : treeTiles) {
                          if (tree.getNumTent() < bestTree.getNumTent())
                              bestTree = tree;
                      }
  
                      // If no tree found, mark tent as invalid.
                      if (treeTiles.empty()) {
                          tile.setDir('X');
                          invalidTentViolations++;
                      } else {
                          // Follow original logic:
                          // If bestTree is above, below, left, or right, set the direction accordingly.
                          // (For example, if bestTree is at (r-1, c), we set dir to 'L', etc.)
                          if (bestTree.getCoord() == Coord(r - 1, c))
                              tile.setDir('L');
                          else if (bestTree.getCoord() == Coord(r + 1, c))
                              tile.setDir('R');
                          else if (bestTree.getCoord() == Coord(r, c - 1))
                              tile.setDir('U');
                          else if (bestTree.getCoord() == Coord(r, c + 1))
                              tile.setDir('D');
  
                          // Update the associated tree’s counter.
                          Coord assocTree;
                          switch (tile.getDir()) {
                              case 'L': assocTree = Coord(r - 1, c); break;
                              case 'R': assocTree = Coord(r + 1, c); break;
                              case 'U': assocTree = Coord(r, c - 1); break;
                              case 'D': assocTree = Coord(r, c + 1); break;
                              default: break;
                          }
                          int oldCount = treeTentCount[assocTree];
                          treeTentCount[assocTree] = oldCount + 1;
                          if (oldCount == 0)
                              treeViolations--;    // Tree now valid
                          else if (oldCount == 1)
                              treeViolations++;    // Now too many tents
                      }
                      // Update overall violation count.
                      violations = rowViolations + colViolations + tentViolations + treeViolations + invalidTentViolations;
                      return true;
                  }
                  availableSpaces--;
              }
          }
      }
      return false;
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

    // Get tent coords
    Coord coord = *it;
    int r = coord.getRow();
    int c = coord.getCol();

    board[r][c].setType(Type::NONE);

    // Update row counts.
    int oldRowCount = currentRowTents[r];
    currentRowTents[r]--;
    int oldRowViol = abs((int)(rowTentNum[r] - oldRowCount));
    int newRowViol = abs((int)(rowTentNum[r] - currentRowTents[r]));
    rowViolations += (newRowViol - oldRowViol);

    // Update column counts.
    int oldColCount = currentColTents[c];
    currentColTents[c]--;
    int oldColViol = abs((int)(colTentNum[c] - oldColCount));
    int newColViol = abs((int)(colTentNum[c] - currentColTents[c]));
    colViolations += (newColViol - oldColViol);

    // Remove tent’s adjacent violation status and update neighbors.
    if (tentAdjViolation[coord])
        tentViolations--;
    tentAdjViolation.erase(coord);
    const Coord DIRS[8] = {
        Coord(-1, -1), Coord(-1, 0), Coord(-1, 1),
        Coord(0, -1),                 Coord(0, 1),
        Coord(1, -1),   Coord(1, 0),  Coord(1, 1)
    };
    for (auto &dir : DIRS) {
        Coord neighbor(r + dir.getRow(), c + dir.getCol());
        if (tents.find(neighbor) != tents.end())
            updateTentAdjacencyForCoord(neighbor);
    }

    // Update tree or invalid-tent violation counts.
    char dirChar = board[r][c].getDir();
    if (dirChar != 'X') {
        Coord assocTree;
        switch (dirChar) {
            case 'L': assocTree = Coord(r - 1, c); break;
            case 'R': assocTree = Coord(r + 1, c); break;
            case 'U': assocTree = Coord(r, c - 1); break;
            case 'D': assocTree = Coord(r, c + 1); break;
            default: break;
        }
        int oldCount = treeTentCount[assocTree];
        treeTentCount[assocTree] = oldCount - 1;
        if (oldCount == 1)
            treeViolations++;   // Now 0 tents: violation appears.
        else if (oldCount == 2)
            treeViolations--;   // Now exactly one: violation resolved.
    } else {
        invalidTentViolations--;
    }

    return true;
}

bool Board::moveTent(){}

double Board::fitnessFunction(double averageViolations){
    return averageViolations - (double)violations;
}

/*
size_t Board::countRowColViolations(){
    size_t violations = 0;

    for(size_t i = 0; i < rowCount; i++){
        // Get the total amount of tents in the current row
        size_t sum = 0;
        for(auto it = tents.begin(); it != tents.end(); it++){
            // Add to sum of tents in row if it equals the current row.
            sum += (it->getRow() == i) ? 1 : 0;
        }
        violations += abs((int)(rowTentNum[i] - sum));
    }

    for(size_t i = 0; i < colCount; i++){
        // Get the total amount of tents in the current row
        size_t sum = 0;
        for(auto it = tents.begin(); it != tents.end(); it++){
            sum += (it->getCol() == i) ? 1 : 0;
        }
        violations += abs((int)(colTentNum[i] - sum));
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
            Coord adjacent = Coord(it->getRow() + dir.getRow(), it->getCol() + dir.getCol());
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
        int currentRow = it->getRow(); int currentCol = it->getCol();
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
*/

size_t Board::checkViolations(){
    return violations;
}

/*
/////////////////////////////////////////////////////////////////////////////
Getters and Setters (Add more if needed)
/////////////////////////////////////////////////////////////////////////////
*/

Tile Board::getTile(size_t row, size_t col) const{
    return board[row][col];
}

Tile Board::setTile(Tile &tile, size_t row, size_t col){
    board[row][col] = tile;
}

void Board::debugPrintViolations(){
    std::cout << "\nRow Violations: " << rowViolations << std::endl;
    std::cout << "Column Violations: " << colViolations << std::endl;
    std::cout << "Tent Adjacency Violations: " << tentViolations << std::endl;
    std::cout << "Tree Violations: " << treeViolations << std::endl;
    std::cout << "Invalid Tent Violations: " << invalidTentViolations << std::endl;
    std::cout << "Total Violations: " << violations << std::endl;
}