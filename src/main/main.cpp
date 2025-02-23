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

    // generationSize, maxGenerations, board, chance for mutation (%), selectionFactor, cooling rate
    TTSolver solver(300, 100, board, 80, 5, 0.995);
    solver.solve();

    return 0;
}
