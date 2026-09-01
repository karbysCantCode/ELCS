#include "settingspanel.h"
#include "hotkeymanager.h"
#include "simulationmonitor.h"
#include "styles.h"

#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QPainter>
#include <QMouseEvent>
#include <QEvent>

SettingsPanel::SettingsPanel(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_TranslucentBackground);
    setGeometry(parent->rect());
    parent->installEventFilter(this);

    auto* outerLayout = new QVBoxLayout(this);
    outerLayout->setAlignment(Qt::AlignCenter);
    outerLayout->setContentsMargins(0, 0, 0, 0);

    card = new QFrame();
    card->setFixedWidth(440);
    card->setStyleSheet(STYLESHEET_FRAME_CARD);

    auto* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(20, 16, 20, 20);
    cardLayout->setSpacing(12);

    auto* headerRow = new QHBoxLayout();

    auto* title = new QLabel("Settings");
    title->setStyleSheet(STYLESHEET_LABEL_TITLE);
    headerRow->addWidget(title);
    headerRow->addStretch();

    auto* closeButton = new QPushButton(QString::fromUtf8("\xE2\x9C\x95"));
    closeButton->setFixedSize(24, 24);
    closeButton->setCursor(Qt::PointingHandCursor);
    closeButton->setStyleSheet(
        "QPushButton { background: #262b36; color: #d5d9e0; border: none; border-radius: 12px; font-weight: bold; }"
        "QPushButton:hover { background: #323949; }"
    );
    connect(closeButton, &QPushButton::clicked, this, &QWidget::deleteLater);
    headerRow->addWidget(closeButton);

    cardLayout->addLayout(headerRow);

    auto* jumpRow = new QHBoxLayout();
    jumpRow->setSpacing(4);
    cardLayout->addLayout(jumpRow);

    scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFixedHeight(360);
    scrollArea->setStyleSheet(STYLESHEET_PROPERTY_SECTION_SCROLLAREA);

    auto* scrollContent = new QWidget();
    scrollContent->setStyleSheet(STYLESHEET_PROPERTY_CONTENT);

    contentLayout = new QVBoxLayout(scrollContent);
    contentLayout->setContentsMargins(4, 4, 4, 4);
    contentLayout->setSpacing(8);

    const QString checkboxStyle =
        "QCheckBox { color: #d5d9e0; padding: 2px 0; }"
        "QCheckBox::indicator { width: 16px; height: 16px; border: 1px solid #3c4351; border-radius: 3px; background: #171a22; }"
        "QCheckBox::indicator:checked { background: #2878d4; border: 1px solid #2878d4; }";

    generalSectionMarker = addSection("General");

    auto* showOverlayCheck = new QCheckBox("Show hotkey overlay");
    showOverlayCheck->setStyleSheet(checkboxStyle);
    showOverlayCheck->setChecked(globalHotkeyManager->overlayGloballyEnabled());
    connect(showOverlayCheck, &QCheckBox::toggled, this, &SettingsPanel::onShowOverlayToggled);
    contentLayout->addWidget(showOverlayCheck);

    simulationSectionMarker = addSection("Simulation");

    auto* alertOnFloatingPinsCheck = new QCheckBox("Alert on unconnected (floating) pins");
    alertOnFloatingPinsCheck->setStyleSheet(checkboxStyle);
    alertOnFloatingPinsCheck->setChecked(globalSimulationMonitor->alertOnFloatingPins());
    connect(alertOnFloatingPinsCheck, &QCheckBox::toggled, this, &SettingsPanel::onAlertOnFloatingPinsToggled);
    contentLayout->addWidget(alertOnFloatingPinsCheck);

    auto* pauseOnConflictCheck = new QCheckBox("Pause simulator on error/conflict");
    pauseOnConflictCheck->setStyleSheet(checkboxStyle);
    pauseOnConflictCheck->setChecked(globalSimulationMonitor->pauseOnConflict());
    connect(pauseOnConflictCheck, &QCheckBox::toggled, this, &SettingsPanel::onPauseOnConflictToggled);
    contentLayout->addWidget(pauseOnConflictCheck);

    hotkeysSectionMarker = addSection("Hotkeys");

    hotkeysSectionLayout = new QVBoxLayout();
    hotkeysSectionLayout->setSpacing(4);
    contentLayout->addLayout(hotkeysSectionLayout);

    contentLayout->addStretch();

    addJumpButton(jumpRow, "General", generalSectionMarker);
    addJumpButton(jumpRow, "Simulation", simulationSectionMarker);
    addJumpButton(jumpRow, "Hotkeys", hotkeysSectionMarker);
    jumpRow->addStretch();

    scrollArea->setWidget(scrollContent);
    cardLayout->addWidget(scrollArea);

    outerLayout->addWidget(card);

    connect(globalHotkeyManager, &HotkeyManager::contextsChanged, this, &SettingsPanel::rebuildContextChecks);
    rebuildContextChecks();

    show();
    raise();
}

QWidget* SettingsPanel::addSection(const QString& title)
{
    auto* label = new QLabel(title);
    label->setStyleSheet(
        "QLabel { color: #8b93a3; font-weight: 600; font-size: 11px; padding-top: 6px; }"
    );
    contentLayout->addWidget(label);

    return label;
}

void SettingsPanel::addJumpButton(QHBoxLayout* row, const QString& label, QWidget* sectionMarker)
{
    auto* button = new QPushButton(label);
    button->setFlat(true);
    button->setCursor(Qt::PointingHandCursor);
    button->setStyleSheet(
        "QPushButton { color: #5da0ee; border: none; background: transparent; padding: 2px 6px; font-size: 11px; }"
        "QPushButton:hover { color: #79b4f5; text-decoration: underline; }"
    );

    connect(button, &QPushButton::clicked, this, [this, sectionMarker]() {
        jumpToSection(sectionMarker);
    });

    row->addWidget(button);
}

void SettingsPanel::jumpToSection(QWidget* sectionMarker)
{
    if (sectionMarker)
        scrollArea->ensureWidgetVisible(sectionMarker, 0, 0);
}

void SettingsPanel::onShowOverlayToggled(bool checked)
{
    globalHotkeyManager->setOverlayGloballyEnabled(checked);
}

void SettingsPanel::onAlertOnFloatingPinsToggled(bool checked)
{
    globalSimulationMonitor->setAlertOnFloatingPins(checked);
}

void SettingsPanel::onPauseOnConflictToggled(bool checked)
{
    globalSimulationMonitor->setPauseOnConflict(checked);
}

void SettingsPanel::rebuildContextChecks()
{
    for (auto& [check, context] : contextChecks)
        check->deleteLater();

    contextChecks.clear();

    const QString checkboxStyle =
        "QCheckBox { color: #d5d9e0; padding: 2px 0; }"
        "QCheckBox::indicator { width: 16px; height: 16px; border: 1px solid #3c4351; border-radius: 3px; background: #171a22; }"
        "QCheckBox::indicator:checked { background: #2878d4; border: 1px solid #2878d4; }";

    for (const std::string& context : globalHotkeyManager->contexts())
    {
        if (context == "global")
            continue;

        auto* check = new QCheckBox(
            QString("Show hotkeys for \"%1\"").arg(QString::fromStdString(context))
        );
        check->setStyleSheet(checkboxStyle);
        check->setChecked(globalHotkeyManager->contextEnabled(context));

        connect(check, &QCheckBox::toggled, this, [context](bool checked) {
            globalHotkeyManager->setContextEnabled(context, checked);
        });

        hotkeysSectionLayout->addWidget(check);
        contextChecks[check] = context;
    }
}

void SettingsPanel::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.fillRect(rect(), QColor(0, 0, 0, 140));
}

void SettingsPanel::mousePressEvent(QMouseEvent* event)
{
    deleteLater();
    QWidget::mousePressEvent(event);
}

bool SettingsPanel::eventFilter(QObject* obj, QEvent* event)
{
    if (obj == parentWidget() && event->type() == QEvent::Resize)
        setGeometry(parentWidget()->rect());

    return QWidget::eventFilter(obj, event);
}