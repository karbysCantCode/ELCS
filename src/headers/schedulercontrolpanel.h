#ifndef SCHEDULERCONTROLPANEL_H
#define SCHEDULERCONTROLPANEL_H

#include <QWidget>

class QPushButton;
class QSpinBox;
class QLabel;
class QTimer;

class SchedulerControlPanel : public QWidget
{
    Q_OBJECT

public:
    explicit SchedulerControlPanel(QWidget* parent = nullptr);

private slots:
    void onPlayPauseClicked();
    void onStepClicked();
    void onSprintClicked();

    
    
    void pollScheduler();

private:
    QPushButton* playPauseButton;
    QPushButton* stepButton;
    QPushButton* sprintButton;
    QSpinBox* sprintCountSpinBox;
    QLabel* statusLabel;
    QLabel* tickCountLabel;

    QTimer* pollTimer;

    
    
    
    bool playMode = false;

    
    
    
    
    bool waitingOnFiniteRun = false;

    void startFiniteRun(int tickCount);
    void refreshControlsEnabled();
    void refreshStatusDisplay();
};

#endif 