#include <iostream>
#include <fstream>
#include <string>
#include "input.h"
#include "ttsolver.h"

int main(int argc, char** argv) {
    Input foobar;

    foobar.inputFromFile(argv[1]);
    foobar.testOutput();

    return 0;
}
