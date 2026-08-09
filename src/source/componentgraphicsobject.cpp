#include "componentgraphicsobject.h"

#include <QGraphicsSceneMouseEvent>
#include <QPainterPath>

ComponentGraphicsObject::ComponentGraphicsObject(
    Component& component,
    QGraphicsObject* parent
)
    : AbstractGraphicsObject(&component, parent),
      component(component)
{
    // setFlag(QGraphicsItem::ItemIsMovable, false);
    // setFlag(QGraphicsItem::ItemIsSelectable, true);

    // setAcceptHoverEvents(true);

    refresh();
}

void ComponentGraphicsObject::updateWorkspacePosition() {
    setPos(component.getGridPosition().getGridScaledCopy().getQPointF());
}

const Component& ComponentGraphicsObject::getComponent() const
{
    return component;
}

void ComponentGraphicsObject::refresh()
{
    prepareGeometryChange();

    update();
}

void ComponentGraphicsObject::setGhostMode(bool enabled)
{
    if (ghost == enabled)
        return;

    ghost = enabled;

    setOpacity(ghost ? 0.45 : 1.0);

    update();
}

bool ComponentGraphicsObject::ghostMode() const
{
    return ghost;
}

void ComponentGraphicsObject::paintLine(
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

void ComponentGraphicsObject::paintCurve(
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

void ComponentGraphicsObject::paintLabel(
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

void ComponentGraphicsObject::paintPins(
    QPainter* painter
) const
{
    painter->setPen(Qt::NoPen);
    painter->setBrush(Qt::white);

    for (const auto& propagator : component.getPropagators())
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

void ComponentGraphicsObject::paint(
    QPainter* painter,
    const QStyleOptionGraphicsItem*,
    QWidget*
)
{
    painter->setRenderHint(QPainter::Antialiasing, true);

    for (const auto& line : component.getAppearance().lines)
    {
        paintLine(painter, line);
    }

    for (const auto& curve : component.getAppearance().curves)
    {
        paintCurve(painter, curve);
    }

    for (const auto& label : component.getAppearance().labels)
    {
        paintLabel(painter, label);
    }
}

QRectF ComponentGraphicsObject::calculateBoundingRect() const
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

    for (const auto& line : component.getAppearance().lines)
    {
        addPoint(line.begin.getQPointF());
        addPoint(line.end.getQPointF());
    }

    for (const auto& curve : component.getAppearance().curves)
    {
        addPoint(curve.begin.getQPointF());
        addPoint(curve.control1.getQPointF());
        addPoint(curve.control2.getQPointF());
        addPoint(curve.end.getQPointF());
    }

    for (const auto& label : component.getAppearance().labels)
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

QRectF ComponentGraphicsObject::boundingRect() const
{
    return calculateBoundingRect();
}

QPointF ComponentGraphicsObject::appearanceToPixel(
    const Position& position
) const
{
    const Position& componentPosition = component.getGridPosition();

    return (position + componentPosition).getQPointF();
}