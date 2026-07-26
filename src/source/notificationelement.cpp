#include "notificationelement.h"
#include "ui_notificationelement.h"
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QTimer>

NotificationElement::NotificationElement(const std::string& title, const std::string& message, size_t durationMilliseconds, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::NotificationElement)
    , millisecondDuration(durationMilliseconds)
{
    ui->setupUi(this);
    // setAttribute(Qt::WA_TransparentForMouseEvents, false);
    ui->title->setText(QString::fromStdString(title));
    ui->message->setText(QString::fromStdString(message));
    connect(ui->exit, &QPushButton::clicked, this, &QWidget::deleteLater);
    QTimer::singleShot(millisecondDuration, this, &NotificationElement::fadeOut);
}

NotificationElement::~NotificationElement()
{
    delete ui;
}

void NotificationElement::fadeOut()
{
    auto *effect = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(effect);

    auto *animation = new QPropertyAnimation(effect, "opacity");

    animation->setDuration(700);
    animation->setStartValue(1.0);
    animation->setEndValue(0.0);

    connect(animation, &QPropertyAnimation::finished,
            this, &QWidget::deleteLater);

    animation->start(QAbstractAnimation::DeleteWhenStopped);
}