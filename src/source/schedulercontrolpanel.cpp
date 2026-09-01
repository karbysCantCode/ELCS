#include "schedulercontrolpanel.h"

#include "scheduler.h"
#include "projectmanager.h"
#include "styles.h"

#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QTimer>

namespace {



constexpr int POLL_INTERVAL_MS = 16;
}

SchedulerControlPanel::SchedulerControlPanel(QWidget* parent)
    : QWidget(parent)
{
    auto* outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);

    auto* card = new QFrame(this);
    card->setStyleSheet(STYLESHEET_FRAME_CARD);
    outerLayout->addWidget(card);

    auto* layout = new QVBoxLayout(card);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(10);

    auto* title = new QLabel("Scheduler", card);
    title->setStyleSheet(STYLESHEET_LABEL_TITLE);
    layout->addWidget(title);

    auto* statusRow = new QHBoxLayout();
    statusRow->setSpacing(8);

    statusLabel = new QLabel("Paused", card);
    tickCountLabel = new QLabel("Tick: 0", card);
    tickCountLabel->setStyleSheet(STYLESHEET_LABEL_SECONDARY);

    statusRow->addWidget(statusLabel);
    statusRow->addStretch();
    statusRow->addWidget(tickCountLabel);

    layout->addLayout(statusRow);

    playPauseButton = new QPushButton("\u25B6 Play", card);
    playPauseButton->setCursor(Qt::PointingHandCursor);
    layout->addWidget(playPauseButton);

    auto* stepSprintRow = new QHBoxLayout();
    stepSprintRow->setSpacing(8);

    stepButton = new QPushButton("Step", card);
    stepButton->setStyleSheet(STYLESHEET_BUTTON_NORMAL);
    stepButton->setCursor(Qt::PointingHandCursor);

    auto* sprintLabel = new QLabel("Sprint:", card);
    sprintLabel->setStyleSheet(STYLESHEET_LABEL_SECONDARY);

    sprintCountSpinBox = new QSpinBox(card);
    sprintCountSpinBox->setRange(1, 1000000);
    sprintCountSpinBox->setValue(100);
    sprintCountSpinBox->setStyleSheet(STYLESHEET_SPINBOX);

    sprintButton = new QPushButton("Sprint", card);
    sprintButton->setStyleSheet(STYLESHEET_BUTTON_NORMAL);
    sprintButton->setCursor(Qt::PointingHandCursor);

    stepSprintRow->addWidget(stepButton);
    stepSprintRow->addWidget(sprintLabel);
    stepSprintRow->addWidget(sprintCountSpinBox, 1);
    stepSprintRow->addWidget(sprintButton);

    layout->addLayout(stepSprintRow);

    pollTimer = new QTimer(this);
    pollTimer->setInterval(POLL_INTERVAL_MS);
    connect(pollTimer, &QTimer::timeout, this, &SchedulerControlPanel::pollScheduler);

    connect(playPauseButton, &QPushButton::clicked, this, &SchedulerControlPanel::onPlayPauseClicked);
    connect(stepButton, &QPushButton::clicked, this, &SchedulerControlPanel::onStepClicked);
    connect(sprintButton, &QPushButton::clicked, this, &SchedulerControlPanel::onSprintClicked);

    refreshControlsEnabled();
    refreshStatusDisplay();
}

void SchedulerControlPanel::onPlayPauseClicked()
{
    
    
    qDebug("njkdsfjk"); 
    if (waitingOnFiniteRun)
        return;

    if (playMode)
    {
        globalScheduler->stopTicks();
        playMode = false;
        
        
        
        
    }
    else
    {
        globalScheduler->runTicks(0);
        playMode = true;

        if (!pollTimer->isActive())
            pollTimer->start();
    }

    refreshControlsEnabled();
    refreshStatusDisplay();
}

void SchedulerControlPanel::onStepClicked()
{
    startFiniteRun(1);
}

void SchedulerControlPanel::onSprintClicked()
{
    startFiniteRun(sprintCountSpinBox->value());
}

void SchedulerControlPanel::startFiniteRun(int tickCount)
{
    if (playMode || waitingOnFiniteRun)
        return;

    waitingOnFiniteRun = true;
    globalScheduler->runTicks(tickCount);

    if (!pollTimer->isActive())
        pollTimer->start();

    refreshControlsEnabled();
    refreshStatusDisplay();
}

void SchedulerControlPanel::pollScheduler()
{
    if (globalProjectManager && globalProjectManager->workspace)
        globalProjectManager->workspace->viewport()->update();

    refreshStatusDisplay();

    if (!globalScheduler->isRunning())
    {
        
        
        
        playMode = false;
        waitingOnFiniteRun = false;

        pollTimer->stop();

        refreshControlsEnabled();
        refreshStatusDisplay();
    }
}

void SchedulerControlPanel::refreshControlsEnabled()
{
    
    
    
    
    playPauseButton->setEnabled(!waitingOnFiniteRun);
    stepButton->setEnabled(!playMode && !waitingOnFiniteRun);
    sprintButton->setEnabled(!playMode && !waitingOnFiniteRun);
    sprintCountSpinBox->setEnabled(!playMode && !waitingOnFiniteRun);

    playPauseButton->setText(playMode ? "\u23F8 Pause" : "\u25B6 Play");
    playPauseButton->setStyleSheet(playMode ? STYLESHEET_BUTTON_DANGER : STYLESHEET_BUTTON_MAIN);
}

void SchedulerControlPanel::refreshStatusDisplay()
{
    tickCountLabel->setText(QString("Tick: %1").arg(globalScheduler->getCurrentTick()));

    if (playMode)
    {
        statusLabel->setText("\u25CF Running (full speed)");
        statusLabel->setStyleSheet(STYLESHEET_LABEL_SUCCESS);
    }
    else if (waitingOnFiniteRun)
    {
        statusLabel->setText("\u25CF Running\u2026");
        statusLabel->setStyleSheet(STYLESHEET_LABEL_SUCCESS);
    }
    else
    {
        statusLabel->setText("Paused");
        statusLabel->setStyleSheet(STYLESHEET_LABEL_SECONDARY);
    }
}