#pragma once

#include "coord.h"

enum class Type {
    TREE,
    TENT,
    NONE
};

class Tile{
    private:

    Type type;
    char tentDir = 'X';
    Coord coord;

    public:

        constexpr Tile(Type tileType, size_t row, size_t col)
            : type(tileType), tentDir('X'), coord(row, col) {}

        constexpr Tile(Type tileType, size_t row, size_t col, char tentDir)
            : type(tileType), tentDir(tentDir), coord(row, col) {}

        // Getters
        constexpr Type getType() const { return type; }
        constexpr char getDir() const { return tentDir; }
        constexpr Coord getCoord() const { return coord; }
        
        // Setters
        void setType(Type type){ this->type = type; }
        void setDir(char tentDir){ this->tentDir = tentDir; }
        void setCoord(size_t row, size_t col) {coord.setRow(row); coord.setCol(col); }

};
