#ifndef COMPONENTGRAPHICSITEM_H
#define COMPONENTGRAPHICSITEM_H

#include <QGraphicsObject>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

#include "component.h"
#include "abstractgraphicsobject.h"


class ComponentGraphicsObject : public AbstractGraphicsObject
{
    Q_OBJECT

public:
    
    explicit ComponentGraphicsObject(
        Component& component,
        QGraphicsObject* parent = nullptr
    );

    ~ComponentGraphicsObject() override;

    virtual void updateWorkspacePosition() override;

    GraphicsObjectTypes graphicsObjectType() const override {return GraphicsObjectTypes::COMPONENT;}

    QRectF boundingRect() const override;

    void paint(
        QPainter* painter,
        const QStyleOptionGraphicsItem* option,
        QWidget* widget = nullptr
    ) override;

    const Component& getComponent() const;

    void setGhostMode(bool enabled);
    bool ghostMode() const;

    void refresh();

    

protected:
    
    
    

    
    
    
    
    

private:
    const Component& component;

    bool ghost = false;

    QRectF calculateBoundingRect() const;

    QPointF appearanceToPixel(const Position& position) const;

    void paintLine(
        QPainter* painter,
        const ComponentLine& line
    ) const;

    void paintCurve(
        QPainter* painter,
        const ComponentCurve& curve
    ) const;

    void paintLabel(
        QPainter* painter,
        const ComponentLabel& label
    ) const;

    void paintPins(
        QPainter* painter
    ) const;
};

#endif