#ifndef DOTGRIDSCENE_H
#define DOTGRIDSCENE_H

#include <QObject>
#include <QGraphicsScene>

class DotGridScene : public QGraphicsScene
{
    Q_OBJECT
public:
    DotGridScene(QObject *parent = nullptr) : QGraphicsScene(parent) {
        //createGridPixmap();
    }
};

#endif // DOTGRIDSCENE_H
