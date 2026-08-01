#ifndef PINGRAPHICSITEM_H
#define PINGRAPHICSITEM_H

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

class PinGraphicsItem : public QGraphicsItem
{
public:
  PinGraphicsItem(Pin& _pin, QGraphicsItem *parent = nullptr)
      : QGraphicsItem(parent), name(QString::fromStdString(_pin.name)), pin(_pin)
  {
    setZValue(1);
  }

  QRectF boundingRect() const override;

  void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;
protected:
  void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;
  void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
  void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
  void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
private:
  QString name;
  Pin& pin;
  bool showText = true;
  QLineEdit* currentEdit = nullptr;
  bool moving = false;
  QPointF dragOffset;
  bool pressed = false;
  QPointF pressPosition;
};

#endif // PINGRAPHICSITEM_H

