#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "notifications.h"
#include "projectmanager.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  __circuitworkspace = ui->SimulatorCircuitWorkspace;
}

MainWindow::~MainWindow()
{
    delete ui;
}

#include "overlaytextentry.h"
#include <iostream>

bool validateCircuitName(const QString& string) {
  if (!(string.length() > 0)) {
    globalNotificationManager->notify("Circuit Creation Error", "Name cannot be empty.");
    return false;
  }

  //create
  globalProjectManager->createNewComponent(string.toStdString());
  return true;
}


void MainWindow::on_createNewCircuitButton_clicked()
{
  OverlayTextEntry *textEntry = new OverlayTextEntry(this, validateCircuitName);
  textEntry->raise();
  textEntry->show();
}
