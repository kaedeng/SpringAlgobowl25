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
    TTSolver(char * filePath, size_t generationSize, size_t maxGenerationsNoImprovement, const Board& board, int mutationChance, int selectionFactor, double coolingRate, int elitismNum, double diversityWeight)
    : filePath(filePath),
      generationSize(generationSize),
      maxGenerationsNoImprovement(maxGenerationsNoImprovement),
      startingBoard(board),
      mutationChance(mutationChance),
      coolingRate(coolingRate),       // coolingRate comes before selectionFactor as declared
      selectionFactor(selectionFactor),
      elitismNum(elitismNum),
      diversityWeight(diversityWeight)
    {
    coolingRate = static_cast<double>(mutationChance) / static_cast<double>(maxGenerationsNoImprovement);
    }

    size_t solve();

    private:

    // Tune-ables (tuna?)
    size_t generationSize;
    size_t maxGenerationsNoImprovement;
    Board startingBoard;
    double diversityWeight;
    int mutationChance;
    int elitismNum;
    double coolingRate; // Should be like 0.98 or something high

    int selectionFactor;

    char * filePath;

    size_t numTiles;
    size_t numRows;
    size_t numCols;

    size_t initalEmptyTiles;

    Board bestBoard = std::move(startingBoard);
    
    // Holds the set of boards, starting with a set starting board
    std::vector<Board> currentGeneration{generationSize, startingBoard};
    std::vector<Board> parentGeneration{generationSize, startingBoard};

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
    std::pair<size_t, size_t> selection(std::mt19937 &gen);

    /**
     * @brief Creates the next generation and mixes genes
     */
    std::pair<Board, Board> crossover(std::pair<size_t, size_t>&, std::mt19937 &gen);

    /**
     * @brief Mutates the next generation
     */
    void mutation(std::pair<Board, Board>&, std::mt19937 &gen);

    void initialize();

    std::vector<int> splice(const std::vector<int>& array, int startIndex, int endIndex);

    double weightedPairScore(const Board &a, const Board &b);

};