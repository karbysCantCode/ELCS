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

        // Color the pin marker by its current propagation state,
        // matching the wire/pin colors elsewhere (see stateColor()).
        painter->setBrush(stateColor(pin->getEffectingState()));

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

    const QRectF br = boundingRect();

    // Draw the bounding box.
    QPen debugBoundingPen(Qt::red);
    debugBoundingPen.setWidthF(1.0);
    debugBoundingPen.setStyle(Qt::DashLine);

    painter->setPen(debugBoundingPen);
    painter->setBrush(Qt::NoBrush);

    painter->drawRect(br);


    // Draw the item's local origin.
    QPen debugOriginPen(Qt::blue);
    debugOriginPen.setWidthF(1.5);

    painter->setPen(debugOriginPen);

    painter->drawLine(
        QPointF(-20, 0),
        QPointF(20, 0)
    );

    painter->drawLine(
        QPointF(0, -20),
        QPointF(0, 20)
    );


    // Draw the component grid position.
    painter->setPen(Qt::black);

    painter->drawText(
        QPointF(10, -10),
        QString(
            "grid=(%1,%2)"
        )
        .arg(component.getGridPosition().x)
        .arg(component.getGridPosition().y)
    );

    // DEBUG: draw bounding box
    QPen debugPen(Qt::red);
    debugPen.setWidthF(1.0);
    debugPen.setStyle(Qt::DashLine);

    painter->setPen(debugPen);
    painter->setBrush(Qt::NoBrush);

    painter->drawRect(boundingRect());


    // Draw a visible point at the component grid position
    // in LOCAL coordinates.
    painter->setBrush(Qt::green);
    painter->setPen(Qt::NoPen);

    painter->drawEllipse(
        component.getGridPosition().getQPointF(),
        5,
        5
    );

    painter->save();
    painter->rotate(component.getRotation());

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

    paintPins(painter);

    painter->restore();

    if (!component.getAppearanceName().empty())
    {
        QFont nameFont;
        nameFont.setPointSize(9);
        nameFont.setBold(true);

        painter->setFont(nameFont);
        painter->setPen(Qt::white);

        QFontMetrics metrics(nameFont);
        const QString appearanceName = QString::fromStdString(component.getAppearanceName());
        const int textWidth = metrics.horizontalAdvance(appearanceName);

        painter->drawText(
            QPointF(br.center().x() - textWidth / 2.0, br.top() - 6),
            appearanceName
        );
    }

    if (isSelected())
    {
        paintSelectionHighlight(painter, boundingRect());
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
            result = QRectF(point, point);
            first = false;
        }
        else
        {
            result.setLeft(std::min(result.left(), point.x()));
            result.setRight(std::max(result.right(), point.x()));
            result.setTop(std::min(result.top(), point.y()));
            result.setBottom(std::max(result.bottom(), point.y()));
        }
    };

    for (const auto& line : component.getAppearance().lines)
    {
        addPoint(line.begin.getRotatedCopy(component.getRotation()).getGridScaledCopy(0).getQPointF());
        addPoint(line.end.getRotatedCopy(component.getRotation()).getGridScaledCopy(0).getQPointF());
    }

    for (const auto& curve : component.getAppearance().curves)
    {
        addPoint(curve.begin.getRotatedCopy(component.getRotation()).getGridScaledCopy(0).getQPointF());
        addPoint(curve.control1.getRotatedCopy(component.getRotation()).getGridScaledCopy(0).getQPointF());
        addPoint(curve.control2.getRotatedCopy(component.getRotation()).getGridScaledCopy(0).getQPointF());
        addPoint(curve.end.getRotatedCopy(component.getRotation()).getGridScaledCopy(0).getQPointF());
    }

    for (const auto& label : component.getAppearance().labels)
    {
        addPoint(label.position.getRotatedCopy(component.getRotation()).getGridScaledCopy(0).getQPointF());
    }

    for (const auto& propagator : component.getPropagators())
    {
        if (propagator->isAbstract())
            continue;

        if (((Propagator*)propagator.get())->getKind() != Propagator::Kinds::PIN)
            continue;

        const Pin* pin =
            static_cast<const Pin*>(propagator.get());

        addPoint(
            pin->getAppearancePosition()
                .getRotatedCopy(component.getRotation())
                .getGridScaledCopy(0)
                .getQPointF()
        );
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
    return position.getGridScaledCopy(0).getQPointF();
}