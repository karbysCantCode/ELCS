#ifndef WIREGRAPHICSITEM_H
#define WIREGRAPHICSITEM_H

#include <QGraphicsItem>
#include <QString>
#include <QRectF>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QPen>
#include <QWidget>
#include <QGraphicsSceneMouseEvent>
#include <QLineEdit>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include <QFontMetrics>
#include <QObject>
#include <QLineEdit>

#include "component.h"

class WireGraphicsItem : public QGraphicsItem
{
public:
  WireGraphicsItem(Wire& _wire, QGraphicsItem *parent = nullptr)
      : QGraphicsItem(parent), wire(_wire)
  {
    setZValue(1);
    if (wire.graphicsItem)
      delete wire.graphicsItem;
    wire.graphicsItem = this;
  }

  void beginGeometryChange();

  QRectF boundingRect() const override;

  void setColor(const QColor& c);

  protected:
  void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;
  QPainterPath shape() const override;
private:
  Wire& wire;
  QColor color = Qt::black;
};

#endif // WIREGRAPHICSITEM_H