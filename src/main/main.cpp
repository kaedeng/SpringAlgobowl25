#include <iostream>
#include <fstream>
#include <string>
#include "input.h"
#include "ttsolver.h"

void clFlags(Input* input, std::string clArg) {
    if (clArg == "--parse") {
        input->testOutput();
        return;
    }
    else {
        return;
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " boardInputFile [precomputedOutputFile] [other flags]" << std::endl;
        return 1;
    }
    
    Input inputData;
    // Read the base board.
    Board board = inputData.inputFromFile(argv[1]);
    
    // Check if an extra argument is provided and is not a flag.
    // (Assumes that if argv[2] does not start with '-' it is a precomputed output file.)
    if (argc >= 3 && argv[2][0] != '-') {
        board = inputData.mergePrecomputedOutput(board, argv[2]);
    }
    
    // Process any remaining command-line flags.
    for (int i = 2; i < argc; ++i) {
        clFlags(&inputData, argv[i]);
    }

    // Run multiple iterations (or runs) of TTSolver.
    for (int i = 0; i < 50; ++i) {
        // Arguments: filepath, generationSize, maxGenerationsNoImprovement, board,
        // chance for mutation (%), selectionFactor, cooling rate, elitism, diversityWeight
        TTSolver solver(argv[1], 100, 50, board, 1, 0, 0, 13, 40);
        solver.solve();
    }

    return 0;
}
