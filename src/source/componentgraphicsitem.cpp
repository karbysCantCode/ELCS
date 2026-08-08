#include "componentgraphicsitem.h"

#include <QGraphicsSceneMouseEvent>
#include <QPainterPath>

ComponentGraphicsItem::ComponentGraphicsItem(
    const Component& component,
    QGraphicsItem* parent
)
    : QGraphicsObject(parent),
      component(component)
{
    // setFlag(QGraphicsItem::ItemIsMovable, false);
    // setFlag(QGraphicsItem::ItemIsSelectable, true);

    // setAcceptHoverEvents(true);

    refresh();
}

const Component& ComponentGraphicsItem::getComponent() const
{
    return component;
}

void ComponentGraphicsItem::refresh()
{
    prepareGeometryChange();

    update();
}

void ComponentGraphicsItem::setGhostMode(bool enabled)
{
    if (ghost == enabled)
        return;

    ghost = enabled;

    setOpacity(ghost ? 0.45 : 1.0);

    update();
}

bool ComponentGraphicsItem::ghostMode() const
{
    return ghost;
}

void ComponentGraphicsItem::paintLine(
    QPainter* painter,
    const ComponentLine& line
) const
{
    QPen pen(line.color);
    pen.setWidthF(line.width);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);

    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    painter->drawLine(
        appearanceToPixel(line.begin),
        appearanceToPixel(line.end)
    );
}

void ComponentGraphicsItem::paintCurve(
    QPainter* painter,
    const ComponentCurve& curve
) const
{
    QPen pen(curve.color);
    pen.setWidthF(curve.width);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);

    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    QPainterPath path;

    path.moveTo(
        appearanceToPixel(curve.begin)
    );

    path.cubicTo(
        appearanceToPixel(curve.control1),
        appearanceToPixel(curve.control2),
        appearanceToPixel(curve.end)
    );

    painter->drawPath(path);
}

void ComponentGraphicsItem::paintLabel(
    QPainter* painter,
    const ComponentLabel& label
) const
{
    QFont font;
    font.setPointSize(label.fontSize);

    painter->setFont(font);
    painter->setPen(label.color);

    painter->drawText(
        appearanceToPixel(label.position),
        label.text
    );
}

void ComponentGraphicsItem::paintPins(
    QPainter* painter
) const
{
    painter->setPen(Qt::NoPen);
    painter->setBrush(Qt::white);

    for (const auto& propagator : component.propagators)
    {
      if (propagator->isAbstract())
        continue;
        
        if (((Pin*)propagator.get())->getKind() != Propagator::Kinds::PIN)
            continue;

        const Pin* pin =
            static_cast<const Pin*>(propagator.get());

        QPointF position =
            appearanceToPixel(
                pin->getAppearancePosition()
            );

        painter->drawEllipse(
            position,
            3.0,
            3.0
        );
    }
}

void ComponentGraphicsItem::paint(
    QPainter* painter,
    const QStyleOptionGraphicsItem*,
    QWidget*
)
{
    painter->setRenderHint(QPainter::Antialiasing, true);

    for (const auto& line : component.appearance.lines)
    {
        paintLine(painter, line);
    }

    for (const auto& curve : component.appearance.curves)
    {
        paintCurve(painter, curve);
    }

    for (const auto& label : component.appearance.labels)
    {
        paintLabel(painter, label);
    }
}

QRectF ComponentGraphicsItem::calculateBoundingRect() const
{
    QRectF result;

    bool first = true;

    auto addPoint = [&](const QPointF& point)
    {
        if (first)
        {
            result = QRectF(point, QSizeF(0, 0));
            first = false;
        }
        else
        {
            result |= QRectF(point, QSizeF(0, 0));
        }
    };

    for (const auto& line : component.appearance.lines)
    {
        addPoint(line.begin.getQPointF());
        addPoint(line.end.getQPointF());
    }

    for (const auto& curve : component.appearance.curves)
    {
        addPoint(curve.begin.getQPointF());
        addPoint(curve.control1.getQPointF());
        addPoint(curve.control2.getQPointF());
        addPoint(curve.end.getQPointF());
    }

    for (const auto& label : component.appearance.labels)
    {
        addPoint(label.position.getQPointF());
    }

    if (first)
        return QRectF();

    return result.adjusted(
        -5, -5,
        5, 5
    );
}

QRectF ComponentGraphicsItem::boundingRect() const
{
    return calculateBoundingRect();
}

QPointF ComponentGraphicsItem::appearanceToPixel(
    const Position& position
) const
{
    const Position& componentPosition = component.position;

    return (position + componentPosition).getQPointF();
}