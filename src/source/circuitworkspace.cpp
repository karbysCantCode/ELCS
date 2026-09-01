#include "circuitworkspace.h"

#include "projectmanager.h"
#include "notifications.h"

#include "segmentgraphicsobject.h"
#include "pingraphicsobject.h"
#include "componentgraphicsobject.h"

#include <QCursor>
#include <QKeyEvent>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <iostream>
#include <format>

#include "styles.h"
#include "hotkeymanager.h"
#include "hotkeyoverlay.h"

CircuitWorkspace::CircuitWorkspace(QWidget* parent) {
    std::cout << "INIT X:" << size().width() << std::endl;
    std::cout << "INIT Y:" << size().height() << std::endl;

    updateViewSize();

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setScene(&workspaceScene);

    setMouseTracking(true);
    viewport()->setMouseTracking(true);

    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

    
    
    setFocusPolicy(Qt::StrongFocus);

    p_breadcrumbBar = new QWidget(this);
    p_breadcrumbBar->setStyleSheet(STYLESHEET_WIDGET_SECONDARY);
    p_breadcrumbBar->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    p_breadcrumbBar->setVisible(false);
    p_breadcrumbBar->raise();

    p_hotkeyOverlay = new HotkeyOverlay("simulator", this);

    globalHotkeyManager->registerHotkey("simulator", QKeySequence(Qt::Key_Delete), "Delete selected item");
    globalHotkeyManager->registerHotkey("simulator", QKeySequence(Qt::Key_R), "Rotate selected item");
    globalHotkeyManager->registerHotkey("simulator", QKeySequence(Qt::Key_Escape), "Cancel placement / step out of nested view");

}

void CircuitWorkspace::resizeEvent ( QResizeEvent * event )  {
    updateViewSize();
    repositionBreadcrumbBar();
    if (p_hotkeyOverlay)
        p_hotkeyOverlay->reposition();
    
}

void CircuitWorkspace::setReadOnly(bool value)
{
    qDebug() << "[DEBUG] CircuitWorkspace::setReadOnly called, value=" << value;
    p_readOnly = value;
}

void CircuitWorkspace::notifyReadOnly()
{
    globalNotificationManager->notify(
        "Read-Only View",
        "This component instance is opened read-only. Return to an ancestor circuit to make edits.",
        2000
    );
}

void CircuitWorkspace::repositionBreadcrumbBar()
{
    if (!p_breadcrumbBar)
        return;

    p_breadcrumbBar->adjustSize();
    p_breadcrumbBar->move(width() - p_breadcrumbBar->width() - 8, 8);
    p_breadcrumbBar->raise();
}

void CircuitWorkspace::updateBreadcrumb(const std::vector<std::string>& path)
{
    rebuildBreadcrumbBar(path);
    repositionBreadcrumbBar();
}

void CircuitWorkspace::rebuildBreadcrumbBar(const std::vector<std::string>& path)
{
    if (!p_breadcrumbBar)
        return;

    delete p_breadcrumbBar->layout();

    for (QObject* obj : p_breadcrumbBar->children()) {
        if (auto* widget = qobject_cast<QWidget*>(obj))
            widget->deleteLater();
    }

    if (path.size() <= 1) {
        p_breadcrumbBar->setVisible(false);
        return;
    }

    auto* layout = new QHBoxLayout(p_breadcrumbBar);
    layout->setContentsMargins(6, 4, 6, 4);
    layout->setSpacing(4);

    for (size_t i = 0; i < path.size(); i++) {
        if (i > 0) {
            auto* separator = new QLabel(QString::fromUtf8("\u203A"));
            separator->setStyleSheet(STYLESHEET_LABEL_SECONDARY);
            layout->addWidget(separator);
        }

        const bool isCurrent = (i == path.size() - 1);

        auto* crumb = new QPushButton(QString::fromStdString(path[i]));
        crumb->setFlat(true);
        crumb->setCursor(isCurrent ? Qt::ArrowCursor : Qt::PointingHandCursor);
        crumb->setEnabled(!isCurrent);
        crumb->setStyleSheet(
            isCurrent
                ? "QPushButton { color: #f3f4f6; font-weight: 600; border: none; background: transparent; }"
                : "QPushButton { color: #5da0ee; border: none; background: transparent; }"
                  "QPushButton:hover { color: #79b4f5; text-decoration: underline; }"
        );

        if (!isCurrent) {
            const size_t depth = i;
            QObject::connect(crumb, &QPushButton::clicked, [depth]() {
                if (globalProjectManager)
                    globalProjectManager->exitToViewDepth(depth);
            });
        }

        layout->addWidget(crumb);
    }

    p_breadcrumbBar->setLayout(layout);
    p_breadcrumbBar->setVisible(true);
}

void CircuitWorkspace::updateViewSize() {
    p_width = size().width();
    p_height = size().height();

    p_maxWidth = p_width * 1.5; 
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

  qDebug() << "[DEBUG] CircuitWorkspace::mousePressEvent reached, button=" << event->button()
           << " isReadOnly=" << isReadOnly()
           << " interactionState=" << static_cast<int>(interactionState)
           << " state=" << static_cast<int>(state);

  if (event->button() == Qt::LeftButton) {
    QGraphicsItem* item = itemAt(event->pos());

    if (interactionState == InteractionState::PLACING_COMPONENT) {

        if (!p_componentToPlace)
            return;

        Position position =
            convertEventPosToPosition(event->pos());

        
        AbstractPropagator* placed =
            globalProjectManager->addNewPropagator(std::move(p_componentToPlace->createDerivativeComponent(position)));

        emit componentPlaced(static_cast<Component*>(placed));

        
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

        
        
        
        
        
        
        p_temporaryPinToPlace->setAppearancePosition(position);

        SentinelComponent* openComponent = globalProjectManager->currentOpenComponent;

        
        
        
        
        AbstractPropagator* placed =
            globalProjectManager->addNewPropagator(std::move(p_temporaryPinToPlace));

        emit pinPlaced(static_cast<Pin*>(placed));

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
        if (isReadOnly()) {
          notifyReadOnly();
          event->accept();
          return;
        }
        if (abstractItem && abstractItem->graphicsObjectType() == AbstractGraphicsObject::PIN) {
          Pin* pin = static_cast<Pin*>(abstractItem->parentPropagator);
          if (pin) {
            States newState = pin->getEffectingState() == States::LOW ? States::HIGH : States::LOW;
            pin->poke(newState);
            abstractItem->update();
          }
        }
        event->accept();
        return;
      } else if (state == EditingStates::EDIT) {
        if ((item == nullptr || item == backgroundGridItem) && interactionState == InteractionState::SELECTED_ITEM) interactionState = InteractionState::NONE;
        qDebug() << "EDITING START" << (int)interactionState;
        switch (interactionState)
        {
        case InteractionState::NONE: {
          if (isReadOnly()) {
            notifyReadOnly();
            event->accept();
            return;
          }
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
          event->accept();
          return;
          break;
        }
        case InteractionState::SELECTED_ITEM: {
          if (isReadOnly()) {
            notifyReadOnly();
            event->accept();
            return;
          }
          p_itemDragStartMouse = event->pos();
          p_itemDragStartPosition = p_selectedItem->pos();
          
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
      update(); 
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

    
    
    
    
    p_componentGhost->updateWorkspacePosition();
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
    emit selectionCleared();
    return;
  }
  auto* abstractItem = dynamic_cast<AbstractGraphicsObject*>(item);
  if (!abstractItem) {
    setInteractionState(InteractionState::NONE);
    emit selectionCleared();
    return;
  }

  switch (interactionState) {
    case InteractionState::SELECTED_ITEM: {
      if (abstractItem == p_selectedItem &&
          abstractItem->graphicsObjectType() == AbstractGraphicsObject::COMPONENT)
      {
        auto* component = static_cast<Component*>(abstractItem->parentPropagator);

        p_selectedItem->setSelected(false);
        p_selectedItem->update();
        p_selectedItem = nullptr;
        setInteractionState(InteractionState::NONE);

        globalProjectManager->enterComponentInstance(component);

        event->accept();
        return;
      }
      [[fallthrough]];
    }
    case InteractionState::NONE: {
      
      
      if (p_selectedItem && p_selectedItem != abstractItem) {
          p_selectedItem->setSelected(false);
          p_selectedItem->update();
      }

      
      p_selectedItem = abstractItem;
      p_selectionGridPosition = convertEventPosToPosition(event->pos());

      p_selectedItem->setSelected(true);
      p_selectedItem->update();

      setInteractionState(InteractionState::SELECTED_ITEM);

      emit itemSelected(p_selectedItem->parentPropagator);

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
      AbstractPropagator* placed = globalProjectManager->addNewPropagator(std::move(unique));
      emit wirePlaced(static_cast<Wire*>(placed));
      event->accept();
      return;
      break;
    }
    case InteractionState::MOVING_ITEM: {
      if (p_selectedItem && p_selectedItem->parentPropagator) {
        auto reconnect = [](Propagator* prop, const Position& oldPos, const Position& newPos) {
          auto disconnected = globalProjectManager->gridManager.removeFromGrid(oldPos, prop);
          auto connected = globalProjectManager->gridManager.addToGrid(newPos, prop);

          prop->propagate();

          for (auto* neighbor : disconnected) {
            neighbor->propagate();
            neighbor->refreshGraphics();
          }
          for (auto* neighbor : connected) {
            neighbor->propagate();
            neighbor->refreshGraphics();
          }
        };

        if (p_selectedItem->parentPropagator->isAbstract()) {
          auto* component = static_cast<Component*>(p_selectedItem->parentPropagator);
          const Position oldGridPosition = Position::setAsScaledFromGrid(p_itemDragStartPosition);
          const Position newGridPosition = component->getGridPosition();

          if (oldGridPosition != newGridPosition) {
            for (auto* pin : component->getPins()) {
              const Position rotatedAppearancePosition = pin->getAppearancePosition().getRotatedCopy(component->getRotation());
              reconnect(pin, oldGridPosition + rotatedAppearancePosition, newGridPosition + rotatedAppearancePosition);
            }
            p_selectedItem->update();
          }
        } else {
          auto* propagator = static_cast<Propagator*>(p_selectedItem->parentPropagator);
          if (propagator->getKind() == Propagator::Kinds::PIN) {
            const Position oldGridPosition = Position::setAsScaledFromGrid(p_itemDragStartPosition);
            const Position newGridPosition = propagator->getGridPosition();

            if (oldGridPosition != newGridPosition) {
              reconnect(propagator, oldGridPosition, newGridPosition);
              p_selectedItem->update();
            }
          }
        }
      }
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
        update(); 

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
        const bool hasActivePlacement =
            p_componentGhost != nullptr ||
            p_pinGhost != nullptr ||
            interactionState == InteractionState::WIRING;

        if (!hasActivePlacement && globalProjectManager && globalProjectManager->isViewingInstance()) {
            globalProjectManager->exitOneViewLevel();
        } else {
            cancelCurrentPlacement();
        }

        event->accept();
        return;
    }

    if (event->key() == Qt::Key_R) {
        rotateSelectedItem();
        event->accept();
        return;
    }

    QGraphicsView::keyPressEvent(event);
}

void CircuitWorkspace::rotateSelectedItem()
{
    if (!p_selectedItem || !p_selectedItem->parentPropagator)
        return;

    if (p_selectedItem->parentPropagator->isAbstract()) {
        auto* component = static_cast<Component*>(p_selectedItem->parentPropagator);
        setComponentRotation(component, component->getRotation() + 90);
        return;
    }

    auto* propagator = static_cast<Propagator*>(p_selectedItem->parentPropagator);

    if (propagator->getKind() == Propagator::Kinds::PIN) {
        auto* pin = static_cast<Pin*>(propagator);
        setPinRotation(pin, pin->getRotation() + 90);
    }
}

void CircuitWorkspace::setPinRotation(Pin* pin, int newRotation)
{
    if (!pin)
        return;

    if (isReadOnly()) {
        notifyReadOnly();
        return;
    }

    pin->setRotation(newRotation);
    pin->refreshGraphics();
}

void CircuitWorkspace::setComponentRotation(Component* component, int newRotation)
{
    if (!component)
        return;

    if (isReadOnly()) {
        notifyReadOnly();
        return;
    }

    auto pins = component->getPins();

    std::vector<Position> oldAbsolutePositions;
    for (auto* pin : pins)
        oldAbsolutePositions.push_back(component->getAbsolutePinPosition(*pin));

    component->setRotation(newRotation);

    for (size_t i = 0; i < pins.size(); i++) {
        auto* pin = pins[i];
        const Position newAbsolutePosition = component->getAbsolutePinPosition(*pin);

        auto disconnected = globalProjectManager->gridManager.removeFromGrid(oldAbsolutePositions[i], pin);
        auto connected = globalProjectManager->gridManager.addToGrid(newAbsolutePosition, pin);

        pin->propagate();

        for (auto* neighbor : disconnected) {
            neighbor->propagate();
            neighbor->refreshGraphics();
        }
        for (auto* neighbor : connected) {
            neighbor->propagate();
            neighbor->refreshGraphics();
        }
    }

    if (component->getGraphicsObject())
        component->getGraphicsObject()->update();
}

void CircuitWorkspace::deleteSelectedItem()
{
    if (!p_selectedItem || !globalProjectManager || !globalProjectManager->currentOpenComponent)
        return;

    if (isReadOnly()) {
        notifyReadOnly();
        return;
    }

    AbstractGraphicsObject* item = p_selectedItem;

    
    
    p_selectedItem = nullptr;
    setInteractionState(InteractionState::NONE);

    switch (item->graphicsObjectType())
    {
        case AbstractGraphicsObject::GraphicsObjectTypes::PIN:
        {
            auto* pin = static_cast<Pin*>(item->parentPropagator);

            emit itemDeleted(pin);

            globalProjectManager->gridManager.removeFromGrid(pin->getGridPosition(), pin);
            globalProjectManager->currentOpenComponent->removePropagator(pin);

            
            
            
            
            globalProjectManager->currentOpenComponent->notifyInstancesOfStructureChange();
            break;
        }

        case AbstractGraphicsObject::GraphicsObjectTypes::COMPONENT:
        {
            auto* component = static_cast<Component*>(item->parentPropagator);

            emit itemDeleted(component);

            for (Pin* pin : component->getPins()) {
                globalProjectManager->gridManager.removeFromGrid(
                    component->getGridPosition() + pin->getAppearancePosition(),
                    pin
                );
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
                emit itemDeleted(wire);
                globalProjectManager->currentOpenComponent->removePropagator(wire);
                break;
            }

            auto groups = wire->findConnectedSegmentGroups();

            wire->segments = groups.front();
            globalProjectManager->gridManager.addToGrid(wire->segments, wire);

            if (wireGraphics) {
                wireGraphics->update();
            }

            emit wireModified(wire);

            for (size_t i = 1; i < groups.size(); i++) {
                auto splitWire = std::make_unique<Wire>();
                splitWire->segments = groups[i];

                AbstractPropagator* placed = globalProjectManager->addNewPropagator(std::move(splitWire));
                emit wirePlaced(static_cast<Wire*>(placed));
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
  cancelCurrentPlacement();

  if (tempWire.graphicsObject) {
    scene()->removeItem(tempWire.graphicsObject);
    delete tempWire.graphicsObject;
    tempWire.graphicsObject = nullptr;
  }
  tempWire.reset();

  scene()->clear();
  backgroundGridItem = nullptr;

  updateWorkspacePosition();
}

void CircuitWorkspace::onComponentSelected(
    const SentinelComponent& component)
{
    if (isReadOnly())
    {
        notifyReadOnly();
        return;
    }

    if (globalProjectManager->currentOpenComponent == &component)
    {
        globalNotificationManager->notify(
            "Can't Place Component",
            std::format(
                "\"{}\" can't be placed inside its own circuit.",
                component.getName()
            )
        );

        return;
    }

    
    cancelCurrentPlacement();

    p_componentToPlace = &component;
    p_temporaryComponentToPlace = std::make_unique<Component>(component);

    setInteractionState(InteractionState::PLACING_COMPONENT);

    
    p_componentGhost =
        new ComponentGraphicsObject(*p_temporaryComponentToPlace.get());

    p_componentGhost->setGhostMode(true);

    scene()->addItem(p_componentGhost);

    
    QPoint mousePos = mapFromGlobal(QCursor::pos());
    Position position = convertEventPosToPosition(mousePos);

    p_temporaryComponentToPlace->setGridPosition(position);
    p_componentGhost->updateWorkspacePosition();

    p_componentGhost->update();

    qDebug() << "Placing:"
             << QString::fromStdString(component.getName());
}

void CircuitWorkspace::onComponentEditRequested(
    const SentinelComponent& component)
{
  
  cancelCurrentPlacement();

    qDebug() << "edit reuqested"
             << QString::fromStdString(component.getName());
}

void CircuitWorkspace::startPlacingPin()
{
    if (!globalProjectManager || !globalProjectManager->currentOpenComponent)
        return;

    if (isReadOnly())
    {
        notifyReadOnly();
        return;
    }

    
    cancelCurrentPlacement();

    p_temporaryPinToPlace = std::make_unique<Pin>(*globalProjectManager->currentOpenComponent);
    p_temporaryPinToPlace->setName("pin");
    p_temporaryPinToPlace->setIODirection(Pin::IODirection::INPUT);
    p_temporaryPinToPlace->poke(States::LOW);

    setInteractionState(InteractionState::PLACING_PIN);

    p_pinGhost = new PinGraphicsObject(*p_temporaryPinToPlace);
    p_pinGhost->setGhostMode(true);

    scene()->addItem(p_pinGhost);

    
    const QPoint mousePos = mapFromGlobal(QCursor::pos());
    const Position position = convertEventPosToPosition(mousePos);

    p_temporaryPinToPlace->setGridPosition(position);
    p_pinGhost->updateWorkspacePosition();
    p_pinGhost->update();

    qDebug() << "Placing a new pin";
}

void CircuitWorkspace::setState(EditingStates _state) {
  state = _state;
  
}