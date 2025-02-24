#pragma once
#include "coord.h"
#include <vector>
#include <unordered_map>
#include <optional>

class OpenTiles {
    private:
        // Vector to store open tiles.
        std::vector<Coord> tiles;
        // Map from Coord to its index in the vector.
        std::unordered_map<Coord, size_t> tileIndex;
        
    public:
        // Insert a new open tile.
        // Returns true if the tile was added; false if it already exists.
        bool insert(const Coord c);
        
        // Delete an open tile.
        // Returns true if the tile was deleted; false if it was not found.
        bool remove(const Coord c);
        
        // Access a tile by index in the vector.
        // Throws an exception if the index is out of range.
        std::optional<Coord> getTileAtIndex(size_t index) const;
        
        // Check if a given Coord is an open tile.
        bool contains(const Coord &c) const;
        
        // Returns the number of open tiles.
        size_t size() const;
    };