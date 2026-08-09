#include <QApplication>
#include "mainwindow.h"
#include "notifications.h"
#include "projectmanager.h"
#include <iostream>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
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
