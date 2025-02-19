#include <iostream>
#include <fstream>
#include <string>
#include "input.h"
#include "ttsolver.h"

int main(int argc, char** argv) {
    Input inputData;

    Board board = inputData.inputFromFile(argv[1]);
    inputData.testOutput();

    return 0;
}
