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
    
    for (int i = 0; i < 500; i++){
        for (int i = 1; i < argc; i++) {
            // filepath, generationSize, maxGenerationsNoImprovement, board, chance for mutation (%), selectionFactor, cooling rate, elitism, diversity
            Input inputData;
            Board board = inputData.inputFromFile(argv[i]);
            TTSolver solver(argv[i], 100, 50000, board, 5, 0, 0, 13, 50);
            solver.solve();
        }
    }
    return 0;
}
