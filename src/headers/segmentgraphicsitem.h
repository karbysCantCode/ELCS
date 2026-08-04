#ifndef SEGMENTGRAPHICSITEM_H
#define SEGMENTGRAPHICSITEM_H

#include <QGraphicsItem>
#include <QPainter>
#include <QPainterPathStroker>
#include <QPen>
#include <vector>

// #include "component.h"

struct Segment;

class Wire;

class SegmentGraphicsItem : public QGraphicsItem
{
public:
    SegmentGraphicsItem(
        Wire& _wire,
        QGraphicsItem* parent = nullptr
    )
        : QGraphicsItem(parent),
          wire(_wire)
    {
        setZValue(1);
    }

    QRectF boundingRect() const override;
    QPainterPath shape() const override;

    void paint(
        QPainter* painter,
        const QStyleOptionGraphicsItem*,
        QWidget*
    ) override;

    void setColor(const QColor& c);

    void beginGeometryChange();

private:
    Wire& wire;
    QColor color = Qt::black;
};

#endif