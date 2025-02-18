#include <iostream>
#include <fstream>
#include <string>
#include "input.h"
#include "ttsolver.h"

int main(int argc, char** argv) {
    Input foobar;
    foobar.inputFromFile("../tests/one.test");

    return 0;
}
