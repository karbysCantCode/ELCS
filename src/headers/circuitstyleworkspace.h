#ifndef CIRCUITSTYLEWORKSPACE_H
#define CIRCUITSTYLEWORKSPACE_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPainter>
#include <QKeyEvent>

#include <memory>

#include "component.h"


class ComponentStyleGraphicsObject;
class QToolButton;
class HotkeyOverlay;


class CircuitStyleWorkspace : public QGraphicsView
{
    Q_OBJECT

public:

    explicit CircuitStyleWorkspace(QWidget* parent = nullptr);
    ~CircuitStyleWorkspace() override = default;
    

    enum class InteractionState
    {
        NONE,

        MOVING_WORKSPACE,

        SELECTED,
        MOVING_ELEMENT,

        ADDING_LINE,
        ADDING_CURVE,
        ADDING_LABEL,

        EDITING_LABEL
    };


    enum class StyleElementType
    {
        NONE,

        ANCHOR,

        LINE,
        LINE_BEGIN,
        LINE_END,

        CURVE,
        CURVE_BEGIN,
        CURVE_CONTROL1,
        CURVE_CONTROL2,
        CURVE_END,

        LABEL,

        PIN
    };


    void setComponent(SentinelComponent& component);

    SentinelComponent* getComponent() const
    {
        return p_component;
    }

    void clearComponent();


    void setInteractionState(InteractionState state);

    InteractionState getInteractionState() const
    {
        return interactionState;
    }


    void setToolAddLine();
    void setToolAddCurve();
    void setToolAddLabel();

    void cancelCurrentOperation();

    void setLineButton(QToolButton* button) {addLineButton = button;}
    void setCurveButton(QToolButton* button) {addCurveButton = button;}
    void setLabelButton(QToolButton* button) {addLabelButton = button;}

signals:

    void componentAppearanceChanged();


protected:

    void resizeEvent(QResizeEvent* event) override;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;

    void wheelEvent(QWheelEvent* event) override;

    void keyPressEvent(QKeyEvent* event) override;

    void drawBackground(
        QPainter* painter,
        const QRectF& rect
    ) override;

private:

    QGraphicsScene workspaceScene;

    SentinelComponent* p_component = nullptr;

    ComponentStyleGraphicsObject* p_graphicsObject = nullptr;

    QToolButton* addLineButton = nullptr;
    QToolButton* addCurveButton = nullptr;
    QToolButton* addLabelButton = nullptr;

    HotkeyOverlay* p_hotkeyOverlay = nullptr;

    InteractionState interactionState =
        InteractionState::NONE;


    StyleElementType selectedType =
        StyleElementType::NONE;

    int selectedIndex = -1;


    /*
        For dragging an existing object.
    */

    QPoint p_dragStartMouse;

    Position p_dragOriginalA;
    Position p_dragOriginalB;

    int p_dragOriginalX = 0;
    int p_dragOriginalY = 0;


    /*
        For adding a line.
    */

    Position p_newLineStart;


    /*
        For adding a curve.

        Four mouse clicks:

        1. begin
        2. control1
        3. control2
        4. end
    */

    int p_curveCreationStage = 0;

    ComponentCurve p_newCurve;


    /*
        Right-click workspace movement.
    */

    QPoint p_movementBegunQPoint;

    int p_preMoveXPosition = 0;
    int p_preMoveYPosition = 0;

    int p_xposition = 0;
    int p_yposition = 0;

    int p_width = 0;
    int p_height = 0;

    int p_maxWidth = 10000;
    int p_maxHeight = 10000;


private:

    Position convertEventPosToPosition(
        const QPoint& point
    ) const;

    QPointF positionToScene(
        const Position& position
    ) const;


    void updateWorkspacePosition();

    void moveWorkspaceToCurrentMouse(
        const QPoint& event
    );


    void selectElement(
        StyleElementType type,
        int index
    );

    void deselectElement();


    void beginMovingSelected(
        const QPoint& mousePosition
    );

    void moveSelected(
        const QPoint& mousePosition
    );

    void finishMovingSelected();


    void addLineAt(
        const Position& position
    );

    void addCurveAt(
        const Position& position
    );

    void addLabelAt(
        const Position& position
    );


    void editSelectedLabel();


    void deleteSelectedElement();


    void refreshGraphics();

    void emitAppearanceChanged();
};

#endif