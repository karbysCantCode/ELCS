#include "componentappearance.h"


void ComponentAppearance::addLine(
    const Position& begin,
    const Position& end,
    const QColor& color,
    double width
)
{
    lines.emplace_back(begin, end, color, width);
}


void ComponentAppearance::addCurve(
    const Position& begin,
    const Position& control1,
    const Position& control2,
    const Position& end,
    const QColor& color,
    double width
)
{
    curves.emplace_back(
        begin,
        control1,
        control2,
        end,
        color,
        width
    );
}


void ComponentAppearance::addLabel(
    const QString& text,
    const Position& position,
    const QColor& color,
    int fontSize
)
{
    labels.emplace_back(
        text,
        position,
        color,
        fontSize
    );
}


void ComponentAppearance::setAnchor(const Position& position)
{
    anchor = position;
}