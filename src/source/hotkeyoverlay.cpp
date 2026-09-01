#include "hotkeyoverlay.h"
#include "hotkeymanager.h"
#include "styles.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QShowEvent>

HotkeyOverlay::HotkeyOverlay(const std::string& context, QWidget* parent)
    : QWidget(parent),
      p_context(context)
{
    toggleButton = new QPushButton("?", this);
    toggleButton->setFixedSize(28, 28);
    toggleButton->setCursor(Qt::PointingHandCursor);
    toggleButton->setStyleSheet(
        "QPushButton { background: #1a1d25; color: #e5e7eb; border: 1px solid #303541; border-radius: 14px; font-weight: 600; }"
        "QPushButton:hover { background: #222631; border: 1px solid #3c4351; }"
    );

    connect(toggleButton, &QPushButton::clicked, this, &HotkeyOverlay::toggleExpanded);

    panel = new QWidget(this);
    panel->setStyleSheet(STYLESHEET_FRAME_CARD);
    panel->setMinimumWidth(240);
    panel->setVisible(false);

    panelLayout = new QVBoxLayout(panel);
    panelLayout->setContentsMargins(10, 8, 10, 8);
    panelLayout->setSpacing(4);

    connect(globalHotkeyManager, &HotkeyManager::hotkeysChanged, this, &HotkeyOverlay::refresh);
    connect(globalHotkeyManager, &HotkeyManager::activeContextChanged, this, &HotkeyOverlay::refresh);
    connect(globalHotkeyManager, &HotkeyManager::settingsChanged, this, &HotkeyOverlay::refresh);

    refresh();
}

void HotkeyOverlay::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);

    globalHotkeyManager->setActiveContext(p_context);

    refresh();
}

void HotkeyOverlay::toggleExpanded()
{
    expanded = !expanded;
    panel->setVisible(expanded);

    reposition();
}

void HotkeyOverlay::refresh()
{
    const bool visibleAtAll =
        globalHotkeyManager->overlayGloballyEnabled() &&
        globalHotkeyManager->contextEnabled(p_context);

    toggleButton->setVisible(visibleAtAll);

    if (!visibleAtAll)
    {
        expanded = false;
        panel->setVisible(false);
    }

    rebuildPanelContents();
    reposition();
}

void HotkeyOverlay::rebuildPanelContents()
{
    QLayoutItem* item;
    while ((item = panelLayout->takeAt(0)) != nullptr)
    {
        if (item->widget())
            item->widget()->deleteLater();

        delete item;
    }

    auto* title = new QLabel("Hotkeys");
    title->setStyleSheet(STYLESHEET_LABEL_TITLE);
    panelLayout->addWidget(title);

    for (const HotkeyEntry& entry : globalHotkeyManager->hotkeysForContext(p_context))
    {
        auto* row = new QLabel(
            QString("%1  \u2014  %2")
                .arg(entry.sequence.toString(QKeySequence::NativeText))
                .arg(QString::fromStdString(entry.description))
        );

        row->setStyleSheet(STYLESHEET_LABEL_TEXT);
        panelLayout->addWidget(row);
    }
}

void HotkeyOverlay::reposition()
{
    if (!parentWidget())
        return;

    const int margin = 12;

    toggleButton->move(
        parentWidget()->width() - toggleButton->width() - margin,
        parentWidget()->height() - toggleButton->height() - margin
    );
    toggleButton->raise();

    panel->adjustSize();
    panel->move(
        parentWidget()->width() - panel->width() - margin,
        parentWidget()->height() - toggleButton->height() - panel->height() - margin - 8
    );
    panel->raise();
}