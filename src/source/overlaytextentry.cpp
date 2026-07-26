#include "overlaytextentry.h"
#include "notifications.h"

#include "styles.h"

void OverlayTextEntry::innerCheck()
{
    if (!checkFunction || checkFunction(textEdit->toPlainText())) {
        deleteLater();
    }
}

OverlayTextEntry::OverlayTextEntry(QWidget *parent,
                                   std::function<bool(const QString&)> _checkFunction)
    : QWidget(parent),
      checkFunction(_checkFunction)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setGeometry(parent->rect());

    auto *layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);
    layout->setContentsMargins(0, 0, 0, 0);


    auto *frame = new QFrame();
    frame->setFixedSize(350, 180);

    frame->setStyleSheet(
        "QFrame {"
        "background: #2b2b2b;"
        "border-radius: 6px;"
        "}"
    );


    auto *fl = new QVBoxLayout(frame);
    fl->setContentsMargins(25, 25, 25, 20);
    fl->setSpacing(15);
    
    auto* title = new QLabel("New Circuit Name");
    title->setStyleSheet(STYLESHEET_LABEL_TITLE);
    fl->addWidget(title);

    textEdit = new QTextEdit();
    textEdit->setPlaceholderText("Enter name...");
    textEdit->setFixedHeight(50);

    textEdit->setStyleSheet(STYLESHEET_TEXTEDIT);

    fl->addWidget(textEdit);

    auto *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);


    auto *cancelButton = new QPushButton("Cancel");
    auto *createButton = new QPushButton("Create");

    cancelButton->setStyleSheet(STYLESHEET_BUTTON_NORMAL);


    createButton->setStyleSheet(STYLESHEET_BUTTON_MAIN);

    buttonLayout->addStretch();
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(createButton);

    fl->addLayout(buttonLayout);

    layout->addWidget(frame);

    globalNotificationManager->show();
    globalNotificationManager->raise();

    connect(createButton, &QPushButton::clicked,
            this, [this]() { innerCheck(); });

    connect(cancelButton, &QPushButton::clicked,
            this, &QWidget::deleteLater);

    parent->installEventFilter(this);
}


void OverlayTextEntry::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.fillRect(rect(), QColor(0, 0, 0, 120));
}


bool OverlayTextEntry::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == parentWidget() && event->type() == QEvent::Resize)
    {
        setGeometry(parentWidget()->rect());
    }

    return QWidget::eventFilter(obj, event);
}