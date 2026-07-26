#ifndef OVERLAYTEXTENTRY_H
#define OVERLAYTEXTENTRY_H

#include <QObject>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <QPainter>
#include <QEvent>

class OverlayTextEntry : public QWidget
{
    Q_OBJECT

public:
    explicit OverlayTextEntry(QWidget *parent, std::function<bool(const QString&)> _checkFunction);

    QTextEdit* textEdit = nullptr;
    
    void innerCheck();
private:
    std::function<bool(const QString&)> checkFunction;
protected:
    void paintEvent(QPaintEvent *) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
};

#endif //OVERLAYTEXTENTRY_H