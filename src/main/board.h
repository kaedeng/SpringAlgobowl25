#pragma once
#include "tile.h"
#include <vector>
#include <unordered_set>

class Board{
    private:
        // Board dimensions
        inline static size_t rowCount = 0;
        inline static size_t colCount = 0;

        // Board itself, populated by Tiles
        std::vector<std::vector<Tile>> board;

        // Target vals for row/col tents
        std::vector<size_t> rowTentNum;
        std::vector<size_t> colTentNum;
        
        // Current count of each row/col's number of tents (Can be negative, meaning too much)
        std::vector<size_t> currentRowTents;
        std::vector<size_t> currentColTents;
        
        // Set of all existing tents, use to check for tent-tent violations
        std::unordered_set<Coord> tents;
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
        bool addTent();

        /**
         * @brief (currently) Deletes a random tent
         * Return values are used as error trackers; this is basically a void function.
         * @return true 
         * @return false 
         */
        bool removeTent();
        
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
        bool moveTent();

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
        
};