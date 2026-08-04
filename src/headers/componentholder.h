#ifndef COMPONENTHOLDER_H
#define COMPONENTHOLDER_H

#include <QObject>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include "component.h"

struct pair_hash {
    template <class T1, class T2>
    std::size_t operator () (const std::pair<T1, T2> &p) const {
        return std::hash<T1>()(p.first) ^ (std::hash<T2>()(p.second) << 1);
    }
};



class ComponentHolder
{
private:
    std::unordered_set<Propagator*> addToGridAlongTwoPoints(const Position& posA, const Position& posB, Propagator* propagator);
    std::unordered_set<Propagator*> getOccupiedAlongTwoPoints(const Position& posA, const Position& posB) const;
    std::unordered_set<Propagator*> getInGridMap(const Position& position) const;
    void removeFromGridAlongTwoPoints(const Position& posA, const Position& posB, Propagator* propagator);
public:
    std::unordered_map<std::pair<unsigned int, unsigned int>, std::unordered_set<Propagator*>, pair_hash> gridMap;

    std::unordered_set<Propagator*> addToGrid(const Position& position, Propagator* propagator);
    void removeFromGrid(const Position& position, Propagator* propagator);

    std::unordered_set<Propagator*> addToGrid(const Segment& segment, Propagator* propagator);
    void removeFromGrid(const Segment& segment, Propagator* propagator);

    std::unordered_set<Propagator*> addToGrid(const std::vector<Position>& anchors, Propagator* propagator);
    void removeFromGrid(const std::vector<Position>& anchors, Propagator* propagator);

    std::unordered_set<Propagator*> addToGrid(const std::vector<Segment>& segments, Propagator* propagator);
    void removeFromGrid(const std::vector<Segment>& segments, Propagator* propagator);
    
    bool isOccupied(const Position& pos) const;
    std::unordered_set<Propagator*> getOccupied(const Position& pos) const;
    std::unordered_set<Propagator*> getOccupied(const std::vector<Segment>& segments, std::unordered_set<Propagator*> excludes) const;
    
    void reset();
    
    ComponentHolder();
};

#endif // COMPONENTHOLDER_H
