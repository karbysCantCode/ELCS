#include "circuitworkspace.h"

#include "projectmanager.h"
#include "notifications.h"

#include "segmentgraphicsobject.h"
#include "pingraphicsobject.h"
#include "componentgraphicsobject.h"

#include <QCursor>
#include <iostream>

CircuitWorkspace::CircuitWorkspace(QWidget* parent) {
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

void CircuitWorkspace::setInteractionState(InteractionState state) {
  interactionState = state;
  if (p_selectedItem && (state != InteractionState::SELECTED_ITEM && state != InteractionState::MOVING_ITEM)) {
    p_selectedItem->setSelected(false);
    p_selectedItem->update();
    p_selectedItem = nullptr;
  }
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

    if (interactionState == InteractionState::PLACING_COMPONENT) {

        if (!p_componentToPlace)
            return;

        Position position =
            convertEventPosToPosition(event->pos());

        // Actually create the component
        globalProjectManager->addNewPropagator(std::move(p_componentToPlace->createDerivativeComponent(position)));

        // Remove ghost
        if (p_componentGhost) {
            scene()->removeItem(p_componentGhost);
            delete p_componentGhost;
            p_componentGhost = nullptr;
        }

        p_componentToPlace = nullptr;
        p_temporaryComponentToPlace = nullptr;

        setInteractionState(InteractionState::NONE);

        event->accept();
        return;
    }
    

    if (item == nullptr || item == backgroundGridItem || item->type() == AbstractGraphicsObject::Type) {
      Position nPos = convertEventPosToPosition((event->pos()));
      auto* abstractItem = dynamic_cast<AbstractGraphicsObject*>(item);
      if (state == EditingStates::POKE) {

      } else if (state == EditingStates::EDIT) {
        if ((item == nullptr || item == backgroundGridItem) && interactionState == InteractionState::SELECTED_ITEM) interactionState = InteractionState::NONE;
        qDebug() << "EDITING START" << (int)interactionState;
        switch (interactionState)
        {
        case InteractionState::NONE: {
          if (globalProjectManager->gridManager.isOccupied(nPos)) {
            globalNotificationManager->notify("Debug", "is occupied", 1000);
            setInteractionState(InteractionState::WIRING);
            if (tempWire.graphicsObject) {
              scene()->removeItem(tempWire.graphicsObject);
              delete tempWire.graphicsObject;
              tempWire.graphicsObject = nullptr;
            }

            tempWire.reset();
            tempWire.segments.push_back(Segment(nPos,nPos));
            tempWire.segments.push_back(Segment(nPos,nPos));

            tempWire.graphicsObject = new SegmentGraphicsObject(tempWire);
            scene()->addItem(tempWire.graphicsObject);
            tempWire.graphicsObject->update();
          } else {
            globalNotificationManager->notify("Debug", "NOT occupied", 1000);
          }
          event->accept();
          return;
          break;
        }
        case InteractionState::WIRING: {
          /* code */
          event->accept();
          return;
          break;
        }
        case InteractionState::SELECTED_ITEM: {
          p_itemDragStartMouse = event->pos();
          p_itemDragStartPosition = p_selectedItem->pos();
          //p_selectedItem = abstractItem;
          setInteractionState(InteractionState::MOVING_ITEM);
          qDebug() << "sefefe:";
          event->accept();
          return;
          break;
        }
        
        default:
          break;
        }
      }
    }
  } else if (event->button() == Qt::RightButton) {
    setInteractionState(InteractionState::MOVING_WORKSPACE);

    p_movementBegunQPoint = std::make_unique<QPoint>();
    p_movementBegunQPoint->setX(event->pos().x());
    p_movementBegunQPoint->setY(event->pos().y());

    p_preMoveXPosition = p_xposition;
    p_preMoveYPosition = p_yposition;
  }
  event->accept();
  return;
  QGraphicsView::mousePressEvent(event);
}

void CircuitWorkspace::mouseMoveEvent(QMouseEvent *event)  {
  qDebug() << "MOVED" << (int)interactionState;
  switch (interactionState) {
    case InteractionState::WIRING: {
      constexpr double margin = 20;
      tempWire.graphicsObject->beginGeometryChange();
      const auto eventPosTemp = convertEventPosToPosition(event->pos());
      const Position eventPosIntermediate = {tempWire.segments[0].begin.x, eventPosTemp.y};
      tempWire.segments[0].end = eventPosIntermediate;
      tempWire.segments[1].begin = eventPosIntermediate;
      tempWire.segments[1].end = eventPosTemp;

      tempWire.graphicsObject->update();
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
      event->accept();
      return;
      break;
    }
    case InteractionState::MOVING_WORKSPACE: {
      moveWorkspaceToCurrentMouse(event->pos());
      update(); // signal redraw
      std::cout << "X:" << p_xposition << " Y:" << p_yposition << " mouse moved with redraw" << std::endl;
      event->accept();
      return;
      break;
    }
    case InteractionState::MOVING_ITEM: {
      if (!p_selectedItem) break;
      QPointF mouseScenePos =
          mapToScene(event->pos());

      QPointF startMouseScenePos =
          mapToScene(p_itemDragStartMouse);

      QPointF delta =
          mouseScenePos - startMouseScenePos;

      p_selectedItem->parentPropagator->setGridPosition(
          Position::setAsScaledFromGrid(p_itemDragStartPosition + delta)
      );

      p_selectedItem->updateWorkspacePosition();

      p_selectedItem->update();
      event->accept();
      return;
      break;
    }
    case InteractionState::PLACING_COMPONENT: {
    if (!p_componentGhost)
        break;

    const Position position =
        convertEventPosToPosition(event->pos());

    p_temporaryComponentToPlace->setGridPosition(position);

    p_componentGhost->update();

    event->accept();
    return;
}
    default: break;
  }
  
  QGraphicsView::mouseMoveEvent(event);

}

void CircuitWorkspace::mouseDoubleClickEvent(QMouseEvent* event)
{
  if (event->button() != Qt::LeftButton) {
    QGraphicsView::mouseDoubleClickEvent(event);
    return;
  }

  QGraphicsItem* item = itemAt(event->pos());

  if (!item || item == backgroundGridItem) {
    p_selectedItem = nullptr;
    setInteractionState(InteractionState::NONE);
    return;
  }
  auto* abstractItem = dynamic_cast<AbstractGraphicsObject*>(item);
  if (!abstractItem) {
    p_selectedItem = nullptr;
    setInteractionState(InteractionState::NONE);
    return;
  }

  switch (interactionState) {
    case InteractionState::NONE: 
    case InteractionState::SELECTED_ITEM: {
      // Select it
      p_selectedItem = abstractItem;
      setInteractionState(InteractionState::SELECTED_ITEM);

      // Optional Qt selection state
      // p_selectedItem->setSelected(true);

      qDebug() << "Selected graphics item:"
              << p_selectedItem;

      event->accept();
      return;
      break;
    }
    default: break;
  }
  QGraphicsView::mouseDoubleClickEvent(event);
}

void CircuitWorkspace::mouseReleaseEvent(QMouseEvent *event)  {
  std::cout << "mouse up" << std::endl;
  if (event->button() == Qt::LeftButton) {
    switch (interactionState)
    {
    case InteractionState::WIRING: {
      setInteractionState(InteractionState::NONE);
      scene()->removeItem(tempWire.graphicsObject);
      tempWire.graphicsObject = nullptr;
      delete tempWire.graphicsObject;
      tempWire.graphicsObject = nullptr;

      std::vector<Segment> newSegments;
      for (auto& segment : tempWire.segments) {
        if (segment.begin == segment.end) continue;
        newSegments.push_back(segment);
      }
      tempWire.segments = std::move(newSegments);

      auto unique = std::make_unique<Wire>(tempWire);
      globalProjectManager->addNewPropagator(std::move(unique));
      event->accept();
      return;
      break;
    }
    case InteractionState::MOVING_ITEM: {
      setInteractionState(InteractionState::SELECTED_ITEM);
      event->accept();
      return;
      break;
    }

    default:
      break;
    }
  } else if (event->button() == Qt::RightButton) {
    switch (interactionState) {
      case InteractionState::MOVING_WORKSPACE: {
        setInteractionState(InteractionState::NONE);
        moveWorkspaceToCurrentMouse(event->pos());
        update(); // signal redraw

        p_movementBegunQPoint.reset();
        event->accept();
        return;
        break;
      }
      default: break;
    }
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
    const SentinelComponent& component)
{
    // Cancel any existing interaction
    if (p_componentGhost) {
        scene()->removeItem(p_componentGhost);
        delete p_componentGhost;
        p_componentGhost = nullptr;
    }

    p_selectedItem = nullptr;

    p_componentToPlace = &component;
    p_temporaryComponentToPlace = std::make_unique<Component>(component);

    setInteractionState(InteractionState::PLACING_COMPONENT);

    // Create the visual ghost
    p_componentGhost =
        new ComponentGraphicsObject(*p_temporaryComponentToPlace.get());

    p_componentGhost->setGhostMode(true);

    scene()->addItem(p_componentGhost);

    // Put it initially under the mouse
    QPoint mousePos = mapFromGlobal(QCursor::pos());
    Position position = convertEventPosToPosition(mousePos);

    p_componentGhost->setPos(
        position.getGridScaledCopy().getQPointF()
    );

    p_componentGhost->update();

    qDebug() << "Placing:"
             << QString::fromStdString(component.getName());
}

void CircuitWorkspace::onComponentEditRequested(
    const SentinelComponent& component)
{
  //please cancel any component placement!
    qDebug() << "edit reuqested"
             << QString::fromStdString(component.getName());
}