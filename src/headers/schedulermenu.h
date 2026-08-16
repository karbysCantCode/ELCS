#ifndef SCHEDULERMENU_H
#define SCHEDULERMENU_H

#include <QMenu>

class QAction;
class QTimer;

class SchedulerMenu : public QMenu
{
    Q_OBJECT

public:
    explicit SchedulerMenu(QWidget* parent = nullptr);

private slots:
    void onPlayPauseTriggered();
    void onStepTriggered();
    void onSprintTriggered();
    void pollScheduler();

private:
    QAction* playPauseAction;
    QAction* stepAction;
    QAction* sprintAction;
    QAction* tickCountAction;
    QTimer* pollTimer;

    int sprintCount = 100;

    bool playMode = false;
    bool waitingOnFiniteRun = false;

    void startFiniteRun(int tickCount);
    void refreshActionsEnabled();
    void refreshStatusDisplay();
};

#endif