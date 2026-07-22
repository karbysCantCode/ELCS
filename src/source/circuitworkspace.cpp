#include "circuitworkspace.h"
#include <iostream>

CircuitWorkspace::CircuitWorkspace(QFrame*& frame) {
    std::cout << "INIT X:" << size().width() << std::endl;
    std::cout << "INIT Y:" << size().height() << std::endl;

    updateViewSize();

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setScene(&workspaceScene);

    resetBackgroundGrid();

}

void CircuitWorkspace::resizeEvent ( QResizeEvent * event )  {
    updateViewSize();
    resetBackgroundGrid();
}

void CircuitWorkspace::updateViewSize() {
    p_width = size().width();
    p_height = size().height();

    p_maxWidth = p_width * 1.5; // needs handling to like actually grab circ size from a save file
    p_maxHeight = p_height * 1.5;
}


void CircuitWorkspace::resetBackgroundGrid() {
    if (backgroundGridItem != nullptr) {
        backgroundGridItem = nullptr;
        workspaceScene.removeItem(backgroundGridItem);
        auto tempPtr = backgroundGridItem; // avoid race condition memory leak.. idk if it event exists but..
        delete tempPtr;

    }

    QPixmap pmap(p_maxWidth, p_maxHeight);
    pmap.fill(Qt::transparent);

    QPixmap tile(10, 10);
    tile.fill(Qt::white);
    QPainter tp(&tile);
    tp.setPen(Qt::lightGray);
    tp.drawPoint(5, 5);
    tp.end();

    QPainter painter(&pmap);
    QBrush tiledBrush(tile);

    painter.fillRect(pmap.rect(), tiledBrush);

    backgroundGridItem = workspaceScene.addPixmap(pmap);


}

void CircuitWorkspace::wheelEvent(QWheelEvent *event)  {

}
void CircuitWorkspace::mousePressEvent(QMouseEvent *event)  {

    /*
    mmb move
    left select
    right
    */

    p_isMoving = true;

    p_movementBegunQPoint = std::make_unique<QPoint>();
    p_movementBegunQPoint->setX(event->pos().x());
    p_movementBegunQPoint->setY(event->pos().y());

    p_preMoveXPosition = p_xposition;
    p_preMoveYPosition = p_yposition;

    std::cout << "mouse down" << std::endl;
}
void CircuitWorkspace::mouseMoveEvent(QMouseEvent *event)  {
    if (p_isMoving) {
        moveWorkspaceToCurrentMouse(event->pos());
        update(); // signal redraw
        std::cout << "X:" << p_xposition << " Y:" << p_yposition << " mouse moved with redraw" << std::endl;
    }

}
void CircuitWorkspace::mouseReleaseEvent(QMouseEvent *event)  {
    std::cout << "mouse up" << std::endl;
    if (p_isMoving) {
        moveWorkspaceToCurrentMouse(event->pos());
        update(); // signal redraw

        p_movementBegunQPoint.reset();
    }
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