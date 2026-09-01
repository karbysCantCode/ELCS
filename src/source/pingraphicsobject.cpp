#include "pingraphicsobject.h" 

#include "projectmanager.h"

#include <algorithm>

#define PADDING 20

QRectF PinGraphicsObject::labelRect() const {
  QFontMetrics fm(QFont("Arial",12));
  int width = fm.horizontalAdvance(name) + PADDING;

  const int rotation = pin.getRotation();

  if (rotation == 90)
    return QRectF(-width / 2.0, 30, width, 24);
  if (rotation == 180)
    return QRectF(-30 - width, -12, width, 24);
  if (rotation == 270)
    return QRectF(-width / 2.0, -30 - 24, width, 24);

  return QRectF(30, -12, width, 24);
}

QRectF PinGraphicsObject::boundingRect() const {
  QRectF rect = labelRect().adjusted(-10, -8, 10, 8);

  if (!pin.getAppearanceName().empty())
  {
    QFontMetrics nameMetrics(QFont("Arial", 9, QFont::Bold));
    const int nameWidth = nameMetrics.horizontalAdvance(QString::fromStdString(pin.getAppearanceName()));

    rect.setLeft(std::min(rect.left(), -nameWidth / 2.0 - 4.0));
    rect.setTop(std::min(rect.top(), -26.0));
  }

  return rect;
}

void PinGraphicsObject::paint(
  QPainter *painter, 
  const QStyleOptionGraphicsItem *, 
  QWidget *) 
{ 
  const QColor stateCol = stateColor(pin.getEffectingState());

  QPen pen(stateCol); 
  pen.setWidthF(2); 
  painter->setPen(pen); 
  painter->setBrush(stateCol); 

  painter->save();
  painter->rotate(pin.getRotation());

  
  painter->drawEllipse(QPointF(0,0), 4, 4); 
  
  
  painter->drawLine( QPointF(4,0), QPointF(30,0) ); 

  painter->restore();
  
  
  QFont font("Arial", 12); 
  QFontMetrics fm(font); 
  QRectF textRect = labelRect();
  painter->setPen(pen);
  painter->setBrush(QColor("#333333")); 
  painter->drawRoundedRect( textRect, 4, 4 );

  if (showText) { 
    painter->setPen(Qt::white);
    painter->setFont(font);
    painter->drawText( textRect, Qt::AlignCenter, name );
  } 

  if (!pin.getAppearanceName().empty())
  {
    QFont nameFont("Arial", 9);
    nameFont.setBold(true);

    QFontMetrics nameMetrics(nameFont);
    const QString appearanceName = QString::fromStdString(pin.getAppearanceName());
    const int textWidth = nameMetrics.horizontalAdvance(appearanceName);

    painter->setFont(nameFont);
    painter->setPen(Qt::white);

    painter->drawText(
      QPointF(-textWidth / 2.0, -12),
      appearanceName
    );
  }

  if (isSelected())
  {
      paintSelectionHighlight(painter, boundingRect());
  }
} 

void PinGraphicsObject::updateWorkspacePosition() {
    const Position gridPos = pin.getGridPosition();
    const Position scaledPos = gridPos.getGridScaledCopy();
    const QPointF scenePos = scaledPos.getQPointF();

    setPos(scenePos);
}

void PinGraphicsObject::setGhostMode(bool enabled)
{
    if (ghost == enabled)
        return;

    ghost = enabled;

    setOpacity(ghost ? 0.45 : 1.0);

    update();
}

bool PinGraphicsObject::ghostMode() const
{
    return ghost;
}