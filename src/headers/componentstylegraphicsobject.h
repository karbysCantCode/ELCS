#ifndef COMPONENTSTYLEGRAPHICSOBJECT_H
#define COMPONENTSTYLEGRAPHICSOBJECT_H

#include <QGraphicsObject>
#include <QGraphicsItem>
#include <QPainterPath>

#include <vector>

#include "component.h"


/*
    What kind of style element a given handle item represents.
    Lives outside both classes below so it can be shared between
    them without ordering headaches (enum classes can't easily be
    forward declared with a nested-class definition split across
    two class bodies).
*/
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


/*
    Invisible child item that exists purely so Qt's own item-picking
    system (QGraphicsView::itemAt() / QGraphicsScene::items()) can
    find individual style elements -- line endpoints, curve handles,
    labels, the anchor, pins, etc -- the same way CircuitWorkspace
    relies on itemAt() against real graphics items on the main
    canvas.

    It paints nothing itself; the actual visuals are still drawn by
    the owning ComponentStyleGraphicsObject::paint(). This item only
    gives each selectable element a real QGraphicsItem with its own
    shape() so mouse events resolve to it via normal Qt hit testing
    instead of manual distance checks.
*/
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


    // Kept as an alias so existing call sites that spelled this
    // ComponentStyleGraphicsObject::HitType keep compiling.
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


    /*
        Rebuilds the invisible hit-test child items to match the
        current appearance state. Called from refresh(), since the
        number of lines/curves/labels can change (add/delete).
    */
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