#ifndef COMPONENTSTYLEGRAPHICSOBJECT_H
#define COMPONENTSTYLEGRAPHICSOBJECT_H

#include <QGraphicsObject>
#include <QGraphicsItem>
#include <QPainterPath>

#include <vector>

#include "component.h"


enum class StyleHitType
{
    NONE,

    ANCHOR,

    LINE_BEGIN,
    LINE_MIDDLE,
    LINE_END,

    CURVE_BEGIN,
    CURVE_CONTROL1,
    CURVE_MIDDLE,
    CURVE_CONTROL2,
    CURVE_END,

    LABEL,

    PIN
};


class ComponentStyleHandleItem : public QGraphicsItem
{
public:

    ComponentStyleHandleItem(
        StyleHitType hitType,
        int index,
        QGraphicsItem* parent
    );

    StyleHitType getHitType() const { return hitType; }
    int getIndex() const { return index; }

    QRectF boundingRect() const override { return bounds; }
    QPainterPath shape() const override { return hitShape; }

    void paint(
        QPainter*,
        const QStyleOptionGraphicsItem*,
        QWidget*
    ) override {}


    void setPointHitArea(
        const QPointF& center,
        double radius
    );

    void setStrokeHitArea(
        const QPainterPath& path,
        double width
    );

    void setRectHitArea(
        const QRectF& rect
    );


private:

    StyleHitType hitType;
    int index;

    QRectF bounds;
    QPainterPath hitShape;
};


class ComponentStyleGraphicsObject : public QGraphicsObject
{
    Q_OBJECT

public:

    explicit ComponentStyleGraphicsObject(
        SentinelComponent& component,
        QGraphicsItem* parent = nullptr
    );


    QRectF boundingRect() const override;

    void paint(
        QPainter* painter,
        const QStyleOptionGraphicsItem* option,
        QWidget* widget = nullptr
    ) override;


    void refresh();


    using HitType = StyleHitType;

    SentinelComponent& getSentinelComponent() {return component;}


private:

    SentinelComponent& component;

    std::vector<ComponentStyleHandleItem*> handleItems;


    static constexpr double GRID_SIZE = 10.0;

    static constexpr double HIT_RADIUS = 7.0;

    static constexpr double PIN_RADIUS = 4.0;

    static constexpr double HANDLE_RADIUS = 5.0;


    QPointF positionToPixel(
        const Position& position
    ) const;


    Position pixelToPosition(
        const QPointF& position
    ) const;


    void paintLine(
        QPainter* painter,
        const ComponentLine& line,
        int index
    ) const;


    void paintCurve(
        QPainter* painter,
        const ComponentCurve& curve,
        int index
    ) const;


    void paintLabel(
        QPainter* painter,
        const ComponentLabel& label,
        int index
    ) const;


    void paintPins(
        QPainter* painter
    ) const;


    void paintAnchor(
        QPainter* painter
    ) const;


    
    void rebuildHandles();

    ComponentStyleHandleItem* addPointHandle(
        StyleHitType type,
        int index,
        const QPointF& center,
        double radius
    );

    ComponentStyleHandleItem* addStrokeHandle(
        StyleHitType type,
        int index,
        const QPainterPath& path,
        double width
    );

    ComponentStyleHandleItem* addRectHandle(
        StyleHitType type,
        int index,
        const QRectF& rect
    );
};

#endif