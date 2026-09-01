#include "componentholder.h"

#include <iostream>
#include <algorithm> 
#include <iterator> 

ComponentHolder::ComponentHolder() {}

std::unordered_set<Propagator*> ComponentHolder::addToGrid(const Position& position, Propagator* propagator) {
    auto& cell = gridMap[{position.x, position.y}];
    auto returnCell = cell;
    for (auto* ptr : returnCell) {
        ptr->addRelatedPropagator(propagator);

        propagator->addRelatedPropagator(ptr);
    }
    cell.emplace(propagator);
    return returnCell;
}

std::unordered_set<Propagator*> ComponentHolder::removeFromGrid(const Position& position, Propagator* propagator) {
    auto it = gridMap.find({position.x, position.y});
    if (it == gridMap.end())
        return {};

    it->second.erase(propagator);

    std::unordered_set<Propagator*> retSet = it->second;

    for (auto* ptr : it->second) {
        ptr->forgetPropagator(propagator);
    }

    if (it->second.empty()) {
        gridMap.erase(it);
    }

    return retSet;
}

std::unordered_set<Propagator*> ComponentHolder::addToGrid(const std::vector<Position>& anchors, Propagator* propagator) {
    if (anchors.size() < 2) return {};

    std::unordered_set<Propagator*> retSet;

    for (size_t i = 0; i + 1 < anchors.size(); i++) {
        retSet.merge(addToGridAlongTwoPoints(anchors[i], anchors[i + 1], propagator));
    }

    retSet.erase(propagator);

    return retSet;
}

std::unordered_set<Propagator*> ComponentHolder::addToGrid(const std::vector<Segment>& segments, Propagator* propagator) {
    std::unordered_set<Propagator*> retSet;

    for (const auto& segment : segments) {
        retSet.merge(addToGridAlongTwoPoints(segment.begin, segment.end, propagator));
    }

    retSet.erase(propagator);

    return retSet;
}

std::unordered_set<Propagator*> ComponentHolder::getOccupied(const std::vector<Segment>& segments, std::unordered_set<Propagator*> excludes) const {
  std::unordered_set<Propagator*> tempSet;

  for (const auto& segment : segments) {
      tempSet.merge(getOccupiedAlongTwoPoints(segment.begin, segment.end));
  }
  for (const auto& elem : excludes) {
    tempSet.erase(elem);
  }

  return tempSet;
}

std::unordered_set<Propagator*> ComponentHolder::removeFromGrid(const std::vector<Position>& anchors, Propagator* propagator) {
    if (anchors.size() < 2) return {};

    std::unordered_set<Propagator*> retSet;

    for (size_t i = 0; i + 1 < anchors.size(); i++) {
        retSet.merge(removeFromGridAlongTwoPoints(anchors[i], anchors[i + 1], propagator));
    }

    retSet.erase(propagator);

    return retSet;
}

std::unordered_set<Propagator*> ComponentHolder::removeFromGrid(const std::vector<Segment>& segments, Propagator* propagator) {
    std::unordered_set<Propagator*> retSet;

    for (const auto& segment : segments) {
        retSet.merge(removeFromGridAlongTwoPoints(segment.begin, segment.end, propagator));
    }

    retSet.erase(propagator);

    return retSet;
}



std::unordered_set<Propagator*> ComponentHolder::addToGridAlongTwoPoints(const Position& posA, const Position& posB, Propagator* propagator) {
    std::unordered_set<Propagator*> retSet;
    if (posA.x == posB.x) {
        
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
        return {}; 
    }
    return retSet;
}

std::unordered_set<Propagator*> ComponentHolder::getInGridMap(const Position& position) const {
  const auto it = gridMap.find({position.x,position.y});
  if (it == gridMap.end())
    return {};

  return it->second;
}

std::unordered_set<Propagator*> ComponentHolder::addToGrid(const Segment& segment, Propagator* propagator) {
  return addToGridAlongTwoPoints(segment.begin,segment.end, propagator);
}
std::unordered_set<Propagator*> ComponentHolder::removeFromGrid(const Segment& segment, Propagator* propagator) {
  return removeFromGridAlongTwoPoints(segment.begin, segment.end, propagator);
}


std::unordered_set<Propagator*> ComponentHolder::getOccupiedAlongTwoPoints(const Position& posA, const Position& posB ) const {
    std::unordered_set<Propagator*> retSet;
    if (posA.x == posB.x) {
        
        int interpPos = posA.y;
        int aLessThanB = posA.y < posB.y;
        if (aLessThanB) {
            while (interpPos <= posB.y) {
                retSet.merge(getInGridMap({posA.x, interpPos}));
                interpPos++;
            }
        } else {
            while (interpPos >= posB.y) {
                retSet.merge(getInGridMap({posA.x, interpPos}));
                interpPos--;
            }
        }
    } else if (posA.y == posB.y) {
        int interpPos = posA.x;
        int aLessThanB = posA.x < posB.x;
        if (aLessThanB) {
            while (interpPos <= posB.x) {
                retSet.merge(getInGridMap({interpPos, posA.y}));
                interpPos++;
            }
        } else {
            while (interpPos >= posB.x) {
                retSet.merge(getInGridMap({interpPos, posA.y}));
                interpPos--;
            }
        }
    } else {
        std::cout << "same two positions componentholder.cpp" << std::endl;
        return {}; 
    }
    return retSet;
}


std::unordered_set<Propagator*> ComponentHolder::removeFromGridAlongTwoPoints(const Position& posA, const Position& posB, Propagator* propagator) {
    std::unordered_set<Propagator*> retSet;
    if (posA.x == posB.x) {
        
        int interpPos = posA.y;
        int aLessThanB = posA.y < posB.y;
        if (aLessThanB) {
            while (interpPos <= posB.y) {
                retSet.merge(removeFromGrid({posA.x, interpPos}, propagator));
                interpPos++;
            }
        } else {
            while (interpPos >= posB.y) {
                retSet.merge(removeFromGrid({posA.x, interpPos}, propagator));
                interpPos--;
            }
        }
    } else if (posA.y == posB.y) {
        int interpPos = posA.x;
        int aLessThanB = posA.x < posB.x;
        if (aLessThanB) {
            while (interpPos <= posB.x) {
                retSet.merge(removeFromGrid({interpPos, posA.y}, propagator));
                interpPos++;
            }
        } else {
            while (interpPos >= posB.x) {
                retSet.merge(removeFromGrid({interpPos, posA.y}, propagator));
                interpPos--;
            }
        }
    } else {
        std::cout << "same two positions componentholder.cpp" << std::endl;
        return {}; 
    }
    return retSet;
}

bool ComponentHolder::isOccupied(const Position& pos) const {
    return gridMap.end() != gridMap.find({pos.x,pos.y});
}

std::unordered_set<Propagator*> ComponentHolder::getOccupied(const Position& pos, std::unordered_set<Propagator*> excludes) const {
    auto it = gridMap.find({pos.x,pos.y});
    std::unordered_set<Propagator*> retSet = {};
    if (it != gridMap.end()) {
        retSet = it->second;
    }
    for (const auto& elem : excludes) {
      retSet.erase(elem);
    }
    return retSet;
}

void ComponentHolder::reset() {
    gridMap.clear();
}