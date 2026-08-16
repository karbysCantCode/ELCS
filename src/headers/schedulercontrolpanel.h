#ifndef SCHEDULERCONTROLPANEL_H
#define SCHEDULERCONTROLPANEL_H

#include <QWidget>

class QPushButton;
class QSpinBox;
class QLabel;
class QTimer;

/*
    Self-contained control panel for the propagation Scheduler
    (globalScheduler). Drop it into any layout you like -- it doesn't
    need to be told about the workspace or scheduler, it talks to the
    existing globals directly, the same way the rest of the app talks
    to globalProjectManager / globalNotificationManager.

        auto* panel = new SchedulerControlPanel(someParent);
        someLayout->addWidget(panel);

    Controls:
      - Play/Pause: toggles globalScheduler between a continuous,
        full-speed run (runTicks(0)) and stopped (stopTicks()).
      - Step: runs exactly one tick (runTicks(1)).
      - Sprint: runs the tick count from the spin box (runTicks(n)).

    While anything is running, this panel repaints the active circuit
    workspace (globalProjectManager->workspace) on a GUI-thread poll
    timer, since the scheduler's worker thread runs propagation on a
    background std::thread and can't safely touch the QGraphicsScene
    itself. The same poll also drives the tick counter / status
    readout, and detects when a Step/Sprint run finishes so the
    controls re-enable themselves automatically.
*/
class SchedulerControlPanel : public QWidget
{
    Q_OBJECT

public:
    explicit SchedulerControlPanel(QWidget* parent = nullptr);

private slots:
    void onPlayPauseClicked();
    void onStepClicked();
    void onSprintClicked();

    // Ticks along at ~60Hz while the scheduler is doing anything, to
    // repaint the workspace and keep the status/tick readout live.
    void pollScheduler();

private:
    QPushButton* playPauseButton;
    QPushButton* stepButton;
    QPushButton* sprintButton;
    QSpinBox* sprintCountSpinBox;
    QLabel* statusLabel;
    QLabel* tickCountLabel;

    QTimer* pollTimer;

    // True from the moment the "Play" toggle is on until the user
    // pauses it again. Distinct from Scheduler::isRunning(), which
    // is also true mid-Step/Sprint.
    bool playMode = false;

    // True while a Step/Sprint run has been kicked off and we're
    // waiting for the worker thread to finish, so Step/Sprint/Play
    // can be disabled for the duration and re-enabled the instant it
    // completes.
    bool waitingOnFiniteRun = false;

    void startFiniteRun(int tickCount);
    void refreshControlsEnabled();
    void refreshStatusDisplay();
};

#endif // SCHEDULERCONTROLPANEL_H