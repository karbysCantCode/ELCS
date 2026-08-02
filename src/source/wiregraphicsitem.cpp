#include "wiregraphicsitem.h"

#include "projectmanager.h"

#define PADDING 20

QRectF WireGraphicsItem::boundingRect() const
{
    QRectF rect;

    for (const auto& p : wire.anchors)
        rect |= QRectF(p.getGridScaledCopy().getQPointF(), QSizeF(0,0));
    return rect.adjusted(-5,-5,5,5);
}

QPainterPath WireGraphicsItem::shape() const
{
    QPainterPath path;
    path.moveTo(wire.anchors[0].getGridScaledCopy().getQPointF());

    for (size_t i = 1; i < wire.anchors.size(); i++){
        path.lineTo(wire.anchors[i].getGridScaledCopy().getQPointF());
    }
    QPainterPathStroker stroker;
    stroker.setWidth(10);

    return stroker.createStroke(path);
}

void WireGraphicsItem::paint(
    QPainter* painter,
    const QStyleOptionGraphicsItem*,
    QWidget*)
{
    QPen pen(Qt::black);
    pen.setWidthF(2.0);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);

    painter->setPen(pen);

    if (wire.anchors.empty())
        return;

    QPolygonF polyline;

    for (const auto& anchor : wire.anchors)
    {
        polyline << anchor.getGridScaledCopy().getQPointF();
    }

    painter->drawPolyline(polyline);
}

void WireGraphicsItem::setColor(const QColor& c)
{
color = c;
update();
}

void WireGraphicsItem::beginGeometryChange()
{
    prepareGeometryChange();    
}