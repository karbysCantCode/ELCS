#include "segmentgraphicsobject.h"

#include "component.h"
#include "projectmanager.h"

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>


QRectF SegmentGraphicsObject::boundingRect() const
{
    QRectF rect;

    for (const auto& segment : wire.segments)
    {
        rect |= QRectF(
            segment.begin.getGridScaledCopy().getQPointF(),
            segment.end.getGridScaledCopy().getQPointF()
        );
    }

    return rect.adjusted(-5, -5, 5, 5);
}


QPainterPath SegmentGraphicsObject::shape() const
{
    QPainterPath path;

    for (const auto& segment : wire.segments)
    {
        path.moveTo(segment.begin.getGridScaledCopy().getQPointF());
        path.lineTo(segment.end.getGridScaledCopy().getQPointF());
    }

    QPainterPathStroker stroker;
    stroker.setWidth(10);

    return stroker.createStroke(path);
}


void SegmentGraphicsObject::paint(
    QPainter* painter,
    const QStyleOptionGraphicsItem*,
    QWidget*)
{
    if (wire.segments.empty())
        return;


    QPen pen(color);
    pen.setWidthF(2.0);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);

    painter->setPen(pen);
    painter->setBrush(color);

    for (const auto& segment : wire.segments)
    {   
        painter->drawLine(
            segment.begin.getGridScaledCopy().getQPointF(),
            segment.end.getGridScaledCopy().getQPointF()
        );
    }

    if (wire.junctionsDirty)
        wire.updateJunctions();

    for (const auto& junction : wire.cachedJunctions) {
        QPointF center = junction.getGridScaledCopy().getQPointF();
        painter->drawEllipse(center, 3.0, 3.0);
    }

    QPen debugPen(Qt::red);
    debugPen.setWidthF(1);
    painter->setPen(debugPen);
    painter->setBrush(Qt::NoBrush);

    painter->drawPath(shape());

    painter->setPen(QPen(Qt::blue, 1));
    painter->drawRect(boundingRect());

    // optional selection outline
    if (isSelected())
    {
        QPen selectionPen(QColor(0,120,255));
        selectionPen.setWidthF(8);
        selectionPen.setCapStyle(Qt::RoundCap);

        painter->setPen(selectionPen);

        for (const auto& segment : wire.segments)
        {
            painter->drawLine(
                segment.begin.getGridScaledCopy().getQPointF(),
                segment.end.getGridScaledCopy().getQPointF()
            );
        }

        painter->setPen(pen);

        for (const auto& segment : wire.segments)
        {
            painter->drawLine(
                segment.begin.getGridScaledCopy().getQPointF(),
                segment.end.getGridScaledCopy().getQPointF()
            );
        }
    }
}


void SegmentGraphicsObject::setColor(const QColor& c)
{
    color = c;
    update();
}

void SegmentGraphicsObject::updateWorkspacePosition() {
    // for (segment)
}


void SegmentGraphicsObject::beginGeometryChange()
{
    prepareGeometryChange();
}

void SegmentGraphicsObject::mousePressEvent(QGraphicsSceneMouseEvent* event) {
  event->ignore();
}
void SegmentGraphicsObject::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
  event->ignore();
}