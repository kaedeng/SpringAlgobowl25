#include "tilesSet.h"
#include <optional>

TilesSet::TilesSet(){
    tiles.reserve(250*400);
}

bool TilesSet::insert(const Coord c){
    if (tileIndex.find(c) != tileIndex.end())
                return false; // Tile already open.
            tiles.push_back(std::move(c));
            tileIndex[c] = tiles.size() - 1;
            return true;
}

bool TilesSet::remove(const Coord c) {
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

std::optional<Coord> TilesSet::getTileAtIndex(size_t index) const {
    if (index >= tiles.size())
        return std::nullopt;
    return std::optional<Coord>{tiles[index]};
}

bool TilesSet::contains(const Coord &c) const {
    return tileIndex.count(c) > 0;
}

size_t TilesSet::size() const {
    return tiles.size();
}