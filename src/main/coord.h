#pragma once
#include <functional>

/**
 * @brief Simple general coordinate class that has hashing, simply keeps the row and column with 1 indexing
 * @author Kaelem Deng
 */
class Coord {
    public:
        constexpr Coord() : row(0), col(0) {}
        constexpr Coord(int row, int col) : row(row), col(col) {}
    
        constexpr bool operator==(const Coord &rhs) const {
            return row == rhs.row && col == rhs.col;
        }
    
        constexpr int getRow() const { return row; }
        constexpr int getCol() const { return col; }
        void setRow(int row) { this->row = row; }
        void setCol(int col) { this->col = col; }
    
    private:
        int row;
        int col;
    };
    
namespace std {
    template <>
    struct hash<Coord> {
        size_t operator()(const Coord &c) const noexcept {
            size_t h1 = std::hash<int>{}(c.getRow());
            size_t h2 = std::hash<int>{}(c.getCol());
            return h1 ^ (h2 << 1); // Combines the two hashes
        }
    };
}