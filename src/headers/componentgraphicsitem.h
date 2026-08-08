#ifndef COMPONENTGRAPHICSITEM_H
#define COMPONENTGRAPHICSITEM_H

#include <QGraphicsObject>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

#include "component.h"
// #include "componentappearance.h"

class ComponentGraphicsItem : public QGraphicsObject
{
    Q_OBJECT

public:
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