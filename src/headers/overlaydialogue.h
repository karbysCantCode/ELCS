#ifndef OVERLAYDIALOGUE_H
#define OVERLAYDIALOGUE_H

#include <QObject>
#include <QWidget>
#include <QVBoxLayout>
#include <QFrame>
#include <QPushButton>
#include <QLabel>
#include <QPainter>
#include <QEvent>

class OverlayDialogue : public QWidget
{
    Q_OBJECT

public:
    explicit OverlayDialogue(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
};

#endif //OVERLAYDIALOGUE_H