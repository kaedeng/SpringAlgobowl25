#pragma once
#include "tile.h"
#include <vector>
#include <unordered_set>

class Board{
    private:

        inline static size_t rowCount = 0;
        inline static size_t colCount = 0;

        std::vector<std::vector<Tile>> board;
        std::vector<size_t> rowTentNum;
        std::vector<size_t> colTentNum;
        size_t volations;
        std::unordered_set<Coord> tents;

        /**
         * Finds all violations from the row/column tent count specification
         */
        size_t countRowColViolations();
        /**
         * Counts all the violations of tents being too close to eachother
         */
        size_t countTentViolations();
        /**
         * Counts the violations from trees being lonely ;( or trees have too many tents (not cool bro)
         */
        size_t countTreeViolations();

    public:

        Board(size_t rowCount, size_t colCount, std::vector<size_t> rowTentNum, std::vector<size_t> colTentNum, std::vector<std::vector<Tile>> board);

        /**
         * @brief Checks for all violations on the board at the current moment
         * 
         * @return size_t 
         */
        size_t checkViolations();

        /**
         * @brief Places tent on the iterator's location
         *
         * @return true
         * @return false
         */
        bool placeTent();

        /**
         * @brief Remove the tent on the iterator's location
         * Return values are used as error trackers; this is basically a void function.
         * @return true 
         * @return false 
         */
        bool removeTent(Coord);
        
        /**
         * @brief Should be a combination of remove tent and place tent, but you can move the tent to any neighbor
         * Return values are used as error trackers; this is basically a void function.
         * @return true 
         * @return false 
         */
        bool moveTent();

        /**
         * @brief This will be the 'heuristic function'
         * Check average violations - current violations.
         * return value from that
         * @return double 
         */
        double fitnessFunction(double);
};