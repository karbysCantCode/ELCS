#include "circuitworkspace.h"

#include "projectmanager.h"
#include "notifications.h"

#include "segmentgraphicsitem.h"
#include "pingraphicsitem.h"

#include <iostream>

CircuitWorkspace::CircuitWorkspace(QFrame*& frame) {
    std::cout << "INIT X:" << size().width() << std::endl;
    std::cout << "INIT Y:" << size().height() << std::endl;

    updateViewSize();

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setScene(&workspaceScene);

    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

}

void CircuitWorkspace::resizeEvent ( QResizeEvent * event )  {
    updateViewSize();
    //resetBackgroundGrid();
}

void CircuitWorkspace::updateViewSize() {
    p_width = size().width();
    p_height = size().height();

    p_maxWidth = p_width * 1.5; // needs handling to like actually grab circ size from a save file
    p_maxHeight = p_height * 1.5;
}

void CircuitWorkspace::drawBackground(QPainter *painter, const QRectF &rect) {
    static QPixmap tile;
    if (tile.isNull()) {
        tile = QPixmap(10, 10);
        tile.fill(Qt::white);
        QPainter tp(&tile);
        tp.setPen(Qt::lightGray);
        tp.drawPoint(5, 5);
    }
    painter->fillRect(rect, QBrush(tile));
}

void CircuitWorkspace::wheelEvent(QWheelEvent *event)  {
  
}

Position CircuitWorkspace::convertEventPosToPosition(const QPoint& point) const {
  const auto np = mapToScene(point);
  return {int(std::floor((np.x()+2.5)/10.0)),
          int(std::floor((np.y()+2.5)/10.0))};
}

void CircuitWorkspace::mousePressEvent(QMouseEvent *event) {

  if (event->button() == Qt::LeftButton) {
    QGraphicsItem* item = itemAt(event->pos());
    

    if (item == nullptr || item == backgroundGridItem || item->type() == SegmentGraphicsItem::Type || item->type() == PinGraphicsItem::Type) {
      Position nPos = convertEventPosToPosition((event->pos()));
      if (state == EditingStates::POKE) {

      } else if (state == EditingStates::EDIT) {
        if (globalProjectManager->gridManager.isOccupied(nPos)) {
          //wire
          globalNotificationManager->notify("Debug", "is occupied", 1000);
          p_isWiring = true;
          
          if (tempWire.graphicsItem) {
            scene()->removeItem(tempWire.graphicsItem);
            delete tempWire.graphicsItem;
            tempWire.graphicsItem = nullptr;
          }

          tempWire.reset();
          tempWire.segments.push_back(Segment(nPos,nPos));
          tempWire.segments.push_back(Segment(nPos,nPos));

          tempWire.graphicsItem = new SegmentGraphicsItem(tempWire);
          scene()->addItem(tempWire.graphicsItem);
          tempWire.graphicsItem->update();

          event->accept();
          return;

        }
      }
    }
    event->accept();
    return;
  } else if (event->button() == Qt::RightButton) {
    globalNotificationManager->notify("Debug", "NOT occupied", 1000);
    p_isMoving = true;

    p_movementBegunQPoint = std::make_unique<QPoint>();
    p_movementBegunQPoint->setX(event->pos().x());
    p_movementBegunQPoint->setY(event->pos().y());

    p_preMoveXPosition = p_xposition;
    p_preMoveYPosition = p_yposition;
  }
  QGraphicsView::mousePressEvent(event);
}

void CircuitWorkspace::mouseMoveEvent(QMouseEvent *event)  {
    if (p_isMoving) {
        moveWorkspaceToCurrentMouse(event->pos());
        update(); // signal redraw
        std::cout << "X:" << p_xposition << " Y:" << p_yposition << " mouse moved with redraw" << std::endl;
    }
    if (p_isWiring) {
      constexpr double margin = 20;
      tempWire.graphicsItem->beginGeometryChange();
      const auto eventPosTemp = convertEventPosToPosition(event->pos());
      const Position eventPosIntermediate = {tempWire.segments[0].begin.x, eventPosTemp.y};
      tempWire.segments[0].end = eventPosIntermediate;
      tempWire.segments[1].begin = eventPosIntermediate;
      tempWire.segments[1].end = eventPosTemp;

      tempWire.graphicsItem->update();
      // const QRectF bounds = scene()->sceneRect();
      // double leftRate =
      //     1.0 - std::clamp(std::min(p_wiringFinalPoint.x() - bounds.left(), margin) / margin, 0.0, 1.0);

      // double rightRate =
      //     1.0 - std::clamp(std::min(bounds.right() - p_wiringFinalPoint.x(), margin) / margin, 0.0, 1.0);

      // double topRate =
      //     1.0 - std::clamp(std::min(p_wiringFinalPoint.y() - bounds.top(), margin) / margin, 0.0, 1.0);

      // double bottomRate =
      //     1.0 - std::clamp(std::min(bounds.bottom() - p_wiringFinalPoint.y(), margin) / margin, 0.0, 1.0);
      // globalNotificationManager->notify("DEUBG", std::format("L{} R{} T{} B{}", leftRate,rightRate,topRate,bottomRate), 100);
      // constexpr double speed = 3;
      // p_xposition = std::clamp(int(p_xposition + speed * (rightRate - leftRate)), 0, getMaxXPosition());
      // p_yposition = std::clamp(int(p_yposition + speed * (bottomRate - topRate)), 0, getMaxYPosition());

      // updateWorkspacePosition();
    }

    QGraphicsView::mouseMoveEvent(event);

}

void CircuitWorkspace::mouseDoubleClickEvent(QMouseEvent *event) {

}
void CircuitWorkspace::mouseReleaseEvent(QMouseEvent *event)  {
    std::cout << "mouse up" << std::endl;
    if (p_isMoving) {
      p_isMoving = false;
      moveWorkspaceToCurrentMouse(event->pos());
      update(); // signal redraw

      p_movementBegunQPoint.reset();
    }

    if (p_isWiring) {
      p_isWiring = false;
      scene()->removeItem(tempWire.graphicsItem);
      tempWire.graphicsItem = nullptr;
      delete tempWire.graphicsItem;
      tempWire.graphicsItem = nullptr;

      std::vector<Segment> newSegments;
      for (auto& segment : tempWire.segments) {
        if (segment.begin == segment.end) continue;
        newSegments.push_back(segment);
      }
      tempWire.segments = std::move(newSegments);

      auto unique = std::make_unique<Wire>(tempWire);
      globalProjectManager->addNewPropagator(std::move(unique));
    }

    QGraphicsView::mouseReleaseEvent(event);
}

void CircuitWorkspace::updateWorkspacePosition() {
    setSceneRect(p_xposition, p_yposition, p_width, p_height);
}

void CircuitWorkspace::moveWorkspaceToCurrentMouse(const QPoint& event) {
    if (!p_movementBegunQPoint) return;

    const int dx = p_movementBegunQPoint->x() - event.x();
    const int dy = p_movementBegunQPoint->y() - event.y();

    p_xposition = std::clamp(p_preMoveXPosition + dx, 0, getMaxXPosition());
    p_yposition = std::clamp(p_preMoveYPosition + dy, 0, getMaxYPosition());

    updateWorkspacePosition();
}

int CircuitWorkspace::getMaxXPosition() const {
    return p_maxWidth - (int)(size().width() / p_magnification);
}
int CircuitWorkspace::getMaxYPosition() const {
    return p_maxHeight - (int)(size().height() / p_magnification);
}

void CircuitWorkspace::reset() {
  scene()->clear();
  updateWorkspacePosition();
}

void CircuitWorkspace::onComponentSelected(
    const Component& component)
{
    qDebug() << "Selected:"
             << QString::fromStdString(component.name);
}
void CircuitWorkspace::onComponentEditRequested(
    const Component& component)
{
    qDebug() << "edit reuqested"
             << QString::fromStdString(component.name);
}