#ifndef NOTIFICATIONELEMENT_H
#define NOTIFICATIONELEMENT_H

#include <QWidget>
#include <string>

namespace Ui {
class NotificationElement;
}

class NotificationElement : public QWidget
{
    Q_OBJECT

public:
    void fadeOut();
    explicit NotificationElement(const std::string& title, const std::string& message, size_t durationMilliseconds = 1000, QWidget *parent = nullptr);
    ~NotificationElement();

    size_t millisecondDuration = 0;

private:
    Ui::NotificationElement *ui;
};

#endif 
