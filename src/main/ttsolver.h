#pragma once

#include "board.h"

#include <stdlib.h>
#include <vector>
#include <random>

/**
 * @brief Our main algorithm for solving the Tents and Trees problem
 * Utilizes the Markov Chain Monte Carlo algorithmic process to iteratively find solutions to the problem 
 * TODO: Change functions as you see fit, these are just stubs so lmk if you find something that may not be useful
 * @author Kaelem Deng, Marina Feller, Shawn Prather
 */
class TTSolver {
    public:
    /**
     * @brief Construct a new TTSolver object
     * Should just pass the information from parsing files to initialize this
     */
    TTSolver(size_t generationSize, size_t maxGenerations, size_t sameChildrenNum, const Board& board) : gen(std::random_device{}()), generationSize(generationSize), maxGenerations(maxGenerations), sameChildrenNum(sameChildrenNum), startingBoard(board) {};

    void solve();

    private:

    std::mt19937 gen;

    size_t generationSize;
    size_t maxGenerations;
    size_t sameChildrenNum;
    Board startingBoard;

    size_t numTiles;
    size_t numRows;
    size_t numCols;

    std::vector<Board> currentGeneration{generationSize, startingBoard};
    std::vector<int> bestViolationsHistory;

    /**
     * @brief Creates the output file with the current iteration of the chart.
     * 
     * @return true 
     * @return false 
     */
    bool createOutput();

    /**
     * @brief Creates initial boards based on generationSize.
     */
    void initialize(const Board&);

    /**
     * @brief Main iteration loop
     */
    void iterate();

    /**
     * @brief Selects the next generation
     */
    void selection();

    /**
     * @brief Creates the next generation and mixes genes
     */
    void crossover();

    /**
     * @brief Mutates the next generation
     */
    void mutation();


};