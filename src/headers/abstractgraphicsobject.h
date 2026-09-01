#ifndef ABSTRACTGRAPHICSOBJECT_H
#define ABSTRACTGRAPHICSOBJECT_H

#include <QGraphicsObject>
#include <QPainter>
#include <QPen>
#include "component.h"

class AbstractGraphicsObject : public QGraphicsObject
{
public:
    virtual ~AbstractGraphicsObject() = default;
    enum { Type = UserType + 1 };

    enum GraphicsObjectTypes {
        PIN,
        SEGMENT,
        COMPONENT
    };
    virtual GraphicsObjectTypes graphicsObjectType() const = 0;
    virtual void updateWorkspacePosition() = 0;
    AbstractPropagator* parentPropagator;
protected:

    void paintSelectionHighlight(QPainter* painter, const QRectF& rect) const
    {
        painter->save();

        QPen pen(QColor(0, 170, 255));
        pen.setWidthF(1.5);
        pen.setStyle(Qt::DashLine);
        pen.setCosmetic(true);

        painter->setPen(pen);
        painter->setBrush(Qt::NoBrush);

        painter->drawRect(rect.adjusted(-4, -4, 4, 4));

        painter->restore();
    }

    int type() const override
    {
        return AbstractGraphicsObject::Type;
    }
    AbstractGraphicsObject(AbstractPropagator* _parentPropagator = nullptr, QGraphicsObject* parent = nullptr)
        : QGraphicsObject(parent),
        parentPropagator(_parentPropagator)
    {
    }
};

#endif 