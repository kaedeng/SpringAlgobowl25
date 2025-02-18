#include "board.h"

size_t Board::checkViolations(){}

bool Board::placeTent(){}

bool Board::removeTent(){}

bool Board::moveTent(){}

double Board::fitnessFunction(double averageViolations){
    return averageViolations - (double)volations;
}
