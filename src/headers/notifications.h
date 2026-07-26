#ifndef NOTIFICATIONS_H
#define NOTIFICATIONS_H

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

class NotificationManager : public QWidget
{
    Q_OBJECT

public:
    explicit NotificationManager(QWidget *parent = nullptr);

    void notify(const std::string& title, const std::string& message, size_t durationMilliseconds = 3000);
private:
  QVBoxLayout* layout = nullptr;
  
protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
};

extern NotificationManager* globalNotificationManager;

#endif //NOTIFICATIONS_H