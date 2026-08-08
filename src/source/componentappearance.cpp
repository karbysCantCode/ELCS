#include "componentappearance.h"

void ComponentAppearance::addLine(
    const Position& begin,
    const Position& end
)
{
    lines.emplace_back(begin, end);
}

void ComponentAppearance::addCurve(
    const Position& begin,
    const Position& control1,
    const Position& control2,
    const Position& end
)
{
    curves.emplace_back(
        begin,
        control1,
        control2,
        end
    );
}

void ComponentAppearance::addLabel(
    const QString& text,
    const Position& position
)
{
    labels.emplace_back(text, position);
}

void ComponentAppearance::setAnchor(
  const Position& position
)
{
  anchor = position;
}