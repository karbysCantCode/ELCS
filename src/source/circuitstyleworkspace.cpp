#include "circuitstyleworkspace.h"
#include "componentstylegraphicsobject.h"
#include "hotkeymanager.h"
#include "hotkeyoverlay.h"

#include <QCursor>
#include <QInputDialog>
#include <QScrollBar>
#include <QToolButton>

#include <algorithm>
#include <cmath>

CircuitStyleWorkspace::CircuitStyleWorkspace(
    QWidget* parent
)
    : QGraphicsView(parent)
{
    setScene(&workspaceScene);

    setHorizontalScrollBarPolicy(
        Qt::ScrollBarAlwaysOff
    );

    setVerticalScrollBarPolicy(
        Qt::ScrollBarAlwaysOff
    );

    setViewportUpdateMode(
        QGraphicsView::FullViewportUpdate
    );

    workspaceScene.setSceneRect(
        -5000,
        -5000,
        10000,
        10000
    );


    setTransformationAnchor(
        QGraphicsView::NoAnchor
    );


    setResizeAnchor(
        QGraphicsView::NoAnchor
    );

    p_hotkeyOverlay = new HotkeyOverlay("style", this);

    globalHotkeyManager->registerHotkey("style", QKeySequence(Qt::Key_Delete), "Delete selected element");
    globalHotkeyManager->registerHotkey("style", QKeySequence(Qt::Key_Escape), "Cancel current operation");

    
}

void CircuitStyleWorkspace::drawBackground(
    QPainter* painter,
    const QRectF& rect
)
{
    static QPixmap tile;


    if (tile.isNull())
    {
        tile = QPixmap(
            10,
            10
        );

        tile.fill(Qt::white);


        QPainter tp(&tile);

        tp.setPen(
            Qt::lightGray
        );

        tp.drawPoint(
            5,
            5
        );
    }


    painter->fillRect(
        rect,
        QBrush(tile)
    );
}

Position CircuitStyleWorkspace::convertEventPosToPosition(
    const QPoint& point
) const
{
    QPointF scenePosition =
        mapToScene(point);


    return Position(
        static_cast<int>(
            std::floor(
                (scenePosition.x()+2.5) / 10.0
            )
        ),

        static_cast<int>(
            std::floor(
                (scenePosition.y()+2.5) / 10.0
            )
        )
    );
}

void CircuitStyleWorkspace::selectElement(
    StyleElementType type,
    int index
)
{
    selectedType = type;
    selectedIndex = index;

    setInteractionState(
        InteractionState::SELECTED
    );

    viewport()->update();
}

void CircuitStyleWorkspace::deselectElement()
{
    selectedType =
        StyleElementType::NONE;

    selectedIndex = -1;

    setInteractionState(
        InteractionState::NONE
    );

    viewport()->update();
}

void CircuitStyleWorkspace::setComponent(
    SentinelComponent& component
)
{
    cancelCurrentOperation();

    workspaceScene.clear();

    p_component = &component;

    if (!p_component)
        return;


    p_graphicsObject =
        new ComponentStyleGraphicsObject(
            *p_component
        );


    workspaceScene.addItem(
        p_graphicsObject
    );


    p_graphicsObject->setPos(
        0,
        0
    );


    

    p_graphicsObject->refresh();
}

void CircuitStyleWorkspace::clearComponent()
{
    cancelCurrentOperation();

    workspaceScene.clear();

    p_graphicsObject = nullptr;
    p_component = nullptr;
}

void CircuitStyleWorkspace::setToolAddLine()
{
    deselectElement();

    setInteractionState(
        InteractionState::ADDING_LINE
    );
}


void CircuitStyleWorkspace::setToolAddCurve()
{
    deselectElement();

    p_curveCreationStage = 0;

    setInteractionState(
        InteractionState::ADDING_CURVE
    );
}


void CircuitStyleWorkspace::setToolAddLabel()
{
    deselectElement();

    setInteractionState(
        InteractionState::ADDING_LABEL
    );
}

void CircuitStyleWorkspace::mousePressEvent(
    QMouseEvent* event
)
{

    if (event->button() == Qt::RightButton)
    {
        setInteractionState(
            InteractionState::MOVING_WORKSPACE
        );


        p_movementBegunQPoint =
            event->pos();


        p_preMoveXPosition =
            horizontalScrollBar()->value();

        p_preMoveYPosition =
            verticalScrollBar()->value();


        event->accept();

        return;
    }


    if (event->button() != Qt::LeftButton)
    {
        QGraphicsView::mousePressEvent(event);

        return;
    }

    if (itemAt(event->pos()) == nullptr) {
        setInteractionState(InteractionState::NONE);
    }


    if (!p_component ||
        !p_graphicsObject)
    {
        return;
    }


    const Position position =
        convertEventPosToPosition(
            event->pos()
        );

    if (interactionState ==
        InteractionState::ADDING_LINE)
    {
        p_newLineStart = position;

        return;
    }

    if (interactionState ==
        InteractionState::ADDING_CURVE)
    {
        addCurveAt(position);

        return;
    }

    if (interactionState ==
        InteractionState::ADDING_LABEL)
    {
        addLabelAt(position);

        return;
    }


    QGraphicsItem* hitItem =
        itemAt(event->pos());

    auto* handle =
        dynamic_cast<ComponentStyleHandleItem*>(
            hitItem
        );


    if (!handle)
    {
        qDebug("BAD BAD GONE>");
        deselectElement();

        return;
    }

    qDebug("BAD BAD GOOD GOOD>");
    switch (handle->getHitType())
    {
        
        
        
        
        
        

        


        case StyleHitType::LINE_BEGIN:
            qDebug("BAD LIINE GONE>");
            selectElement(
                StyleElementType::LINE_BEGIN,
                handle->getIndex()
            );

            break;


        case StyleHitType::LINE_MIDDLE:

            selectElement(
                StyleElementType::LINE,
                handle->getIndex()
            );

            break;


        case StyleHitType::LINE_END:

            selectElement(
                StyleElementType::LINE_END,
                handle->getIndex()
            );

            break;


        case StyleHitType::CURVE_BEGIN:

            selectElement(
                StyleElementType::CURVE_BEGIN,
                handle->getIndex()
            );

            break;


        case StyleHitType::CURVE_CONTROL1:

            selectElement(
                StyleElementType::CURVE_CONTROL1,
                handle->getIndex()
            );

            break;


        case StyleHitType::CURVE_CONTROL2:

            selectElement(
                StyleElementType::CURVE_CONTROL2,
                handle->getIndex()
            );

            break;


        case StyleHitType::CURVE_END:

            selectElement(
                StyleElementType::CURVE_END,
                handle->getIndex()
            );

            break;


        case StyleHitType::CURVE_MIDDLE:

            selectElement(
                StyleElementType::CURVE,
                handle->getIndex()
            );

            break;


        case StyleHitType::LABEL:

            selectElement(
                StyleElementType::LABEL,
                handle->getIndex()
            );

            break;


        case StyleHitType::PIN:
            qDebug("BAD pin pin pin GONE>");
            selectElement(
                StyleElementType::PIN,
                handle->getIndex()
            );

            break;


        default:
            break;
    }

    
    
    
    
    


    beginMovingSelected(
        event->pos()
    );


    event->accept();
}

void CircuitStyleWorkspace::beginMovingSelected(
    const QPoint& mousePosition
)
{
    if (!p_component)
        return;


    p_dragStartMouse =
        mousePosition;


    const auto& appearance =
        p_component->getAppearance();


    switch (selectedType)
    {
        case StyleElementType::LINE_BEGIN:

            p_dragOriginalA =
                appearance.lines[selectedIndex].begin;

            break;


        case StyleElementType::LINE_END:

            p_dragOriginalA =
                appearance.lines[selectedIndex].end;

            break;


        case StyleElementType::LINE:

            p_dragOriginalA =
                appearance.lines[selectedIndex].begin;

            p_dragOriginalB =
                appearance.lines[selectedIndex].end;

            break;


        

        
        

        


        case StyleElementType::LABEL:

            p_dragOriginalA =
                appearance.labels[selectedIndex].position;

            break;


        case StyleElementType::CURVE_BEGIN:

            p_dragOriginalA =
                appearance.curves[selectedIndex].begin;

            break;


        case StyleElementType::CURVE_CONTROL1:

            p_dragOriginalA =
                appearance.curves[selectedIndex].control1;

            break;


        case StyleElementType::CURVE_CONTROL2:

            p_dragOriginalA =
                appearance.curves[selectedIndex].control2;

            break;


        case StyleElementType::CURVE_END:

            p_dragOriginalA =
                appearance.curves[selectedIndex].end;

            break;


        case StyleElementType::CURVE:

            p_dragOriginalA =
                appearance.curves[selectedIndex].begin;

            p_dragOriginalB =
                appearance.curves[selectedIndex].end;

            break;
        case StyleElementType::PIN:{

            const auto pins = p_component->getPins();

            if (selectedIndex < 0 ||
                selectedIndex >= static_cast<int>(pins.size()))
            {
                return;
            }

            p_dragOriginalA =
                pins[selectedIndex]->getAppearancePosition();


            break;
        }


        default:
            return;
    }


    setInteractionState(
        InteractionState::MOVING_ELEMENT
    );
}

void CircuitStyleWorkspace::moveSelected(
    const QPoint& mousePosition
)
{
    if (!p_component)
        return;


    QPointF startScene =
        mapToScene(p_dragStartMouse);

    QPointF currentScene =
        mapToScene(mousePosition);


    QPointF delta =
        currentScene - startScene;


    Position gridDelta(
        static_cast<int>(
            std::round(delta.x() / 10.0)
        ),

        static_cast<int>(
            std::round(delta.y() / 10.0)
        )
    );


    auto& appearance =
        p_component->getAppearance();


    switch (selectedType)
    {
        /*
            LINE ENDPOINT
        */

        case StyleElementType::LINE_BEGIN:
            qDebug("MOOOOO ve L BG>");
            appearance.lines[selectedIndex].begin =
                p_dragOriginalA + gridDelta;

            break;


        case StyleElementType::LINE_END:
        qDebug("MOOOOO ve LINEE DN>");
            appearance.lines[selectedIndex].end =
                p_dragOriginalA + gridDelta;

            break;


        /*
            ENTIRE LINE
        */

        case StyleElementType::LINE:
        qDebug("MOOOOO ve LIIN D>");
            appearance.lines[selectedIndex].begin =
                p_dragOriginalA + gridDelta;

            appearance.lines[selectedIndex].end =
                p_dragOriginalB + gridDelta;

            break;


        /*
            ANCHOR
        */

        case StyleElementType::ANCHOR:
            qDebug("MOOOOO ve ACHOR>");
            appearance.anchor =
                p_dragOriginalA + gridDelta;

                qDebug()
    << "COMPONENT ANCHOR:"
    << p_component->getAppearance().anchor.x
    << p_component->getAppearance().anchor.y;

            break;


        /*
            LABEL
        */

        case StyleElementType::LABEL:
        qDebug("MOOOOO ve LIBERAL>");
            appearance.labels[selectedIndex].position =
                p_dragOriginalA + gridDelta;

            break;


        /*
            CURVE POINTS
        */

        case StyleElementType::CURVE_BEGIN:
qDebug("MOOOOO ve CURVACIOUS>");
            appearance.curves[selectedIndex].begin =
                p_dragOriginalA + gridDelta;

            break;


        case StyleElementType::CURVE_CONTROL1:
qDebug("MOOOOO ve CURVE CONTROL 1>");
            appearance.curves[selectedIndex].control1 =
                p_dragOriginalA + gridDelta;

            break;


        case StyleElementType::CURVE_CONTROL2:
qDebug("MOOOOO ve CURVE CONTROL 2>");
            appearance.curves[selectedIndex].control2 =
                p_dragOriginalA + gridDelta;

            break;


        case StyleElementType::CURVE_END:
qDebug("MOOOOO ve CURVE END>");
            appearance.curves[selectedIndex].end =
                p_dragOriginalA + gridDelta;

            break;


        /*
            WHOLE CURVE
        */

        case StyleElementType::CURVE:
qDebug("MOOOOO ve WHOOLE CURVE>");
            appearance.curves[selectedIndex].begin =
                p_dragOriginalA + gridDelta;

            appearance.curves[selectedIndex].end =
                p_dragOriginalB + gridDelta;

            break;

        case StyleElementType::PIN:
        {
            qDebug("MOOOOO ve PIN>");

            const auto pins =
                p_component->getPins();

            if (selectedIndex < 0 ||
                selectedIndex >= static_cast<int>(pins.size()))
            {
                return;
            }

            pins[selectedIndex]->setAppearancePosition(
                p_dragOriginalA + gridDelta
            );

            qDebug()
                << "PIN POSITION:"
                << pins[selectedIndex]->getAppearancePosition().x
                << pins[selectedIndex]->getAppearancePosition().y;

            break;
        }


        default:
            return;
    }


    p_graphicsObject->refresh();

    emitAppearanceChanged();
}

void CircuitStyleWorkspace::mouseMoveEvent(
    QMouseEvent* event
)
{
    switch (interactionState)
    {
        case InteractionState::MOVING_WORKSPACE:

            moveWorkspaceToCurrentMouse(
                event->pos()
            );

            event->accept();

            return;


        case InteractionState::MOVING_ELEMENT:
            qDebug("MOOOOO ve element>");
            moveSelected(   
                event->pos()
            );

            event->accept();

            return;


        default:
            break;
    }


    QGraphicsView::mouseMoveEvent(event);
}

void CircuitStyleWorkspace::moveWorkspaceToCurrentMouse(
    const QPoint& event
)
{
    const int dx =
        p_movementBegunQPoint.x() - event.x();

    const int dy =
        p_movementBegunQPoint.y() - event.y();


    horizontalScrollBar()->setValue(
        p_preMoveXPosition + dx
    );

    verticalScrollBar()->setValue(
        p_preMoveYPosition + dy
    );
}

void CircuitStyleWorkspace::mouseReleaseEvent(
    QMouseEvent* event
)
{
    if (event->button() == Qt::RightButton)
    {
        if (interactionState ==
            InteractionState::MOVING_WORKSPACE)
        {
            setInteractionState(
                InteractionState::NONE
            );
        }

        event->accept();

        return;
    }


    if (event->button() != Qt::LeftButton)
        return;


    /*
        Finish line creation.
    */

    if (interactionState ==
        InteractionState::ADDING_LINE)
    {
        Position end =
            convertEventPosToPosition(
                event->pos()
            );
        
        qDebug("Efjef");

        if (end != p_newLineStart)
        {
            qDebug("Efjef2");
            p_component->getAppearance().addLine(
                p_newLineStart,
                end
            );
        }


        setInteractionState(
            InteractionState::ADDING_LINE
        );

        qDebug("heh");
        p_graphicsObject->refresh();

        emitAppearanceChanged();

        event->accept();

        return;
    }


    /*
        Finish movement.
    */

    if (interactionState ==
        InteractionState::MOVING_ELEMENT)
    {
        setInteractionState(
            InteractionState::SELECTED
        );

        event->accept();

        return;
    }


    QGraphicsView::mouseReleaseEvent(event);
}

void CircuitStyleWorkspace::addCurveAt(
    const Position& position
)
{
    auto& appearance =
        p_component->getAppearance();


    switch (p_curveCreationStage)
    {
        case 0:

            p_newCurve.begin =
                position;

            p_curveCreationStage = 1;

            break;


        case 1:

            p_newCurve.control1 =
                position;

            p_curveCreationStage = 2;

            break;


        case 2:

            p_newCurve.control2 =
                position;

            p_curveCreationStage = 3;

            break;


        case 3:

            p_newCurve.end =
                position;


            appearance.curves.push_back(
                p_newCurve
            );


            p_newCurve =
                ComponentCurve();


            p_curveCreationStage = 0;


            emitAppearanceChanged();

            break;
    }


    p_graphicsObject->refresh();
}

void CircuitStyleWorkspace::addLabelAt(
    const Position& position
)
{
    bool accepted = false;


    QString text =
        QInputDialog::getText(
            this,
            "Add Label",
            "Label:",
            QLineEdit::Normal,
            "",
            &accepted
        );


    if (!accepted ||
        text.isEmpty())
    {
        setInteractionState(
            InteractionState::NONE
        );

        return;
    }


    p_component->getAppearance().addLabel(
        text,
        position
    );


    p_graphicsObject->refresh();

    emitAppearanceChanged();


    setInteractionState(
        InteractionState::NONE
    );
}


void CircuitStyleWorkspace::mouseDoubleClickEvent(
    QMouseEvent* event
)
{
    if (event->button() != Qt::LeftButton)
        return;


    if (!p_graphicsObject ||
        !p_component)
        return;


    QGraphicsItem* hitItem =
        itemAt(event->pos());

    auto* handle =
        dynamic_cast<ComponentStyleHandleItem*>(
            hitItem
        );


    if (handle &&
        handle->getHitType() == StyleHitType::LABEL)
    {
        selectedType =
            StyleElementType::LABEL;

        selectedIndex =
            handle->getIndex();


        editSelectedLabel();

        event->accept();

        return;
    }


    QGraphicsView::mouseDoubleClickEvent(event);
}

void CircuitStyleWorkspace::editSelectedLabel()
{
    if (selectedType !=
        StyleElementType::LABEL)
        return;


    if (selectedIndex < 0)
        return;


    auto& label =
        p_component->getAppearance()
            .labels[selectedIndex];


    bool accepted = false;


    QString text =
        QInputDialog::getText(
            this,
            "Edit Label",
            "Label:",
            QLineEdit::Normal,
            label.text,
            &accepted
        );


    if (!accepted)
        return;


    label.text = text;


    p_graphicsObject->refresh();

    emitAppearanceChanged();
}

void CircuitStyleWorkspace::deleteSelectedElement()
{
    if (!p_component)
        return;


    auto& appearance =
        p_component->getAppearance();


    switch (selectedType)
    {
        case StyleElementType::LINE:
        case StyleElementType::LINE_BEGIN:
        case StyleElementType::LINE_END:

            if (selectedIndex >= 0 &&
                selectedIndex <
                    static_cast<int>(
                        appearance.lines.size()
                    ))
            {
                appearance.lines.erase(
                    appearance.lines.begin()
                    + selectedIndex
                );
            }

            break;


        case StyleElementType::CURVE:
        case StyleElementType::CURVE_BEGIN:
        case StyleElementType::CURVE_CONTROL1:
        case StyleElementType::CURVE_CONTROL2:
        case StyleElementType::CURVE_END:

            if (selectedIndex >= 0 &&
                selectedIndex <
                    static_cast<int>(
                        appearance.curves.size()
                    ))
            {
                appearance.curves.erase(
                    appearance.curves.begin()
                    + selectedIndex
                );
            }

            break;


        case StyleElementType::LABEL:

            if (selectedIndex >= 0 &&
                selectedIndex <
                    static_cast<int>(
                        appearance.labels.size()
                    ))
            {
                appearance.labels.erase(
                    appearance.labels.begin()
                    + selectedIndex
                );
            }

            break;


        /*
            Anchor and pins cannot be deleted.
        */

        default:
            return;
    }


    deselectElement();

    p_graphicsObject->refresh();

    emitAppearanceChanged();
}

void CircuitStyleWorkspace::keyPressEvent(
    QKeyEvent* event
)
{
    if (event->key() == Qt::Key_Delete ||
        event->key() == Qt::Key_Backspace)
    {
        deleteSelectedElement();

        event->accept();

        return;
    }


    if (event->key() == Qt::Key_Escape)
    {
        cancelCurrentOperation();

        event->accept();

        return;
    }


    QGraphicsView::keyPressEvent(event);
}

void CircuitStyleWorkspace::cancelCurrentOperation()
{
    p_curveCreationStage = 0;

    addLineButton->setChecked(false);
    addCurveButton->setChecked(false);
    addLabelButton->setChecked(false);

    selectedType =
        StyleElementType::NONE;

    selectedIndex = -1;


    setInteractionState(
        InteractionState::NONE
    );
}

void CircuitStyleWorkspace::resizeEvent(
    QResizeEvent* event
)
{
    QGraphicsView::resizeEvent(event);

    p_width = width();
    p_height = height();

    if (p_hotkeyOverlay)
        p_hotkeyOverlay->reposition();
}

void CircuitStyleWorkspace::setInteractionState(
    InteractionState state
)
{
    interactionState = state;

    viewport()->update();
}

void CircuitStyleWorkspace::emitAppearanceChanged()
{
    if (p_graphicsObject)
        p_graphicsObject->refresh();

    const auto& appearance = p_graphicsObject->getSentinelComponent().getAppearance();

    qDebug() << "========== COMPONENT APPEARANCE ==========";

    qDebug() << "Anchor:"
             << "(" << appearance.anchor.x
             << "," << appearance.anchor.y << ")";

    qDebug() << "Lines:" << appearance.lines.size();

    for (size_t i = 0; i < appearance.lines.size(); ++i)
    {
        const auto& line = appearance.lines[i];

        qDebug() << "  Line" << i
                 << ":"
                 << "(" << line.begin.x << "," << line.begin.y << ")"
                 << "->"
                 << "(" << line.end.x << "," << line.end.y << ")"
                 << "color =" << line.color
                 << "width =" << line.width;
    }

    qDebug() << "Curves:" << appearance.curves.size();

    for (size_t i = 0; i < appearance.curves.size(); ++i)
    {
        const auto& curve = appearance.curves[i];

        qDebug() << "  Curve" << i
                 << ":"
                 << "begin ="
                 << "(" << curve.begin.x << "," << curve.begin.y << ")"
                 << "control1 ="
                 << "(" << curve.control1.x << "," << curve.control1.y << ")"
                 << "control2 ="
                 << "(" << curve.control2.x << "," << curve.control2.y << ")"
                 << "end ="
                 << "(" << curve.end.x << "," << curve.end.y << ")"
                 << "color =" << curve.color
                 << "width =" << curve.width;
    }

    qDebug() << "Labels:" << appearance.labels.size();

    for (size_t i = 0; i < appearance.labels.size(); ++i)
    {
        const auto& label = appearance.labels[i];

        qDebug() << "  Label" << i
                 << ":"
                 << "text =" << label.text
                 << "position ="
                 << "(" << label.position.x
                 << "," << label.position.y << ")"
                 << "color =" << label.color
                 << "fontSize =" << label.fontSize;
    }

    qDebug() << "===========================================";

    emit componentAppearanceChanged();
}

void CircuitStyleWorkspace::wheelEvent(QWheelEvent* event)
{
    QGraphicsView::wheelEvent(event);
}