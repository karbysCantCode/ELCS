#include "componentstylegraphicsobject.h"

#include <QPainter>
#include <QFontMetrics>
#include <QPainterPathStroker>

#include <algorithm>
#include <cmath>


namespace
{
    
    
    
    double handleZValueFor(StyleHitType type)
    {
        switch (type)
        {
            case StyleHitType::LINE_MIDDLE:
            case StyleHitType::CURVE_MIDDLE:
                return 0.0;

            case StyleHitType::LINE_BEGIN:
            case StyleHitType::LINE_END:
            case StyleHitType::CURVE_BEGIN:
            case StyleHitType::CURVE_CONTROL1:
            case StyleHitType::CURVE_CONTROL2:
            case StyleHitType::CURVE_END:
                return 1.0;

            case StyleHitType::ANCHOR:
            case StyleHitType::LABEL:
            case StyleHitType::PIN:
                return 2.0;

            default:
                return 0.0;
        }
    }
}



ComponentStyleHandleItem::ComponentStyleHandleItem(
    StyleHitType hitType,
    int index,
    QGraphicsItem* parent
)
    : QGraphicsItem(parent),
      hitType(hitType),
      index(index)
{
}


void ComponentStyleHandleItem::setPointHitArea(
    const QPointF& center,
    double radius
)
{
    prepareGeometryChange();

    bounds = QRectF(
        center.x() - radius,
        center.y() - radius,
        radius * 2,
        radius * 2
    );

    hitShape = QPainterPath();
    hitShape.addEllipse(center, radius, radius);
}


void ComponentStyleHandleItem::setStrokeHitArea(
    const QPainterPath& path,
    double width
)
{
    prepareGeometryChange();

    QPainterPathStroker stroker;

    stroker.setWidth(width);
    stroker.setCapStyle(Qt::RoundCap);
    stroker.setJoinStyle(Qt::RoundJoin);

    hitShape = stroker.createStroke(path);
    bounds = hitShape.boundingRect();
}


void ComponentStyleHandleItem::setRectHitArea(
    const QRectF& rect
)
{
    prepareGeometryChange();

    bounds = rect;

    hitShape = QPainterPath();
    hitShape.addRect(rect);
}



ComponentStyleGraphicsObject::ComponentStyleGraphicsObject(
    SentinelComponent& component,
    QGraphicsItem* parent
)
    : QGraphicsObject(parent),
      component(component)
{
    setFlag(QGraphicsItem::ItemIsSelectable, false);

    setAcceptedMouseButtons(Qt::NoButton);

    setZValue(10.0);

    refresh();
}


void ComponentStyleGraphicsObject::refresh()
{
    prepareGeometryChange();

    rebuildHandles();

    update();
}


QPointF ComponentStyleGraphicsObject::positionToPixel(
    const Position& position
) const
{
    const Position anchor =
        component.getAppearance().anchor;

    return QPointF(
        (position.x - anchor.x) * GRID_SIZE,
        (position.y - anchor.y) * GRID_SIZE
    );
}


Position ComponentStyleGraphicsObject::pixelToPosition(
    const QPointF& position
) const
{
    const Position anchor =
        component.getAppearance().anchor;

    return Position(
        anchor.x +
            static_cast<int>(
                std::round(position.x() / GRID_SIZE)
            ),

        anchor.y +
            static_cast<int>(
                std::round(position.y() / GRID_SIZE)
            )
    );
}


QRectF ComponentStyleGraphicsObject::boundingRect() const
{
    QRectF bounds(-50, -50, 100, 100);

    const auto& appearance =
        component.getAppearance();


    auto includePoint =
        [&](const QPointF& point)
        {
            bounds |= QRectF(
                point.x() - 20,
                point.y() - 20,
                40,
                40
            );
        };

    includePoint(
        appearance.anchor.getGridScaledCopy().getQPointF()
        
    );


    for (const auto& line : appearance.lines)
    {
        includePoint(line.begin.getGridScaledCopy().getQPointF());
        includePoint(line.end.getGridScaledCopy().getQPointF());
    }


    for (const auto& curve : appearance.curves)
    {
        includePoint(curve.begin.getGridScaledCopy().getQPointF());
        includePoint(curve.control1.getGridScaledCopy().getQPointF());
        includePoint(curve.control2.getGridScaledCopy().getQPointF());
        includePoint(curve.end.getGridScaledCopy().getQPointF());
    }


    for (const auto& label : appearance.labels)
    {
        includePoint(label.position.getGridScaledCopy().getQPointF());
    }


    for (const Pin* pin : component.getPins())
    {
        includePoint(
            pin->getAppearancePosition().getGridScaledCopy().getQPointF()
        );
    }


    return bounds.adjusted(
        -20,
        -20,
        20,
        20
    );
}

void ComponentStyleGraphicsObject::paintLine(
    QPainter* painter,
    const ComponentLine& line,
    int
) const
{
    painter->setRenderHint(QPainter::Antialiasing);

    QPen pen(
        line.color,
        line.width
    );

    painter->setPen(pen);

    painter->drawLine(
        line.begin.getGridScaledCopy().getQPointF(),
        line.end.getGridScaledCopy().getQPointF()
    );
}

void ComponentStyleGraphicsObject::paintCurve(
    QPainter* painter,
    const ComponentCurve& curve,
    int
) const
{
    painter->setRenderHint(QPainter::Antialiasing);

    QPen pen(
        curve.color,
        curve.width
    );

    painter->setPen(pen);

    QPainterPath path;

    path.moveTo(
        curve.begin.getGridScaledCopy().getQPointF()
    );

    path.cubicTo(
        curve.control1.getGridScaledCopy().getQPointF(),
        curve.control2.getGridScaledCopy().getQPointF(),
        curve.end.getGridScaledCopy().getQPointF()
    );

    painter->drawPath(path);
}

void ComponentStyleGraphicsObject::paintLabel(
    QPainter* painter,
    const ComponentLabel& label,
    int
) const
{
    painter->setPen(label.color);

    QFont font = painter->font();

    font.setPointSize(label.fontSize);

    painter->setFont(font);

    painter->drawText(
        label.position.getGridScaledCopy().getQPointF(),
        label.text
    );
}

void ComponentStyleGraphicsObject::paintPins(
    QPainter* painter
) const
{
    painter->setRenderHint(QPainter::Antialiasing);

    for (const Pin* pin : component.getPins())
    {
        const QPointF point = pin->getAppearancePosition().getGridScaledCopy().getQPointF();


        QPen pen(
            QColor("#55aaff"),
            2.0
        );

        painter->setPen(pen);

        painter->setBrush(
            QColor("#202a35")
        );


        painter->drawEllipse(
            point,
            PIN_RADIUS + 2,
            PIN_RADIUS + 2
        );


        painter->setBrush(
            QColor("#55aaff")
        );

        painter->drawEllipse(
            point,
            PIN_RADIUS / 2,
            PIN_RADIUS / 2
        );


        QFont font = painter->font();

        font.setPointSize(10);

        painter->setFont(font);

        painter->setPen(
            QColor("#55aaff")
        );


        painter->drawText(
            point + QPointF(9, 4),
            QString::fromStdString(
                pin->getName()
            )
        );
    }
}

void ComponentStyleGraphicsObject::paintAnchor(
    QPainter* painter
) const
{
    const QPointF point = component.getAppearance().anchor.getGridScaledCopy().getQPointF();


    painter->setRenderHint(QPainter::Antialiasing);


    QPen pen(
        QColor("#ff5555"),
        2.0
    );

    painter->setPen(pen);

    painter->setBrush(Qt::NoBrush);


    painter->drawLine(
        point + QPointF(-9, 0),
        point + QPointF(9, 0)
    );

    painter->drawLine(
        point + QPointF(0, -9),
        point + QPointF(0, 9)
    );


    QPolygonF diamond;

    diamond
        << point + QPointF(0, -7)
        << point + QPointF(7, 0)
        << point + QPointF(0, 7)
        << point + QPointF(-7, 0);


    painter->drawPolygon(diamond);
}

void ComponentStyleGraphicsObject::paint(
    QPainter* painter,
    const QStyleOptionGraphicsItem*,
    QWidget*
)
{
    const auto& appearance =
        component.getAppearance();


    for (int i = 0;
         i < static_cast<int>(appearance.lines.size());
         ++i)
    {
        paintLine(
            painter,
            appearance.lines[i],
            i
        );
    }


    for (int i = 0;
         i < static_cast<int>(appearance.curves.size());
         ++i)
    {
        paintCurve(
            painter,
            appearance.curves[i],
            i
        );
    }


    for (int i = 0;
         i < static_cast<int>(appearance.labels.size());
         ++i)
    {
        paintLabel(
            painter,
            appearance.labels[i],
            i
        );
    }


    paintPins(painter);

    paintAnchor(painter);
}



ComponentStyleHandleItem* ComponentStyleGraphicsObject::addPointHandle(
    StyleHitType type,
    int index,
    const QPointF& center,
    double radius
)
{
    auto* handle = new ComponentStyleHandleItem(type, index, this);

    handle->setPointHitArea(center, radius);
    handle->setZValue(handleZValueFor(type));

    handleItems.push_back(handle);

    return handle;
}

ComponentStyleHandleItem* ComponentStyleGraphicsObject::addStrokeHandle(
    StyleHitType type,
    int index,
    const QPainterPath& path,
    double width
)
{
    auto* handle = new ComponentStyleHandleItem(type, index, this);

    handle->setStrokeHitArea(path, width);
    handle->setZValue(handleZValueFor(type));

    handleItems.push_back(handle);

    return handle;
}

ComponentStyleHandleItem* ComponentStyleGraphicsObject::addRectHandle(
    StyleHitType type,
    int index,
    const QRectF& rect
)
{
    auto* handle = new ComponentStyleHandleItem(type, index, this);

    handle->setRectHitArea(rect);
    handle->setZValue(handleZValueFor(type));

    handleItems.push_back(handle);

    return handle;
}

void ComponentStyleGraphicsObject::rebuildHandles()
{
    for (auto* handle : handleItems)
        delete handle;

    handleItems.clear();


    const auto& appearance =
        component.getAppearance();



    addPointHandle(
        StyleHitType::ANCHOR,
        -1,
        appearance.anchor.getGridScaledCopy().getQPointF(),
        9.0
    );



    for (int i = 0;
         i < static_cast<int>(appearance.lines.size());
         ++i)
    {
        const auto& line = appearance.lines[i];

        const QPointF a = line.begin.getGridScaledCopy().getQPointF();
        const QPointF b = line.end.getGridScaledCopy().getQPointF();

        QPainterPath path(a);
        path.lineTo(b);

        addStrokeHandle(
            StyleHitType::LINE_MIDDLE,
            i,
            path,
            std::max(line.width, HIT_RADIUS * 2)
        );

        addPointHandle(StyleHitType::LINE_BEGIN, i, a, HANDLE_RADIUS + 3.0);
        addPointHandle(StyleHitType::LINE_END, i, b, HANDLE_RADIUS + 3.0);
    }


    /*
        Curves: body handle + begin/control1/control2/end handles.
    */

    for (int i = 0;
         i < static_cast<int>(appearance.curves.size());
         ++i)
    {
        const auto& curve = appearance.curves[i];

        const QPointF a  = curve.begin.getGridScaledCopy().getQPointF();
        const QPointF c1 = curve.control1.getGridScaledCopy().getQPointF();
        const QPointF c2 = curve.control2.getGridScaledCopy().getQPointF();
        const QPointF b  = curve.end.getGridScaledCopy().getQPointF();

        QPainterPath path(a);
        path.cubicTo(c1, c2, b);

        addStrokeHandle(
            StyleHitType::CURVE_MIDDLE,
            i,
            path,
            std::max(curve.width, HIT_RADIUS * 2)
        );

        addPointHandle(StyleHitType::CURVE_BEGIN, i, a, HANDLE_RADIUS + 3.0);
        addPointHandle(StyleHitType::CURVE_CONTROL1, i, c1, HANDLE_RADIUS);
        addPointHandle(StyleHitType::CURVE_CONTROL2, i, c2, HANDLE_RADIUS);
        addPointHandle(StyleHitType::CURVE_END, i, b, HANDLE_RADIUS + 3.0);
    }



    for (int i = 0;
         i < static_cast<int>(appearance.labels.size());
         ++i)
    {
        const auto& label = appearance.labels[i];

        const QPointF point = label.position.getGridScaledCopy().getQPointF();

        QFont font;
        font.setPointSize(label.fontSize);

        QFontMetrics metrics(font);

        QRectF textRect = metrics.boundingRect(label.text);

        textRect.moveTo(point.x(), point.y() - textRect.height());
        textRect.adjust(-2, -2, 2, 2);

        addRectHandle(StyleHitType::LABEL, i, textRect);
    }


    const auto pins = component.getPins();

    for (int i = 0; i < static_cast<int>(pins.size()); ++i)
    {
        addPointHandle(
            StyleHitType::PIN,
            i,
            pins[i]->getAppearancePosition().getGridScaledCopy().getQPointF(),
            PIN_RADIUS + 2.0
        );
    }
}