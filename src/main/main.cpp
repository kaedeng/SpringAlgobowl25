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

    // generationSize, maxGenerations, board, chance for mutation (%), selectionFactor, cooling rate, elitism
    TTSolver solver(100, 3000, board, 80, 5, 0.999, 5);
    solver.solve();

    return 0;
}
