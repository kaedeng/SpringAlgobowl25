#pragma once
#include "tile.h"
#include "tilesSet.h"
#include <vector>
#include <unordered_set>
#include <random>
#include <bitset>

class Board{
    private:
        static constexpr std::size_t MAX_BOARD_SIZE = 250*400;

        // Board dimensions
        size_t rowCount = 0;
        size_t colCount = 0;

        // Board itself, populated by Tiles
        std::vector<std::vector<Tile>> board;

        // Target vals for row/col tents
        std::vector<size_t> rowTentNum;
        std::vector<size_t> colTentNum;
        
        // Current count of each row/col's number of tents (Can be negative, meaning too much)
        std::vector<size_t> currentRowTents;
        std::vector<size_t> currentColTents;
        
        // Check if the tent already violates a tent-tent violation
        std::unordered_map<Coord, bool> tentAdjViolation;

        // Map of all trees with an count of how many tents are attached, for tent-tree violations
        std::unordered_map<Coord, size_t> treeTentCount;
        size_t numTrees;

        // Row/col violations
        size_t rowViolations;         // Sum_i |rowTentNum[i] - currentRowTents[i]|
        size_t colViolations;         // Sum_j |colTentNum[j] - currentColTents[j]|

        // Tent-tent violations
        size_t tentViolations;     // Number of tents that have at least one neighbor

        // Lonely tent/tree violations
        size_t treeViolations;        // Count of trees where treeTentCount != 1
        size_t lonelyTentViolations; // Count of tents with no valid tree (dir 'X')
        
        // Total violations
        size_t violations;

        // Num tiles
        size_t numTiles = rowCount * colCount;
        
        TilesSet openTiles;
        TilesSet tentTiles;

        std::bitset<MAX_BOARD_SIZE> bitBoard;

        // Helper functions to update a tent’s adjacent violation status.
        std::unordered_set<Coord> getAdjacentTents(const Coord &coord) const;
        void updateTentAdjacencyForCoord(const Coord &coord);

        // Helper functions to update row/col violations for tents
        void updateRowAndColForTent(const size_t, const size_t, const bool);

    public:

        Board(
            size_t rowCount,
            size_t colCount,
            std::vector<size_t> rowTentNum,
            std::vector<size_t> colTentNum,
            std::vector<std::vector<Tile>> board,
            size_t numTrees
        );

        Board(const Board& other);
        Board& operator=(const Board& other);
        
        /**
         * @brief Checks for all violations on the board at the current moment
         * @return current number of violations
         */
        size_t getViolations() const;

        /**
         * @brief Places tent based on given coords
         * @return true
         * @return false
         */
        bool placeTent(Tile&);

        /**
         * @brief (currently) Places tent randomly
         * @return true
         * @return false
         */
        bool addTent(std::mt19937&);

        /**
         * @brief (currently) Deletes a random tent
         * Return values are used as error trackers; this is basically a void function.
         * @return true 
         * @return false 
         */
        bool removeTent(std::mt19937&);
        
        /**
         * @brief Deletes a tent at the given tile's coords
         * @return true 
         * @return false 
         */
        bool deleteTent(Coord coord);
    
        /**
         * @brief Should be a combination of remove tent and place tent, but you can move the tent to any neighbor
         * Return values are used as error trackers; this is basically a void function.
         * @return true 
         * @return false 
         */
        bool moveTent(std::mt19937&);

        /*
        /////////////////////////////////////////////////////////////////////////////
        Getters and Setters (Add more if needed)
        /////////////////////////////////////////////////////////////////////////////
        */

        /**
         * @brief Get the Tile given a row and column
         * 0 Indexed
         * @return Tile 
         */
        Tile getTile(size_t, size_t) const;

        /**
         * @brief Get the Tile given a row and column
         * 0 Indexed
         * @return Tile 
         */
        void setTile(const Tile);

        /**
         * @brief Draws the current state of the board
         */
        void drawBoard() const;

        void debugPrintViolations();

        /**
         * @brief Returns the number of rows
         */
        size_t getNumRows() const { return rowCount; };

        /**
         * @brief Returns the number of cols
         */
        size_t getNumCols() const { return colCount; };

        std::vector<std::vector<Tile>> getBoard();

        void printFullBoardInfo() const;
        
        // Bitset operations
        void bitSetTent(const Coord&);
        void bitClearTent(const Coord&);
        size_t countXorBits(const std::bitset<MAX_BOARD_SIZE>&);
        std::bitset<MAX_BOARD_SIZE> getBitBoard() const;

        // Getters and Setters for Board private variables

        // Getter and Setter for rowTentNum
        const std::vector<size_t>& getRowTentNum() const { return rowTentNum; }
        void setRowTentNum(const std::vector<size_t>& rtNum) { rowTentNum = rtNum; }

        // Getter and Setter for colTentNum
        const std::vector<size_t>& getColTentNum() const { return colTentNum; }
        void setColTentNum(const std::vector<size_t>& ctNum) { colTentNum = ctNum; }

        // Getter and Setter for currentRowTents
        const std::vector<size_t>& getCurrentRowTents() const { return currentRowTents; }
        void setCurrentRowTents(const std::vector<size_t>& crt) { currentRowTents = crt; }

        // Getter and Setter for currentColTents
        const std::vector<size_t>& getCurrentColTents() const { return currentColTents; }
        void setCurrentColTents(const std::vector<size_t>& cct) { currentColTents = cct; }

        // Getter and Setter for tentAdjViolation
        const std::unordered_map<Coord, bool>& getTentAdjViolation() const { return tentAdjViolation; }
        void setTentAdjViolation(const std::unordered_map<Coord, bool>& tav) { tentAdjViolation = tav; }

        // Getter and Setter for treeTentCount
        const std::unordered_map<Coord, size_t>& getTreeTentCount() const { return treeTentCount; }
        void setTreeTentCount(const std::unordered_map<Coord, size_t>& ttc) { treeTentCount = ttc; }

        // Getter and Setter for numTrees
        size_t getNumTrees() const { return numTrees; }
        void setNumTrees(size_t nt) { numTrees = nt; }

        // Getter and Setter for rowViolations
        size_t getRowViolations() const { return rowViolations; }
        void setRowViolations(size_t rv) { rowViolations = rv; }

        // Getter and Setter for colViolations
        size_t getColViolations() const { return colViolations; }
        void setColViolations(size_t cv) { colViolations = cv; }

        // Getter and Setter for tentViolations
        size_t getTentViolations() const { return tentViolations; }
        void setTentViolations(size_t tv) { tentViolations = tv; }

        // Getter and Setter for treeViolations
        size_t getTreeViolations() const { return treeViolations; }
        void setTreeViolations(size_t tv) { treeViolations = tv; }

        // Getter and Setter for lonelyTentViolations
        size_t getLonelyTentViolations() const { return lonelyTentViolations; }
        void setLonelyTentViolations(size_t ltv) { lonelyTentViolations = ltv; }

        // Getter and Setter for total violations
        size_t getTotalViolations() const { return violations; }
        void setTotalViolations(size_t v) { violations = v; }

        // Getter and Setter for numTiles
        size_t getNumTiles() const { return numTiles; }
        void setNumTiles(size_t nt) { numTiles = nt; }
  
        TilesSet getOpenTilesData() const { return openTiles; }

        TilesSet getTentTilesData() const { return tentTiles; }

        std::vector<std::vector<Tile>> getBoard() const {
            return board;
        }
};