#include "opentiles.h"
#include <optional>

bool OpenTiles::insert(const Coord c){
    if (tileIndex.find(c) != tileIndex.end())
                return false; // Tile already open.
            tiles.push_back(c);
            tileIndex[c] = tiles.size() - 1;
            return true;
}

bool OpenTiles::remove(const Coord c) {
    auto it = tileIndex.find(c);
    if (it == tileIndex.end())
        return false; // Tile not found.

    size_t idx = it->second;
    // Swap with the last tile.
    Coord lastTile = tiles.back();
    tiles[idx] = lastTile;
    tileIndex[lastTile] = idx;
    
    // Remove the last tile.
    tiles.pop_back();
    tileIndex.erase(it);
    return true;
}

std::optional<Coord> OpenTiles::getTileAtIndex(size_t index) const {
    if (index >= tiles.size())
        return std::nullopt;
    return std::optional<Coord>{tiles[index]};
}

bool OpenTiles::contains(const Coord &c) const {
    return tileIndex.find(c) != tileIndex.end();
}

size_t OpenTiles::size() const {
    return tiles.size();
}