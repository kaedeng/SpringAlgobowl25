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

        Tile(Type tileType, size_t row, size_t col){
            type = tileType;
            coord = Coord(row, col);
        }    

        Type getType() const { return type; };
        void setType(Type type){ this->type = type; };

        char getDir() const { return tentDir; };
        void setDir(char tentDir){ this->tentDir = tentDir; }

        Coord getCoord() const { return coord; }
        void setCoord(size_t row, size_t col) {coord.setRow(row); coord.setCol(col); }
};
