#include "pingraphicsitem.h" 

#include "projectmanager.h"

#define PADDING 20

QRectF PinGraphicsItem::boundingRect() const {
  QFontMetrics fm(QFont("Arial",12));
  int width = fm.horizontalAdvance(name) + PADDING;

  return QRectF(30,-20,width,40);
}

void PinGraphicsItem::paint(
  QPainter *painter, 
  const QStyleOptionGraphicsItem *, 
  QWidget *) 
{ 
  QPen pen(Qt::black); 
  pen.setWidthF(2); 
  painter->setPen(pen); 
  painter->setBrush(Qt::black); 
  
  // dot 
  painter->drawEllipse(QPointF(0,0), 4, 4); 
  
  // line 
  painter->drawLine( QPointF(4,0), QPointF(30,0) ); 
  
  // text box 
  QFont font("Arial", 12); 
  QFontMetrics fm(font); 
  QRectF textRect(30,-12,fm.horizontalAdvance(name) + PADDING ,24); 
  painter->setBrush(QColor("#333333")); 
  painter->drawRoundedRect( textRect, 4, 4 );

  if (showText) { 
    painter->setPen(Qt::white);
    painter->setFont(font);
    painter->drawText( textRect, Qt::AlignCenter, name );
  } 
} 
  
// void PinGraphicsItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) 
// { 
//   if (event->button() == Qt::LeftButton && !currentEdit) { 
//     qDebug("double");
//     moving = false;
//     pressed = false;
//     ungrabMouse(); 

//     showText = false;
//     currentEdit = new QLineEdit;
//     currentEdit->setText(name);
//     QFont font("Arial", 12); 
//     currentEdit->setFont(font);

//     QGraphicsProxyWidget* proxy = scene()->addWidget(currentEdit);
//     proxy->setParentItem(this);
//     proxy->setPos(30, -12);
//     proxy->resize(10, 24);

//     currentEdit->setStyleSheet(
//       "QLineEdit {"
//       "    background-color: #333333;"
//       "    color: white;"
//       "    border: 2px solid black;"
//       "    border-radius: 4px;"
//       "    padding-left: 4px;"
//       "}"
//     );

//     auto resizeEdit = [this, proxy]() { 
//       QFontMetrics fm(this->currentEdit->font());
//       name = this->currentEdit->text();
//       int width = fm.horizontalAdvance(this->currentEdit->text()) + PADDING;
//       this->prepareGeometryChange();
//       this->update();
//       proxy->resize(width, 24);
//     };
    
//     resizeEdit();

//     QObject::connect(currentEdit, &QLineEdit::textChanged, [resizeEdit]() { 
//       resizeEdit();
//     });
//     QObject::connect(currentEdit, &QLineEdit::editingFinished, 
//       [this, proxy]() { 
//         name = this->currentEdit->text();
//         showText = true;

//         currentEdit->clearFocus();
//         proxy->deleteLater();

//         currentEdit = nullptr;

//         update();
//       }
//     );

//     currentEdit->setFocus(Qt::OtherFocusReason);
//     event->accept();
//     return;
//   } 
//   QGraphicsItem::mouseDoubleClickEvent(event);
// }

// void PinGraphicsItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
// {
//   if (currentEdit) {
//     QGraphicsItem::mousePressEvent(event);
//     return;
//   }

//   if(event->button() == Qt::LeftButton)
//   {
//     qDebug("A");
//     pressed = true;
//     moving = false;

//     scene()->clearFocus();

//     pressPosition = event->scenePos();
//     dragOffset = event->pos();

//     event->accept();
//     return;
//   }

//   QGraphicsItem::mousePressEvent(event);
// }

// void PinGraphicsItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
// {
//   if(pressed)
//   {
//     if(!moving &&
//       (event->scenePos()-pressPosition).manhattanLength() > 5)
//     {
//       moving = true;
//       globalProjectManager->gridManager.removeFromGrid(pin.relPosition, &pin);
//     }

//     if(moving) {
//       auto pos = event->scenePos()-dragOffset;
//       Position nPos = {int(std::floor((pos.x()-5)/10.0)),
//                        int(std::floor((pos.y()-5)/10.0))};
//       qDebug(std::format("{}x {}y :old {}x {}y", nPos.x,nPos.y, pin.relPosition.x, pin.relPosition.y).c_str());
//       if (nPos != pin.relPosition) {
//         // globalProjectManager->gridManager.removeFromGrid(pin.relPosition, &pin);
//         // globalProjectManager->gridManager.addToGrid(nPos, &pin);
//         pin.relPosition = nPos;
//         setPos(pin.qGridPosition());
//         prepareGeometryChange();
//         update();
//       }
//     }

//     event->accept();
//   }
// }

// void PinGraphicsItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
  if (event->button() == Qt::LeftButton) {
    pressed = false;
    if (moving) {
      moving = false;
      globalProjectManager->gridManager.addToGrid(pin.relPosition, &pin);
    }
    event->accept();
    return;
  }

  QGraphicsItem::mouseReleaseEvent(event);
}