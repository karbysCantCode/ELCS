#include "segmentgraphicsitem.h"

#include "component.h"
#include "projectmanager.h"

#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>


QRectF SegmentGraphicsItem::boundingRect() const
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


QPainterPath SegmentGraphicsItem::shape() const
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


void SegmentGraphicsItem::paint(
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

    for (const auto& segment : wire.segments)
    {   
        painter->drawLine(
            segment.begin.getGridScaledCopy().getQPointF(),
            segment.end.getGridScaledCopy().getQPointF()
        );
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


void SegmentGraphicsItem::setColor(const QColor& c)
{
    color = c;
    update();
}


void SegmentGraphicsItem::beginGeometryChange()
{
    prepareGeometryChange();
}

void SegmentGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
  event->ignore();
}
void SegmentGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
  event->ignore();
}