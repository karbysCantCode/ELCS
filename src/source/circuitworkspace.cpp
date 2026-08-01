#include "circuitworkspace.h"
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

// void CircuitWorkspace::resetBackgroundGrid() {
//     QPixmap pmap(p_maxWidth, p_maxHeight);
//     pmap.fill(Qt::transparent);

//     QPixmap tile(10, 10);
//     tile.fill(Qt::white);
//     QPainter tp(&tile);
//     tp.setPen(Qt::lightGray);
//     tp.drawPoint(5, 5);
//     tp.end();

//     QPainter painter(&pmap);
//     painter.fillRect(pmap.rect(), QBrush(tile));
//     painter.end();

//     if (backgroundGridItem == nullptr) {
//         backgroundGridItem = workspaceScene.addPixmap(pmap);
//         backgroundGridItem->setZValue(-1);
//     } else {
//         backgroundGridItem->setPixmap(pmap);
//     }


// }

void CircuitWorkspace::wheelEvent(QWheelEvent *event)  {
  
}
void CircuitWorkspace::mousePressEvent(QMouseEvent *event)  {

    if (event->button() == Qt::LeftButton) {
        QGraphicsItem* item = itemAt(event->pos());

        if (item == nullptr || item == backgroundGridItem) {
            p_isMoving = true;

            p_movementBegunQPoint = std::make_unique<QPoint>();
            p_movementBegunQPoint->setX(event->pos().x());
            p_movementBegunQPoint->setY(event->pos().y());

            p_preMoveXPosition = p_xposition;
            p_preMoveYPosition = p_yposition;

            event->accept();
            return;
        }
    }

    QGraphicsView::mousePressEvent(event);
}
void CircuitWorkspace::mouseMoveEvent(QMouseEvent *event)  {
    if (p_isMoving) {
        moveWorkspaceToCurrentMouse(event->pos());
        update(); // signal redraw
        std::cout << "X:" << p_xposition << " Y:" << p_yposition << " mouse moved with redraw" << std::endl;
    }

    QGraphicsView::mouseMoveEvent(event);

}
void CircuitWorkspace::mouseReleaseEvent(QMouseEvent *event)  {
    std::cout << "mouse up" << std::endl;
    if (p_isMoving) {
      p_isMoving = false;
        moveWorkspaceToCurrentMouse(event->pos());
        update(); // signal redraw

        p_movementBegunQPoint.reset();
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