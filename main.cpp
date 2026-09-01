#include <QApplication>
#include "mainwindow.h"
#include "notifications.h"
#include "projectmanager.h"
#include "hotkeymanager.h"
#include "simulationmonitor.h"
#include <iostream>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    globalHotkeyManager = new HotkeyManager();
    globalSimulationMonitor = new SimulationMonitor();
    MainWindow w;
    w.show();
    globalNotificationManager = new NotificationManager(&w);
    globalNotificationManager->show();
    globalNotificationManager->raise();
    globalProjectManager = new ProjectManager();
    globalProjectManager->workspace = __circuitworkspace;
    globalProjectManager->styleWorkspace = __circuitStyleWorkspace;
    globalProjectManager->dummyLoad();
    return QApplication::exec();
}