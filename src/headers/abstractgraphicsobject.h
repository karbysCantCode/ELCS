#ifndef ABSTRACTGRAPHICSOBJECT_H
#define ABSTRACTGRAPHICSOBJECT_H

#include <QGraphicsObject>
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

#endif // ABSTRACTGRAPHICSOBJECT_H