#ifndef SEGMENTGRAPHICSITEM_H
#define SEGMENTGRAPHICSITEM_H

#include <QGraphicsItem>
#include <QPainter>
#include <QPainterPathStroker>
#include <QPen>
#include <vector>
#include <QRandomGenerator>

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
        auto* rng = QRandomGenerator::global();

        color = QColor::fromHsv(
          rng->bounded(360),      // Hue
          rng->bounded(160, 256), // Saturation
          rng->bounded(180, 256)  // Value (brightness)
        );
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

protected:
  void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
  void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

private:
    Wire& wire;
    QColor color = Qt::black;
};

#endif