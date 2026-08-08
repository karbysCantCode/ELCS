#ifndef CIRCUITWORKSPACE_H
#define CIRCUITWORKSPACE_H

#include <QObject>
#include <QGraphicsView>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QGraphicsPixmapItem>
#include <QResizeEvent>

#include "component.h"

#include "wiregraphicsitem.h"

class CircuitWorkspace : public QGraphicsView
{
    Q_OBJECT
public:
    CircuitWorkspace(QFrame*&);

    void reset();
    
    enum class EditingStates {
        EDIT,
        POKE,
    };

    void setState(EditingStates _state) {state = _state;}
    inline EditingStates getState() const {return state;}
public slots:
    void onComponentSelected(const Component& component);
    void onComponentEditRequested(const Component& component);
protected:
    void resizeEvent ( QResizeEvent * event ) override;
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
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

    bool p_isMoving = false;
    bool p_isWiring = false;
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
};

#endif // CIRCUITWORKSPACE_H
