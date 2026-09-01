#ifndef CIRCUITWORKSPACE_H
#define CIRCUITWORKSPACE_H

#include <QObject>
#include <QGraphicsView>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QGraphicsPixmapItem>
#include <QResizeEvent>
#include <QWidget>
#include <QString>
#include <vector>

#include "component.h"



class AbstractGraphicsObject;
class ComponentGraphicsObject;
class PinGraphicsObject;
class SentinelComponent;

class QWidget;
class HotkeyOverlay;

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

    void setComponentRotation(Component* component, int newRotation);
    void rotateSelectedComponent();
    void setPinRotation(Pin* pin, int newRotation);

    bool isReadOnly() const {return p_readOnly;}
    void setReadOnly(bool value);

    void updateBreadcrumb(const std::vector<std::string>& path);
public slots:
    void onComponentSelected(const SentinelComponent& component);
    void onComponentEditRequested(const SentinelComponent& component);

    void startPlacingPin();

signals:
    void componentPlaced(Component* component);
    void pinPlaced(Pin* pin);
    void wirePlaced(Wire* wire);

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
    int p_height = 0; 



    int p_maxWidth = 0; 
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

    
    std::unique_ptr<Pin> p_temporaryPinToPlace;
    PinGraphicsObject* p_pinGhost = nullptr;

    InteractionState interactionState = InteractionState::NONE;
    AbstractGraphicsObject* p_selectedItem = nullptr;

    Position p_selectionGridPosition;

    QPoint p_itemDragStartMouse;
    QPointF p_itemDragStartPosition;
    

    /*
        anchors
        0: start
        1: midpoint
        2: end
    */
    Wire tempWire;
    
    std::unique_ptr<QPoint> p_movementBegunQPoint = nullptr; 

    QGraphicsScene workspaceScene;
    QGraphicsPixmapItem* backgroundGridItem = nullptr;

    Position convertEventPosToPosition(const QPoint& point) const;

    void moveWorkspaceToCurrentMouse(const QPoint& event);
    void updateWorkspacePosition();
    
    void updateViewSize();
    int getMaxXPosition() const;
    int getMaxYPosition() const;

    void deleteSelectedItem();
    void rotateSelectedItem();

    void cancelCurrentPlacement();

    void notifyReadOnly();

    bool p_readOnly = false;

    QWidget* p_breadcrumbBar = nullptr;
    void rebuildBreadcrumbBar(const std::vector<std::string>& path);
    void repositionBreadcrumbBar();

    HotkeyOverlay* p_hotkeyOverlay = nullptr;
};

#endif 