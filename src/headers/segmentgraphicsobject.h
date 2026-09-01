#ifndef SEGMENTGRAPHICSITEM_H
#define SEGMENTGRAPHICSITEM_H

#include <QGraphicsItem>
#include <QPainter>
#include <QPainterPathStroker>
#include <QPen>
#include <vector>
#include "abstractgraphicsobject.h"



struct Segment;

class Wire;

class SegmentGraphicsObject : public AbstractGraphicsObject
{
public:
    
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
    }

    ~SegmentGraphicsObject() override;

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
  
  
  
  
private:
    Wire& wire;
    QColor color = Qt::black;
};

#endif