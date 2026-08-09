#ifndef COMPONENTAPPEARANCE_H
#define COMPONENTAPPEARANCE_H

#include <QColor>
#include <QString>

#include <vector>

#include "position.h"


struct ComponentLine
{
    Position begin;
    Position end;

    QColor color = Qt::black;
    double width = 2.0;

    ComponentLine() = default;

    ComponentLine(
        const Position& begin,
        const Position& end,
        const QColor& color = Qt::black,
        double width = 2.0
    )
        : begin(begin),
          end(end),
          color(color),
          width(width)
    {}
};


struct ComponentCurve
{
    Position begin;
    Position control1;
    Position control2;
    Position end;

    QColor color = Qt::black;
    double width = 2.0;

    ComponentCurve() = default;

    ComponentCurve(
        const Position& begin,
        const Position& control1,
        const Position& control2,
        const Position& end,
        const QColor& color = Qt::black,
        double width = 2.0
    )
        : begin(begin),
          control1(control1),
          control2(control2),
          end(end),
          color(color),
          width(width)
    {}
};


struct ComponentLabel
{
    QString text;
    Position position;

    QColor color = Qt::black;
    int fontSize = 14;

    ComponentLabel() = default;

    ComponentLabel(
        const QString& text,
        const Position& position,
        const QColor& color = Qt::black,
        int fontSize = 14
    )
        : text(text),
          position(position),
          color(color),
          fontSize(fontSize)
    {}
};


struct ComponentAppearance
{
    std::vector<ComponentLine> lines;
    std::vector<ComponentCurve> curves;
    std::vector<ComponentLabel> labels;

    Position anchor;

    void addLine(
        const Position& begin,
        const Position& end,
        const QColor& color = Qt::black,
        double width = 2.0
    );

    void addCurve(
        const Position& begin,
        const Position& control1,
        const Position& control2,
        const Position& end,
        const QColor& color = Qt::black,
        double width = 2.0
    );

    void addLabel(
        const QString& text,
        const Position& position,
        const QColor& color = Qt::black,
        int fontSize = 14
    );

    void setAnchor(const Position& position);
};

#endif