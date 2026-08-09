#include "position.h"

Position Position::setAsScaledFromGrid(const QPointF& point, int offset)
{
    Position a;
    a.x = static_cast<int>(std::floor((point.x() + offset) / 10.0));
    a.y = static_cast<int>(std::floor((point.y() + offset) / 10.0));

    return a;
}

Position Position::getGridScaledCopy(int offset) const {
    return {x*10+offset,y*10+offset};
}