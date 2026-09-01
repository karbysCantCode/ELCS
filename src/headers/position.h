#ifndef POSITION_H
#define POSITION_H

#include <QPointF>
#include <QPointF>
class Position
{
public:
    int x = 0;
    int y = 0;

    Position(int _x, int _y) : x(_x), y(_y) {}
    Position(const Position& _other) : x(_other.x), y(_other.y) {}
    Position() {}

    constexpr int getX() const {return x;}
    constexpr int getY() const {return y;}
    constexpr QPointF getQPointF() const {return {(qreal)x,(qreal)y};}
    constexpr QPoint getQPoint() const {return {x,y};}

    Position getGridScaledCopy(int offset = 5) const;
    static Position setAsScaledFromGrid(const QPointF& point, int offset = 5);
    Position getRotatedCopy(int degrees) const;

    

    constexpr bool operator==(const Position& other) const {return other.x == x && other.y == y;}
    constexpr bool operator!=(const Position& other) const {return !(other.x == x && other.y == y);}
    Position operator+(const Position& other) const {return Position(other.x+x, other.y+y);}
    Position operator-(const Position& other) const {return Position(x-other.x, y-other.y);}
};

struct PositionHash
{
    std::size_t operator()(const Position& p) const
    {
        return std::hash<int>()(p.x) ^ (std::hash<int>()(p.y) << 1);
    }
};

#endif 