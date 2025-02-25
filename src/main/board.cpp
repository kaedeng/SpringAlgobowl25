#include "board.h"
#include <cmath>
#include <unordered_set>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <algorithm>
#include <random>
#include <iomanip>

/*
/////////////////////////////////////////////////////////////////////////////
Helper Functions (Add more if needed)
/////////////////////////////////////////////////////////////////////////////
*/

// Get all adjacent tents from a given coord, roughly O( 8log(n) )
std::unordered_set<Coord> Board::getAdjacentTents(const Coord &coord) const {
    std::unordered_set<Coord> adj; 

    constexpr Coord DIRS[8] = {
        Coord(-1, -1), Coord(-1, 0), Coord(-1, 1),
        Coord(0, -1),                Coord(0, 1),
        Coord(1, -1),  Coord(1, 0),  Coord(1, 1)
    };
    
    for (const Coord& dir : DIRS) {
        Coord adjacent(coord.getRow() + dir.getRow(), coord.getCol() + dir.getCol());
        if (tentTiles.contains(adjacent))
            adj.insert(std::move(adjacent));
    }
    return adj;
}

// Update all tents if they need to. roughly O( 8log(n) + 64log(n) )
void Board::updateTentAdjacencyForCoord(const Coord &coord) {
    std::unordered_set<Coord> adj = getAdjacentTents(coord);

    // If there's still a tent at coord, update its violation status.
    if (board[coord.getRow()][coord.getCol()].getType() == Type::TENT) {
        bool prevStatus = tentAdjViolation[coord];
        bool newStatus = !adj.empty();
        if (prevStatus != newStatus) {
            tentViolations += (newStatus ? 1 : -1);
            tentAdjViolation[coord] = newStatus;
        }
    } else {
        // If the coordinate was marked as violating, remove that violation before erasing.
        if (tentAdjViolation.count(coord) && tentAdjViolation[coord] == true) {
            tentViolations -= 1;
        }
        tentAdjViolation.erase(std::move(coord));
    }

    // Update each neighboring tent's violation status.
    for (const Coord& neighbor : adj) {
        std::unordered_set<Coord> neighborAdj = getAdjacentTents(neighbor);
        bool prevNeighborStatus = tentAdjViolation[neighbor];
        bool newNeighborStatus = !neighborAdj.empty();
        if (prevNeighborStatus != newNeighborStatus) {
            tentViolations += (newNeighborStatus ? 1 : -1);
            tentAdjViolation[neighbor] = newNeighborStatus;
        }
    }
}


void Board::drawBoard() const {
    size_t rows = board.size();
    if (rows == 0) return;
    size_t cols = board[0].size();

    // Print column numbers on top with a 4-space offset for row numbers.
    std::cout << "    ";
    for (size_t j = 0; j < cols; j++) {
        std::cout << std::setw(2) << colTentNum[j] << " ";
    }
    std::cout << std::endl;

    // Print a separating line for clarity.
    std::cout << "   +";
    for (size_t j = 0; j < cols; j++) {
        std::cout << "---";
    }
    std::cout << "+\n";

    // Print each row.
    for (size_t i = 0; i < rows; i++) {
        // Print the row's tent count on the left.
        std::cout << std::setw(2) << rowTentNum[i] << " |";
        for (size_t j = 0; j < cols; j++) {
            char outputChar;
            // Determine which character to display based on the tile type.
            switch (board[i][j].getType()) {
                case Type::NONE:
                    outputChar = '.';
                    break;
                case Type::TREE:
                    outputChar = 'T';
                    break;
                case Type::TENT: {
                    char dir = board[i][j].getDir();
                    // Use lowercase 't' if the direction is 'X', otherwise use the direction character.
                    outputChar = (dir == 'X') ? 't' : dir;
                    break;
                }

            }
            std::cout << " " << outputChar << " ";
        }
        std::cout << "|\n";
    }

    // Print the bottom border.
    std::cout << "   +";
    for (size_t j = 0; j < cols; j++) {
        std::cout << "---";
    }
    std::cout << "+\n";
}

void Board::updateRowAndColForTent (const size_t r, const size_t c, const bool addTent) {
    
    int oldRowCount = currentRowTents[r];
    int oldColCount = currentColTents[c];

    if(addTent){
        currentRowTents[r]++;
        currentColTents[c]++;
    }else{
        currentRowTents[r]--;
        currentColTents[c]--;
    }

    // Update row counts.
    int oldRowViol = abs((int)(rowTentNum[r] - oldRowCount));
    int newRowViol = abs((int)(rowTentNum[r] - currentRowTents[r]));
    rowViolations += (newRowViol - oldRowViol);

    // Update column counts.
    int oldColViol = abs((int)(colTentNum[c] - oldColCount));
    int newColViol = abs((int)(colTentNum[c] - currentColTents[c]));
    colViolations += (newColViol - oldColViol);
}

/*
/////////////////////////////////////////////////////////////////////////////
Actual functions
/////////////////////////////////////////////////////////////////////////////
*/

// O( n + m + n*m * (nlog(n) + 72log(n) ) if theres a shit load of tents lol.
// theta( n + m + n*m * klog(n) ) probably.
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
    lonelyTentViolations = 0;
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
                bitSetTent(coord);
                if (!tentTiles.contains(coord)) {
                    tentTiles.insert(coord);
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
                        lonelyTentViolations++; // Lonely tent (womp)
                    }

                    // Check if this tent has any adjacent tent already.
                    updateTentAdjacencyForCoord(coord);
                }
            }
            if(board[i][j].getType() == Type::NONE){
                openTiles.insert(board[i][j].getCoord());
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
    violations = rowViolations + colViolations + tentViolations + treeViolations + lonelyTentViolations;

}

Board::Board(const Board& other) {
    rowCount = other.getNumRows();
    colCount = other.getNumCols();
    board = other.getBoard();
    rowTentNum = other.getRowTentNum();
    colTentNum = other.getColTentNum();
    currentRowTents = other.getCurrentRowTents();
    currentColTents = other.getCurrentColTents();
    tentTiles = other.getTentTilesData();
    tentAdjViolation = other.getTentAdjViolation();
    treeTentCount = other.getTreeTentCount();
    numTrees = other.getNumTrees();
    rowViolations = other.getRowViolations();
    colViolations = other.getColViolations();
    tentViolations = other.getTentViolations();
    treeViolations = other.getTreeViolations();
    lonelyTentViolations = other.getLonelyTentViolations();
    violations = other.getViolations();
    numTiles = other.getNumTiles();
    openTiles = other.getOpenTilesData();
    bitBoard = other.getBitBoard();
}

Board& Board::operator=(const Board& other) {
    if (this != &other) {
        rowCount = other.getNumRows();
        colCount = other.getNumCols();
        board = other.getBoard();
        rowTentNum = other.getRowTentNum();
        colTentNum = other.getColTentNum();
        currentRowTents = other.getCurrentRowTents();
        currentColTents = other.getCurrentColTents();
        tentTiles = other.getTentTilesData();
        tentAdjViolation = other.getTentAdjViolation();
        treeTentCount = other.getTreeTentCount();
        numTrees = other.getNumTrees();
        rowViolations = other.getRowViolations();
        colViolations = other.getColViolations();
        tentViolations = other.getTentViolations();
        treeViolations = other.getTreeViolations();
        lonelyTentViolations = other.getLonelyTentViolations();
        violations = other.getViolations();
        numTiles = other.getNumTiles();
        openTiles = other.getOpenTilesData();
        bitBoard = other.getBitBoard();
    }
    return *this;
}

bool Board::placeTent(Tile& tile, std::mt19937& gen) {
    int r = tile.getCoord().getRow();
    int c = tile.getCoord().getCol();

    // Place tent
    tile.setType(Type::TENT);
    openTiles.remove(tile.getCoord());
    bitSetTent(tile.getCoord());

    // Update row counts.
    updateRowAndColForTent(r, c, true);

    Coord newCoord(r, c);
    tentTiles.insert(newCoord);

    // Update adjacent tents.
    updateTentAdjacencyForCoord(newCoord);

    // Choose an associated tree.
    std::vector<Tile> treeTiles;
    if (c - 1 >= 0 && board[r][c - 1].getType() == Type::TREE && (treeTentCount[board[r][c - 1].getCoord()] == 0))
        treeTiles.push_back(board[r][c - 1]);
    if (c + 1 < colCount && board[r][c + 1].getType() == Type::TREE && (treeTentCount[board[r][c + 1].getCoord()] == 0))
        treeTiles.push_back(board[r][c + 1]);
    if (r - 1 >= 0 && board[r - 1][c].getType() == Type::TREE && (treeTentCount[board[r - 1][c].getCoord()] == 0))
        treeTiles.push_back(board[r - 1][c]);
    if (r + 1 < rowCount && board[r + 1][c].getType() == Type::TREE && (treeTentCount[board[r + 1][c].getCoord()] == 0))
        treeTiles.push_back(board[r + 1][c]);

    // Shuffle treeTiles to randomize the iteration order.
    std::uniform_int_distribution<> dis(0, treeTiles.size() - 1);
    Tile bestTree = treeTiles.empty() ? tile : treeTiles[0];


    
    // If no tree found, mark tent as invalid.
    if (treeTiles.empty()) {
        tile.setDir('X');
        lonelyTentViolations++;
    } else {
        bestTree = treeTiles[dis(gen)];
        // Follow original logic:
        // If bestTree is above, below, left, or right, set the direction accordingly.
        // (For example, if bestTree is at (r-1, c), we set dir to 'L', etc.)
        Coord assocTree;
        if (bestTree.getCoord() == Coord(r, c - 1)){
            tile.setDir('L');
            assocTree = Coord(r, c - 1);
        }else if (bestTree.getCoord() == Coord(r, c + 1)){
            tile.setDir('R');
            assocTree = Coord(r, c + 1);
        }else if (bestTree.getCoord() == Coord(r - 1, c)){
            tile.setDir('U');
            assocTree = Coord(r - 1, c);
        }else if (bestTree.getCoord() == Coord(r + 1, c)){
            tile.setDir('D');
            assocTree = Coord(r + 1, c);
        }

        // Update tree tent count.
        int oldCount = treeTentCount[assocTree];
        treeTentCount[assocTree] = oldCount + 1;
        if (oldCount == 0)
            treeViolations--;    // Tree now valid
        else if (oldCount == 1)
            treeViolations++;    // Now too many tents
    }
    // Update overall violation count.
    violations = rowViolations + colViolations + tentViolations + treeViolations + lonelyTentViolations;

    return true;
}

bool Board::addTent(std::mt19937 &gen) {

    if(openTiles.size() == 0)
        return false;

    std::uniform_int_distribution<int> dist(0, openTiles.size() - 1);
    std::optional<Coord> coord = openTiles.getTileAtIndex(dist(gen));
    if (coord != std::nullopt) {
        if (placeTent(board[coord.value().getRow()][coord.value().getCol()], gen))
            return true;
    }

    return false;
}

bool Board::removeTent(std::mt19937 &gen) {
    // Check if there are any tents to remove
    if (tentTiles.size() == 0) {
        return false; // No tents available to remove
    }
    
    // Select a random tent to remove
    std::uniform_int_distribution<int> dist(0, static_cast<int>(tentTiles.size()) - 1);
    int randCoord = dist(gen);
    
    deleteTent(tentTiles.getTileAtIndex(randCoord).value());

    return true;
}

bool Board::deleteTent(Coord coord) {

    int r = coord.getRow();
    int c = coord.getCol();

    tentTiles.remove(coord);
    openTiles.insert(coord);
    bitClearTent(coord);

    board[r][c].setType(Type::NONE);

    // Update row counts.
    updateRowAndColForTent(r, c, false);

    // Update tent adjacency.
    updateTentAdjacencyForCoord(coord);

    // Update tree or invalid-tent violation counts.
    char dirChar = board[r][c].getDir();
    if (dirChar != 'X') {
        Coord assocTree;
        switch (dirChar) {
            case 'L': assocTree = Coord(r, c - 1); break;
            case 'R': assocTree = Coord(r, c + 1); break;
            case 'U': assocTree = Coord(r - 1, c); break;
            case 'D': assocTree = Coord(r + 1, c); break;
            default: break;
        }
        int oldCount = treeTentCount[assocTree];
        treeTentCount[assocTree] = oldCount - 1;
        if (oldCount == 1)
            treeViolations++;   // Now 0 tents: violation appears.
        else if (oldCount == 2)
            treeViolations--;   // Now exactly one: violation resolved.
    } else {
        lonelyTentViolations--;
    }

    // Update overall violation count.
    violations = rowViolations + colViolations + tentViolations + treeViolations + lonelyTentViolations;

    return true;

}

bool Board::moveTent(std::mt19937 &gen) {
    if(removeTent(gen)){
        if(addTent(gen))
            return true;
    }
    return false;
}


/*
/////////////////////////////////////////////////////////////////////////////
Getters and Setters (Add more if needed)
/////////////////////////////////////////////////////////////////////////////
*/

void Board::bitSetTent(const Coord& location){
    size_t index = location.getRow() * colCount + location.getCol();
    bitBoard.set(index, true);
}
void Board::bitClearTent(const Coord& location){
    size_t index = location.getRow() * colCount + location.getCol();
    bitBoard.set(index, false);
}
size_t Board::countXorBits(const std::bitset<MAX_BOARD_SIZE>& other) const{
    return (bitBoard ^ other).count();
}
std::bitset<Board::MAX_BOARD_SIZE> Board::getBitBoard() const{
    return bitBoard;
}

void Board::printFullBoardInfo() const {
    // Print board dimensions and tent requirements.
    std::cout << "Board Dimensions: " << rowCount << " rows x " << colCount << " cols\n";
    std::cout << "Row Tent Requirements: ";
    for (auto n : rowTentNum)
        std::cout << n << " ";
    std::cout << "\nColumn Tent Requirements: ";
    for (auto n : colTentNum)
        std::cout << n << " ";
    std::cout << "\n\n";

    // Print current tent counts.
    std::cout << "Current Row Tent Counts: ";
    for (auto n : currentRowTents)
        std::cout << n << " ";
    std::cout << "\nCurrent Column Tent Counts: ";
    for (auto n : currentColTents)
        std::cout << n << " ";
    std::cout << "\n\n";

    // Print set of tent coordinates.
    std::cout << "Tents (Coordinates):\n";
    for (int i = 0; i < tentTiles.size(); i++)
        std::cout << "(" << tentTiles.getTileAtIndex(i).value().getRow() << ", " << tentTiles.getTileAtIndex(i).value().getCol() << ") ";
    std::cout << "\n\n";

    // Print tent adjacency violations.
    std::cout << "Tent Adjacency Violations:\n";
    for (const auto &entry : tentAdjViolation)
        std::cout << "(" << entry.first.getRow() << ", " << entry.first.getCol() << "): " << (entry.second ? "Violation" : "No Violation") << "\n";
    std::cout << "\n";

    // Print tree tent counts.
    std::cout << "Tree Tent Counts:\n";
    for (const auto &entry : treeTentCount)
        std::cout << "(" << entry.first.getRow() << ", " << entry.first.getCol() << "): " << entry.second << "\n";
    std::cout << "\n";

    // Print violations summary.
    std::cout << "Violation Summary:\n";
    std::cout << "Row Violations: " << rowViolations << "\n";
    std::cout << "Column Violations: " << colViolations << "\n";
    std::cout << "Tent Adjacency Violations: " << tentViolations << "\n";
    std::cout << "Tree Violations: " << treeViolations << "\n";
    std::cout << "Lonely Tent Violations: " << lonelyTentViolations << "\n";
    std::cout << "Total Violations: " << violations << "\n\n";

    // Print additional board information.
    std::cout << "Number of Trees: " << numTrees << "\n";
    std::cout << "Number of Tiles: " << numTiles << "\n";
}

Tile Board::getTile(size_t row, size_t col) const{
    return board[row][col];
}

void Board::setTile(const Tile tile, std::mt19937 gen){
    size_t col = tile.getCoord().getCol();
    size_t row = tile.getCoord().getRow();

    if (board[row][col].getType() == Type::NONE && tile.getType() == Type::TENT)
        placeTent(board[row][col], gen);
    else if (board[row][col].getType() == Type::TENT && tile.getType() == Type::NONE)
        deleteTent(board[row][col].getCoord());
    //else
        //board[row][col] = tile;

}

void Board::debugPrintViolations(){
    std::cout << "\nRow Violations: " << rowViolations << std::endl;
    std::cout << "Column Violations: " << colViolations << std::endl;
    std::cout << "Tent Adjacency Violations: " << tentViolations << std::endl;
    std::cout << "Tree Violations: " << treeViolations << std::endl;
    std::cout << "Invalid Tent Violations: " << lonelyTentViolations << std::endl;
    std::cout << "Total Violations: " << violations << std::endl;
    std::cout << "Num Tents: " << tentTiles.size() << std::endl;

    std::cout << violations << std::endl;
    std::cout << tentTiles.size() << std::endl;
    
    for (int i = 0; i < tentTiles.size(); i++){

        Coord tentCoord = tentTiles.getTileAtIndex(i).value();

        std::cout << tentCoord.getRow()+1 << " " << tentCoord.getCol()+1 << " " << board.at(tentCoord.getRow()).at(tentCoord.getCol()).getDir() << std::endl;

    }


    
}

size_t Board::getViolations() const{
    return violations;
}

std::vector<std::vector<Tile>> Board::getBoard(){
    return board;
}