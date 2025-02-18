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
     * @brief Checks for all violations on the board at the current moment
     * 
     * @return size_t 
     */
    size_t checkViolations();

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
     * This may just be as simple as returning the current amount of violations, or we can change this later.
     * @return long 
     */
    long energyFunction();

    /**
     * @brief Creates the output file with the current iteration of the chart.
     * 
     * @return true 
     * @return false 
     */
    bool createOutput();

    private:

};