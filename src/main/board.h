#pragma once
#include "tile.h"
#include <vector>

class Board{
    private:

        static size_t rowCount;
        static size_t colCount;

        std::vector<std::vector<Tile>> board;
        std::vector<size_t> rowTentNum;
        std::vector<size_t> colTentNum;
        size_t volations;

    public:

        Board(size_t rowCount, size_t colCount, std::vector<size_t> rowTentNum, std::vector<size_t> colTentNum, std::vector<std::vector<Tile>> board){
            this->rowCount = rowCount;
            this->colCount = colCount;
            this->rowTentNum = rowTentNum;
            this->colTentNum = colTentNum;
            this->board = board;
        }

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
        bool removeTent();
        
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
         * @return long 
         */
        long fitnessFunction(size_t);
};