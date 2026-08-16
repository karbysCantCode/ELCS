#include "schedulermenu.h"
#include "scheduler.h"
#include "projectmanager.h"

#include <QAction>
#include <QTimer>
#include <QInputDialog>

namespace {
constexpr int POLL_INTERVAL_MS = 16;
}

SchedulerMenu::SchedulerMenu(QWidget* parent)
    : QMenu("Scheduler", parent)
{
    playPauseAction = addAction("\u25B6 Play");
    stepAction = addAction("Step");
    sprintAction = addAction("Sprint...");

    addSeparator();
    tickCountAction = addAction("Tick: 0");
    tickCountAction->setEnabled(false);

    connect(playPauseAction, &QAction::triggered, this, &SchedulerMenu::onPlayPauseTriggered);
    connect(stepAction, &QAction::triggered, this, &SchedulerMenu::onStepTriggered);
    connect(sprintAction, &QAction::triggered, this, &SchedulerMenu::onSprintTriggered);

    pollTimer = new QTimer(this);
    pollTimer->setInterval(POLL_INTERVAL_MS);
    connect(pollTimer, &QTimer::timeout, this, &SchedulerMenu::pollScheduler);

    refreshActionsEnabled();
    refreshStatusDisplay();
}

void SchedulerMenu::onPlayPauseTriggered()
{
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

    refreshActionsEnabled();
    refreshStatusDisplay();
}

void SchedulerMenu::onStepTriggered()
{
    startFiniteRun(1);
}

void SchedulerMenu::onSprintTriggered()
{
    if (playMode || waitingOnFiniteRun)
        return;

    bool ok = false;
    int count = QInputDialog::getInt(this, "Sprint", "Number of ticks:", sprintCount, 1, 1000000, 1, &ok);

    if (!ok)
        return;

    sprintCount = count;
    startFiniteRun(count);
}

void SchedulerMenu::startFiniteRun(int tickCount)
{
    if (playMode || waitingOnFiniteRun)
        return;

    waitingOnFiniteRun = true;
    globalScheduler->runTicks(tickCount);

    if (!pollTimer->isActive())
        pollTimer->start();

    refreshActionsEnabled();
    refreshStatusDisplay();
}

void SchedulerMenu::pollScheduler()
{
    if (globalProjectManager && globalProjectManager->workspace)
        globalProjectManager->workspace->viewport()->update();

    refreshStatusDisplay();

    if (!globalScheduler->isRunning())
    {
        playMode = false;
        waitingOnFiniteRun = false;

        pollTimer->stop();

        refreshActionsEnabled();
        refreshStatusDisplay();
    }
}

void SchedulerMenu::refreshActionsEnabled()
{
    playPauseAction->setEnabled(!waitingOnFiniteRun);
    stepAction->setEnabled(!playMode && !waitingOnFiniteRun);
    sprintAction->setEnabled(!playMode && !waitingOnFiniteRun);

    playPauseAction->setText(playMode ? "\u23F8 Pause" : "\u25B6 Play");
}

void SchedulerMenu::refreshStatusDisplay()
{
    tickCountAction->setText(QString("Tick: %1").arg(globalScheduler->getCurrentTick()));
}