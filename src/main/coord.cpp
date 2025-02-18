#include "coord.h"

bool Coord::operator==(const Coord& rhs) const {
    return row == rhs.row && col == rhs.col;
}