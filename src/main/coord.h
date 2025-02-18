#pragma once
#include <functional>

/**
 * @brief Simple general coordinate class that has hashing, simply keeps the row and column with 1 indexing
 * @author Kaelem Deng
 */
class Coord {    
    public:
    Coord() : row(0), col(0) {}
    Coord(int row, int col) : row(row), col(col) {}

    bool operator==(const Coord&) const;

    int getRow() const {return row;}
    void setRow(int row) {this->row = row;}

    int getCol() const {return col;}
    void setCol(int col) {this->col = col;}

    private:
    int row;
    int col;

};

namespace std {
    template <>
    struct hash<Coord> {
        int operator()(const Coord &c) const noexcept {
            // Obtain hash values for the row and column.
            int h1 = std::hash<int>{}(c.getRow());
            int h2 = std::hash<int>{}(c.getCol());

            // Combine the two hashes.
            return h1 ^ (h2 << 1);
        }
    };
}