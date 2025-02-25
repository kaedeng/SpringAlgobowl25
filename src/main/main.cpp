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

    for (int i = 0; i < 1; ++i) {
        // filepath, generationSize, maxGenerationsNoImprovement, board, chance for mutation (%), selectionFactor, cooling rate, elitism, diversity
        TTSolver solver(argv[1], 300, 1000, board, 80, 25, 0.999, 15, 0.1);
        solver.solve();
    }

    return 0;
}
