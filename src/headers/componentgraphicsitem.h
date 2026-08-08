#ifndef COMPONENTGRAPHICSITEM_H
#define COMPONENTGRAPHICSITEM_H

#include <QGraphicsItem>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

#include "component.h"
// #include "componentappearance.h"

class ComponentGraphicsItem : public QGraphicsItem
{
    Q_OBJECT

public:
    enum { Type = UserType + 3 };
    explicit ComponentGraphicsItem(
        const Component& component,
        QGraphicsItem* parent = nullptr
    );

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
    // void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    // void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    // void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

    //handled by circuitworkspace
    int type() const override
    {
      return ComponentGraphicsItem::Type;
    }

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