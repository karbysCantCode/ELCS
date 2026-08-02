#include "componentholder.h"

#include <iostream>

ComponentHolder::ComponentHolder() {}

std::unordered_set<Propagator*> ComponentHolder::addToGrid(const Position& position, Propagator* propagator) {
    auto& cell = gridMap[{position.x, position.y}];
    auto returnCell = cell;
    cell.emplace(propagator);
    return returnCell;
}

void ComponentHolder::removeFromGrid(const Position& position, Propagator* propagator) {
    auto it = gridMap.find({position.x, position.y});
    if (it != gridMap.end()) {
        it->second.erase(propagator);
        if (it->second.empty()) {
            gridMap.erase(it);
        }
    }
}

std::unordered_set<Propagator*> ComponentHolder::addToGrid(const std::vector<Position>& anchors, Propagator* propagator) {
    if (anchors.size() < 2) return {};

    std::unordered_set<Propagator*> retSet;

    // FIX: the old `while (a < anchors.size() - 1)` loop always dropped the
    // final segment (and ran zero times for a 2-anchor straight wire).
    // Iterate directly over adjacent anchor pairs instead.
    for (size_t i = 0; i + 1 < anchors.size(); i++) {
        retSet.merge(addToGridAlongTwoPoints(anchors[i], anchors[i + 1], propagator));
    }

    retSet.erase(propagator);

    return retSet;
}

void ComponentHolder::removeFromGrid(const std::vector<Position>& anchors, Propagator* propagator) {
    if (anchors.size() < 2) return;

    // FIX: same off-by-one as addToGrid() above — must match it exactly,
    // or removal will leave stale entries for whichever segment addToGrid
    // added but removeFromGrid failed to remove (or vice versa).
    for (size_t i = 0; i + 1 < anchors.size(); i++) {
        removeFromGridAlongTwoPoints(anchors[i], anchors[i + 1], propagator);
    }
}


//A to B
std::unordered_set<Propagator*> ComponentHolder::addToGridAlongTwoPoints(const Position& posA, const Position& posB, Propagator* propagator) {
    std::unordered_set<Propagator*> retSet;
    if (posA.x == posB.x) {
        // change y
        int interpPos = posA.y;
        int aLessThanB = posA.y < posB.y;
        if (aLessThanB) {
            while (interpPos <= posB.y) {
                retSet.merge(addToGrid({posA.x, interpPos}, propagator));
                interpPos++;
            }
        } else {
            while (interpPos >= posB.y) {
                retSet.merge(addToGrid({posA.x, interpPos}, propagator));
                interpPos--;
            }
        }
    } else if (posA.y == posB.y) {
        int interpPos = posA.x;
        int aLessThanB = posA.x < posB.x;
        if (aLessThanB) {
            while (interpPos <= posB.x) {
                retSet.merge(addToGrid({interpPos, posA.y}, propagator));
                interpPos++;
            }
        } else {
            while (interpPos >= posB.x) {
                retSet.merge(addToGrid({interpPos, posA.y}, propagator));
                interpPos--;
            }
        }
    } else {
        std::cout << "same two positions componentholder.cpp" << std::endl;
        return {}; //erorr.....
    }
    return retSet;
}

//A to B
void ComponentHolder::removeFromGridAlongTwoPoints(const Position& posA, const Position& posB, Propagator* propagator) {
    if (posA.x == posB.x) {
        // change y
        int interpPos = posA.y;
        int aLessThanB = posA.y < posB.y;
        if (aLessThanB) {
            while (interpPos <= posB.y) {
                removeFromGrid({posA.x, interpPos}, propagator);
                interpPos++;
            }
        } else {
            while (interpPos >= posB.y) {
                removeFromGrid({posA.x, interpPos}, propagator);
                interpPos--;
            }
        }
    } else if (posA.y == posB.y) {
        int interpPos = posA.x;
        int aLessThanB = posA.x < posB.x;
        if (aLessThanB) {
            while (interpPos <= posB.x) {
                removeFromGrid({interpPos, posA.y}, propagator);
                interpPos++;
            }
        } else {
            while (interpPos >= posB.x) {
                removeFromGrid({interpPos, posA.y}, propagator);
                interpPos--;
            }
        }
    } else {
        std::cout << "same two positions componentholder.cpp" << std::endl;
        return; //erorr.....
    }
}

bool ComponentHolder::isOccupied(const Position& pos) const {
    return gridMap.end() != gridMap.find({pos.x,pos.y});
}

std::unordered_set<Propagator*> ComponentHolder::getOccupied(const Position& pos) const {
    auto it = gridMap.find({pos.x,pos.y});
    if (it != gridMap.end()) {
        return it->second;
    }
    return {};
}

void ComponentHolder::reset() {
    gridMap.clear();
}