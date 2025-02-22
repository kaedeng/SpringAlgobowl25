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
    TTSolver(size_t generationSize, size_t maxGenerations, const Board& board, int mutationChance, int selectionFactor, double coolingRate)
    : gen(std::random_device{}()), 
    generationSize(generationSize), 
    maxGenerations(maxGenerations), 
    startingBoard(board),
    mutationChance(mutationChance),
    selectionFactor(selectionFactor),
    coolingRate(coolingRate)
    {
        coolingRate = (double)mutationChance/(double)maxGenerations;
    };

    void solve();

    private:

    std::mt19937 gen;

    // Tune-ables (tuna?)
    size_t generationSize;
    size_t maxGenerations;
    Board startingBoard;
    int mutationChance;
    double coolingRate; // Should be like 0.98 or something high

    int selectionFactor;

    size_t numTiles;
    size_t numRows;
    size_t numCols;
    
    // Holds the set of boards, starting with a set starting board
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
    std::vector<Board> selection(std::vector<Board>);

    /**
     * @brief Creates the next generation and mixes genes
     */
    std::vector<Board> crossover(std::vector<Board>);

    /**
     * @brief Mutates the next generation
     */
    void mutation(Board&);

    std::vector<int> splice(const std::vector<int>& array, int startIndex, int endIndex);

};