#include "overlaydialogue.h"
#include "notifications.h"

OverlayDialogue::OverlayDialogue(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setGeometry(parentWidget()->rect());
    auto *layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);
    auto *frame = new QFrame;
    frame->setStyleSheet(
      "background:white;"
      "border-radius:8px;"
      "padding:20px;"
    );
    
    auto *fl = new QVBoxLayout(frame);
    fl->addWidget(new QLabel("Hello!"));
    
    auto *button = new QPushButton("OK");
    fl->addWidget(button);
    
    layout->addWidget(frame);
    
    globalNotificationManager->raise();
    connect(button, &QPushButton::clicked, this, &QWidget::deleteLater);
    parent->installEventFilter(this);
  }

void OverlayDialogue::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.fillRect(rect(), QColor(0, 0, 0, 120));
}

bool OverlayDialogue::eventFilter(QObject *obj, QEvent *event)
    {
        if (obj == parentWidget() && event->type() == QEvent::Resize)
        {
            setGeometry(parentWidget()->rect());
        }

        return QWidget::eventFilter(obj, event);
    }