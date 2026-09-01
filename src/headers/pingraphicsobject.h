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
  
  PinGraphicsObject(Pin& _pin, QGraphicsObject *parent = nullptr)
      : AbstractGraphicsObject(&_pin, parent), name(QString::fromStdString(_pin.getName())), pin(_pin)
  {
    setZValue(1);
  }

  ~PinGraphicsObject() override
  {
    if (pin.getGraphicsObject() == this)
      pin.setGraphicsObject(nullptr);
  }

  virtual void updateWorkspacePosition() override;

  GraphicsObjectTypes graphicsObjectType() const override {return GraphicsObjectTypes::PIN;}

  QRectF boundingRect() const override;

  void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) override;

  void setGhostMode(bool enabled);
  bool ghostMode() const;

protected:
  
  
  
  
private:
  QRectF labelRect() const;

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

#endif 