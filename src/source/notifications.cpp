#include "notifications.h"
#include "notificationelement.h"

#include <QMouseEvent>

NotificationManager* globalNotificationManager = nullptr;

NotificationManager::NotificationManager(QWidget *parent)
  : QWidget(parent)
{
  setAttribute(Qt::WA_NoSystemBackground);
  setAttribute(Qt::WA_TranslucentBackground);
  setAttribute(Qt::WA_TransparentForMouseEvents, true);
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  setGeometry(parentWidget()->rect());

  layout = new QVBoxLayout(this);
  layout->addStretch(1);
  layout->setAlignment(Qt::AlignRight | Qt::AlignBottom);
  parent->installEventFilter(this);
}

void NotificationManager::notify(const std::string& title, const std::string& message, size_t durationMilliseconds) {
  auto *notification = new NotificationElement(title, message, durationMilliseconds, this);
  layout->addWidget(notification);
}

bool NotificationManager::eventFilter(QObject *obj, QEvent *event)
{
  if (obj == parentWidget() && event->type() == QEvent::Resize)
  {
      setGeometry(parentWidget()->rect());
  }

  return QWidget::eventFilter(obj, event);
}