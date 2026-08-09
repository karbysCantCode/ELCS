#ifndef SEGMENTGRAPHICSITEM_H
#define SEGMENTGRAPHICSITEM_H

#include <QGraphicsItem>
#include <QPainter>
#include <QPainterPathStroker>
#include <QPen>
#include <vector>
#include <QRandomGenerator>
#include "abstractgraphicsobject.h"

// #include "component.h"

struct Segment;

class Wire;

class SegmentGraphicsObject : public AbstractGraphicsObject
{
public:
    // enum { Type = UserType + 2 };
    GraphicsObjectTypes graphicsObjectType() const override {return GraphicsObjectTypes::SEGMENT;}
    SegmentGraphicsObject(
        Wire& _wire,
        QGraphicsObject* parent = nullptr
    )
        : AbstractGraphicsObject(&_wire, parent),
          wire(_wire)
    {
        setZValue(1);
        setAcceptedMouseButtons(Qt::NoButton);
        auto* rng = QRandomGenerator::global();

        color = QColor::fromHsv(
          rng->bounded(360),      // Hue
          rng->bounded(160, 256), // Saturation
          rng->bounded(180, 256)  // Value (brightness)
        );
    }

    virtual void updateWorkspacePosition() override;

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
  // int type() const override
  // {
  //   return SegmentGraphicsObject::Type;
  // }
private:
    Wire& wire;
    QColor color = Qt::black;
};

#endif