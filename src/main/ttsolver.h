#pragma once
#include <stdlib.h>

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
    TTSolver();

    /**
     * @brief Creates the output file with the current iteration of the chart.
     * 
     * @return true 
     * @return false 
     */
    bool createOutput();

    private:

};