#ifndef CIRCUITWORKSPACE_H
#define CIRCUITWORKSPACE_H

#include <QObject>
#include <QGraphicsView>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QGraphicsPixmapItem>
#include <QResizeEvent>

#include "component.h"

// #include "wiregraphicsobject.h"

class AbstractGraphicsObject;
class ComponentGraphicsObject;
class PinGraphicsObject;
class SentinelComponent;

class CircuitWorkspace : public QGraphicsView
{
    Q_OBJECT
public:
    explicit CircuitWorkspace(QWidget* parent = nullptr);
    ~CircuitWorkspace() = default;

    void reset();
    
    enum class EditingStates {
        EDIT,
        POKE,
    };

    void setState(EditingStates _state);
    inline EditingStates getState() const {return state;}
public slots:
    void onComponentSelected(const SentinelComponent& component);
    void onComponentEditRequested(const SentinelComponent& component);

    /*
        Begins placing a new pin on whichever component is currently
        open for editing. A ghosted PinGraphicsObject follows the
        cursor (same idiom as onComponentSelected()'s ghost) until
        the next left click, which finalizes it into a real pin via
        ProjectManager::addNewPropagator() and un-ghosts by simply
        handing off to the normal (non-ghost) registration path.
        Escape cancels. Wire placement is untouched -- it keeps its
        existing click-drag-release flow.
    */
    void startPlacingPin();

signals:
    /*
        Tutorial / scripting hooks. The workspace has no idea
        tutorials exist -- these just report "something changed" so
        anything (a TutorialManager, logging, whatever) can react.

        Pointers are valid only for the duration of the signal
        emission. itemDeleted()/wireModified() in particular are
        emitted *before* the underlying change actually happens
        (deletion/segment removal), specifically so a listener can
        still safely read or pointer-compare it. Don't store the
        pointer past the connected slot call.
    */
    void componentPlaced(Component* component);
    void pinPlaced(Pin* pin);
    void wirePlaced(Wire* wire);

    // A wire lost a segment but is still alive (had more than one
    // segment left). Full removal of a wire goes through
    // itemDeleted() instead.
    void wireModified(Wire* wire);

    void itemDeleted(AbstractPropagator* propagator);
    void itemSelected(AbstractPropagator* propagator);
    void selectionCleared();

protected:
    void resizeEvent ( QResizeEvent * event ) override;
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void drawBackground(QPainter *painter, const QRectF &rect) override;
private:
    EditingStates state = EditingStates::EDIT;

    int p_width = 0;
    int p_height = 0; // TODO set to window size?



    int p_maxWidth = 0; // current * growth rate?
    int p_maxHeight = 0;

    int p_xposition = 0;
    int p_yposition = 0;

    int p_preMoveXPosition = 0;
    int p_preMoveYPosition = 0;
    float p_magnification = 1;

    enum class InteractionState {
        NONE,
        MOVING_WORKSPACE,
        WIRING,
        SELECTED_ITEM,
        MOVING_ITEM,
        PLACING_COMPONENT,
        PLACING_PIN
    };

    void setInteractionState(InteractionState state);

    std::unique_ptr<Component> p_temporaryComponentToPlace;
    const SentinelComponent* p_componentToPlace = nullptr;
    ComponentGraphicsObject* p_componentGhost = nullptr;

    // Pin placement -- mirrors the component-ghost members above.
    std::unique_ptr<Pin> p_temporaryPinToPlace;
    PinGraphicsObject* p_pinGhost = nullptr;

    InteractionState interactionState = InteractionState::NONE;
    AbstractGraphicsObject* p_selectedItem = nullptr;

    Position p_selectionGridPosition;

    QPoint p_itemDragStartMouse;
    QPointF p_itemDragStartPosition;
    // WireGraphicsItem* tempWireItem = nullptr;

    /*
        anchors
        0: start
        1: midpoint
        2: end
    */
    Wire tempWire;
    
    std::unique_ptr<QPoint> p_movementBegunQPoint = nullptr; // unique ptr only used for the null state + safety

    QGraphicsScene workspaceScene;
    QGraphicsPixmapItem* backgroundGridItem = nullptr;

    Position convertEventPosToPosition(const QPoint& point) const;

    void moveWorkspaceToCurrentMouse(const QPoint& event);
    void updateWorkspacePosition();
    // void resetBackgroundGrid();
    void updateViewSize();
    int getMaxXPosition() const;
    int getMaxYPosition() const;

    void deleteSelectedItem();

    void cancelCurrentPlacement();
};

#endif // CIRCUITWORKSPACE_H