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
#include "abstractgraphicsobject.h"

class PinGraphicsObject : public AbstractGraphicsObject
{
  Q_OBJECT
public:
  // enum { Type = UserType + 3 };
  PinGraphicsObject(Pin& _pin, QGraphicsObject *parent = nullptr)
      : AbstractGraphicsObject(&_pin, parent), name(QString::fromStdString(_pin.getName())), pin(_pin)
  {
    setZValue(1);
  }

  virtual void updateWorkspacePosition() override;

  GraphicsObjectTypes graphicsObjectType() const override {return GraphicsObjectTypes::PIN;}

  QRectF boundingRect() const override;

  void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;

  // Ghosted while being placed (startPlacingPin()'s follow-the-cursor
  // preview) -- reduced opacity so it reads as "not committed yet",
  // same idiom as ComponentGraphicsObject's ghost mode.
  void setGhostMode(bool enabled);
  bool ghostMode() const;

protected:
  // int type() const override
  // {
  //     return PinGraphicsObject::Type;
  // }
private:
  QString name;
  Pin& pin;
  bool showText = true;
  QLineEdit* currentEdit = nullptr;
  bool moving = false;
  QPointF dragOffset;
  bool pressed = false;
  QPointF pressPosition;

  bool ghost = false;
};

#endif // PINGRAPHICSITEM_H