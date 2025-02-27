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
    Input inputData;

    Board board = inputData.inputFromFile(argv[1]);
    for (int i = 2; i < argc; ++i) {
        clFlags(&inputData, argv[i]);
    }

    for (int i = 0; i < 50; ++i) {
        // filepath, generationSize, maxGenerationsNoImprovement, board, chance for mutation (%), selectionFactor, cooling rate, elitism, diversity
        TTSolver solver(argv[1], 100, 50, board, 1, 0, 0, 13, 40);
        solver.solve();
    }

    return 0;
}
