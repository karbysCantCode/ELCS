#ifndef PINGRAPHICSITEM_H
#define PINGRAPHICSITEM_H

#include <QGraphicsItem>
#include <QString>
#include <QRectF>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QPen>
#include <QWidget>

#include "component.h"

class PinGraphicsItem : public QGraphicsItem
{
public:
    PinGraphicsItem(Pin& _pin, QGraphicsItem *parent = nullptr)
        : QGraphicsItem(parent),
          name(QString::fromStdString(_pin.name)),
          pin(_pin)
    {
    }

    QRectF boundingRect() const override
    {
        return QRectF(-5, -20, 120, 40);
    }

    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *,
               QWidget *) override
    {
        QPen pen(Qt::black);
        pen.setWidthF(2);

        painter->setPen(pen);
        painter->setBrush(Qt::black);


        // dot
        painter->drawEllipse(QPointF(0,0), 4, 4);


        // line
        painter->drawLine(
            QPointF(4,0),
            QPointF(35,0)
        );


        // text box
        QRectF textRect(40,-12,80,24);

        painter->setBrush(QColor("#333333"));
        painter->drawRoundedRect(
            textRect,
            4,
            4
        );


        // text
        painter->setPen(Qt::white);
        painter->drawText(
            textRect,
            Qt::AlignCenter,
            name
        );
    }

private:
    QString name;
    Pin& pin;
};

#endif // PINGRAPHICSITEM_H
