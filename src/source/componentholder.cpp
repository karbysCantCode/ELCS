#include "componentholder.h"

#include <iostream>

ComponentHolder::ComponentHolder() {}

std::unordered_set<Propagator*> ComponentHolder::addToGrid(const Position& position, Propagator* propagator) {
    auto cell = gridMap[{position.x, position.y}];
    cell.emplace(propagator);
    return cell;
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
    Position posA = anchors[0];
    Position posB = anchors[1];
    unsigned int a = 1;
    while(a < anchors.size() - 1) {
        retSet.merge(addToGridAlongTwoPoints(posA, posB, propagator));
        a++;
        posA = posB;
        posB = anchors[a];
    }
    return retSet;
}

void ComponentHolder::removeFromGrid(const std::vector<Position>& anchors, Propagator* propagator) {
    if (anchors.size() < 2) return;
    Position posA = anchors[0];
    Position posB = anchors[1];
    unsigned int a = 1;
    while(a < anchors.size() - 1) {
        removeFromGridAlongTwoPoints(posA, posB, propagator);
        a++;
        posA = posB;
        posB = anchors[a];
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