#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "notifications.h"
#include "projectmanager.h"
#include "propertysection.h"
#include "componenttoolbox.h"

#include <QWidget>
#include <QStyle>

void repolishVariants(QWidget* widget)
{
    if (!widget)
        return;

    if (widget->property("variant").isValid())
    {
        widget->style()->unpolish(widget);
        widget->style()->polish(widget);
        widget->update();
    }

    const auto children = widget->findChildren<QWidget*>(
        QString(),
        Qt::FindDirectChildrenOnly);

    for (QWidget* child : children)
        repolishVariants(child);
}

#include <QShortcut>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  __circuitworkspace = ui->SimulatorCircuitWorkspace;

  auto* propertyHolderLayout = new QVBoxLayout(ui->propertyHolder);
  propertyHolderLayout->setContentsMargins(0,0,0,0);
  auto* section = new PropertySection();
  section->setTitle("Properties");
  propertyHolderLayout->addWidget(section);

  simulatorCircuitToolbox = ui->SimulatorCircuitToolbox;
  simulatorCircuitToolbox->initScrollArea(ui->SimulatorCircuitToolboxScrollArea);
  // styleCircuitToolbox = 
  // leave this past any ui creation

  repolishVariants(this);

  QShortcut* saveShortcut = new QShortcut(
    QKeySequence(Qt::CTRL | Qt::Key_S),
    this
  );

  connect(saveShortcut, &QShortcut::activated, this, [this](){
    if (this->ui->simulatorPage->isVisible()) {
      globalProjectManager->saveCurrentComponent();
      globalNotificationManager->notify("Save", std::format("Successfully saved component \"{}\" to \"{}\"", globalProjectManager->currentOpenComponent->name, globalProjectManager->currentOpenComponent->filePath.string()));
    } else {
      qDebug("NAHHH!!");
    }
  });
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


void MainWindow::on_selectTool_clicked()
{
    globalProjectManager->workspace->setState(CircuitWorkspace::EditingStates::EDIT);
}


void MainWindow::on_pokeTool_clicked()
{
    globalProjectManager->workspace->setState(CircuitWorkspace::EditingStates::POKE);
}

