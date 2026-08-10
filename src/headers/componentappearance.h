#ifndef COMPONENTAPPEARANCE_H
#define COMPONENTAPPEARANCE_H

#include <QColor>
#include <QString>

#include <cstdint>
#include <functional>
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


/*
    Purely visual "shape" of a component: how it's drawn as a symbol.

    Deliberately holds NOTHING pin-related -- a pin's on-symbol
    position lives on the Pin itself (Pin::appearancePosition), not
    here, so each placed Component instance can keep its own per-pin
    identity (and therefore its own live state/effecting color) while
    still sharing/mirroring the rest of the drawn shape from its
    source SentinelComponent (see Component::getAppearance()).
*/
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


    /*
        Serialization. Appended to a component's own save buffer by
        SentinelComponent::saveToFile()/loadFromFile(); kept
        self-contained here so the on-disk shape format lives next to
        the in-memory shape format.

        Colors are stored as QRgb (ARGB32, 1 uint32 each). Widths are
        stored as a single uint32 by reinterpreting a float bit
        pattern (narrowed from double -- plenty of precision for a
        line width).
    */

    size_t getUint32sToSave() const;

    // Writes starting at *address. Returns the number of uint32s
    // written (always equal to getUint32sToSave()).
    size_t saveToAddress(uint32_t* address) const;

    // Reads using the same readU32(peek) callback convention as
    // SentinelComponent::loadFromFile()'s local lambda: readU32(true)
    // reads and advances, readU32(false) peeks without advancing.
    // Throws std::runtime_error on malformed/truncated data, same as
    // the rest of the loader, so a single try/catch at the top level
    // catches everything.
    void loadFromReader(
        const std::function<uint32_t(bool)>& readU32
    );
};

#endif