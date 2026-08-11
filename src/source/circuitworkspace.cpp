#include "circuitworkspace.h"

#include "projectmanager.h"
#include "notifications.h"

#include "segmentgraphicsobject.h"
#include "pingraphicsobject.h"
#include "componentgraphicsobject.h"

#include <QCursor>
#include <QKeyEvent>
#include <iostream>

CircuitWorkspace::CircuitWorkspace(QWidget* parent) {
    std::cout << "INIT X:" << size().width() << std::endl;
    std::cout << "INIT Y:" << size().height() << std::endl;

    updateViewSize();

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setScene(&workspaceScene);

    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

    setFocusPolicy(Qt::StrongFocus);

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

    if (interactionState == InteractionState::PLACING_PIN) {

        if (!p_temporaryPinToPlace) {
            event->accept();
            return;
        }

        const Position position =
            convertEventPosToPosition(event->pos());

        p_temporaryPinToPlace->setGridPosition(position);

        SentinelComponent* openComponent = globalProjectManager->currentOpenComponent;

        // Hands ownership to the currently open component;
        // addNewPropagator() registers it in the grid and creates
        // its real (un-ghosted) PinGraphicsObject -- the ghost item
        // itself is just discarded below.
        globalProjectManager->addNewPropagator(std::move(p_temporaryPinToPlace));

        if (p_pinGhost) {
            scene()->removeItem(p_pinGhost);
            delete p_pinGhost;
            p_pinGhost = nullptr;
        }

        p_temporaryPinToPlace = nullptr;

        if (openComponent) {
            openComponent->notifyInstancesOfStructureChange();
        }

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
    case InteractionState::PLACING_PIN: {
        if (!p_pinGhost || !p_temporaryPinToPlace)
            break;

        const Position position =
            convertEventPosToPosition(event->pos());

        p_temporaryPinToPlace->setGridPosition(position);
        p_pinGhost->updateWorkspacePosition();
        p_pinGhost->update();

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
    setInteractionState(InteractionState::NONE);
    return;
  }
  auto* abstractItem = dynamic_cast<AbstractGraphicsObject*>(item);
  if (!abstractItem) {
    setInteractionState(InteractionState::NONE);
    return;
  }

  switch (interactionState) {
    case InteractionState::NONE: 
    case InteractionState::SELECTED_ITEM: {
      if (p_selectedItem && p_selectedItem != abstractItem) {
          p_selectedItem->setSelected(false);
          p_selectedItem->update();
      }

      // Select it
      p_selectedItem = abstractItem;
      p_selectionGridPosition = convertEventPosToPosition(event->pos());

      p_selectedItem->setSelected(true);
      p_selectedItem->update();

      setInteractionState(InteractionState::SELECTED_ITEM);

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

void CircuitWorkspace::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Delete || event->key() == Qt::Key_Backspace) {
        deleteSelectedItem();
        event->accept();
        return;
    }

    if (event->key() == Qt::Key_Escape) {
        cancelCurrentPlacement();
        event->accept();
        return;
    }

    QGraphicsView::keyPressEvent(event);
}

void CircuitWorkspace::deleteSelectedItem()
{
    if (!p_selectedItem || !globalProjectManager || !globalProjectManager->currentOpenComponent)
        return;

    AbstractGraphicsObject* item = p_selectedItem;

    p_selectedItem = nullptr;
    setInteractionState(InteractionState::NONE);

    switch (item->graphicsObjectType())
    {
        case AbstractGraphicsObject::GraphicsObjectTypes::PIN:
        {
            auto* pin = static_cast<Pin*>(item->parentPropagator);

            globalProjectManager->gridManager.removeFromGrid(pin->getGridPosition(), pin);
            globalProjectManager->currentOpenComponent->removePropagator(pin);

            globalProjectManager->currentOpenComponent->notifyInstancesOfStructureChange();
            break;
        }

        case AbstractGraphicsObject::GraphicsObjectTypes::COMPONENT:
        {
            auto* component = static_cast<Component*>(item->parentPropagator);

            for (Pin* pin : component->getPins()) {
                globalProjectManager->gridManager.removeFromGrid(component->getGridPosition() + pin->getAppearancePosition(), pin);
            }

            globalProjectManager->currentOpenComponent->removePropagator(component);
            break;
        }

        case AbstractGraphicsObject::GraphicsObjectTypes::SEGMENT:
        {
            auto* wire = static_cast<Wire*>(item->parentPropagator);
            auto* wireGraphics = wire->graphicsObject;

            const int index = wire->nearestSegmentIndex(p_selectionGridPosition);

            globalProjectManager->gridManager.removeFromGrid(wire->segments, wire);

            if (wireGraphics) {
                wireGraphics->beginGeometryChange();
            }

            if (index >= 0) {
                wire->removeSegmentAt(static_cast<size_t>(index));
            }

            if (wire->segments.empty()) {
                globalProjectManager->currentOpenComponent->removePropagator(wire);
            } else {
                globalProjectManager->gridManager.addToGrid(wire->segments, wire);

                if (wireGraphics) {
                    wireGraphics->update();
                }
            }

            break;
        }
    }
}

void CircuitWorkspace::cancelCurrentPlacement()
{
    if (p_componentGhost) {
        scene()->removeItem(p_componentGhost);
        delete p_componentGhost;
        p_componentGhost = nullptr;
    }
    p_componentToPlace = nullptr;
    p_temporaryComponentToPlace = nullptr;

    if (p_pinGhost) {
        scene()->removeItem(p_pinGhost);
        delete p_pinGhost;
        p_pinGhost = nullptr;
    }
    p_temporaryPinToPlace = nullptr;

    setInteractionState(InteractionState::NONE);
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
    // Cancel any existing interaction (component OR pin placement).
    cancelCurrentPlacement();

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
  cancelCurrentPlacement();

    qDebug() << "edit reuqested"
             << QString::fromStdString(component.getName());
}

void CircuitWorkspace::startPlacingPin()
{
    if (!globalProjectManager || !globalProjectManager->currentOpenComponent)
        return;

    // Cancel any existing interaction (component OR pin placement).
    cancelCurrentPlacement();

    p_temporaryPinToPlace = std::make_unique<Pin>(*globalProjectManager->currentOpenComponent);
    p_temporaryPinToPlace->setName("pin");

    setInteractionState(InteractionState::PLACING_PIN);

    p_pinGhost = new PinGraphicsObject(*p_temporaryPinToPlace);
    p_pinGhost->setGhostMode(true);

    scene()->addItem(p_pinGhost);

    // Put it initially under the mouse
    const QPoint mousePos = mapFromGlobal(QCursor::pos());
    const Position position = convertEventPosToPosition(mousePos);

    p_temporaryPinToPlace->setGridPosition(position);
    p_pinGhost->updateWorkspacePosition();
    p_pinGhost->update();

    qDebug() << "Placing a new pin";
}