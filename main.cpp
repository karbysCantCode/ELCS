#include "mainwindow.h"
#include "notifications.h"
#include "projectmanager.h"
#include <QApplication>
#include <iostream>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    globalProjectManager = new ProjectManager();
    globalProjectManager->workspace = __circuitworkspace;
    globalProjectManager->dummyLoad();
    globalNotificationManager = new NotificationManager(&w);
    globalNotificationManager->show();
    globalNotificationManager->raise();
    return QApplication::exec();
}
