#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "notifications.h"
#include "projectmanager.h"
#include "propertysection.h"
#include "componenttoolbox.h"
#include "circuitstyleworkspace.h"
#include "tutorialoverlay.h"
#include "tutorialtoolbox.h"
#include "schedulermenu.h"
#include "hotkeymanager.h"
#include "styles.h"
#include "genericcomponenttoolbox.h"

#include <QWidget>
#include <QStyle>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QTimer>
#include <QSpacerItem>

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
  __circuitStyleWorkspace = ui->StyleCircuitWorkspace;

  menuBar()->setStyleSheet(STYLESHEET_MENUBAR);
  menuBar()->addMenu(new SchedulerMenu(this));
  
  

  qDebug() << "menuBar =" << menuBar();
qDebug() << "isVisible =" << menuBar()->isVisible();
qDebug() << "actions =" << menuBar()->actions().size();
qDebug() << "menuBar visible:" << menuBar()->isVisible();
qDebug() << "menuBar native:" << menuBar()->isNativeMenuBar();
qDebug() << "menuBar parent:" << menuBar()->parent();
qDebug() << "menuBar geometry:" << menuBar()->geometry();

QAction* test = menuBar()->addAction("TEST");
test->setMenuRole(QAction::NoRole);
qDebug() << "after adding:";
qDebug() << "actions =" << menuBar()->actions().size();

for (QAction* action : menuBar()->actions())
    qDebug() << action->text();

  __circuitStyleWorkspace->setCurveButton(ui->AddCurveButton);
  __circuitStyleWorkspace->setLineButton(ui->AddLineButton);
  __circuitStyleWorkspace->setLabelButton(ui->AddLabelButton);
  
  
  

  auto* propertyHolderLayout = new QVBoxLayout(ui->propertyHolder);
  propertyHolderLayout->setContentsMargins(0,0,0,0);
  auto* section = new PropertySection();
  section->setTitle("Properties");
  propertyHolderLayout->addWidget(section);

  simulatorCircuitToolbox = ui->SimulatorCircuitToolbox;
  simulatorCircuitToolbox->initScrollArea(ui->SimulatorCircuitToolboxScrollArea);
  styleCircuitToolbox = ui->StyleCircuitToolbox;
  styleCircuitToolbox->initScrollArea(ui->StyleCircuitToolboxScrollArea);

  simulatorCircuitToolbox->registerConnection(
    [](ToolboxElement* element)
    {
      QObject::connect(
        element,
        &ToolboxElement::componentEditRequested,
        [](const SentinelComponent& component) {
          globalProjectManager->onComponentEditRequested(component);
        }
      );
    }
  );
  simulatorCircuitToolbox->registerConnection(
    [](ToolboxElement* element)
    {
      QObject::connect(
        element,
        &ToolboxElement::componentEditRequested,
        [](const SentinelComponent& component) {
          globalProjectManager->onComponentEditRequested(component);
        }
      );
    }
  );
  simulatorCircuitToolbox->registerConnection(
    [](ToolboxElement* element)
    {
      QObject::connect(
        element,
        &ToolboxElement::componentSelected,
        globalProjectManager->workspace,
        &CircuitWorkspace::onComponentSelected
      );
    }
  );

  styleCircuitToolbox->registerConnection(
    [](ToolboxElement* element)
    {
      QObject::connect(
        element,
        &ToolboxElement::componentSelected,
        globalProjectManager->styleWorkspace,
        &CircuitStyleWorkspace::setComponent
      );
    }
  );

  auto overlay = new TutorialOverlay(ui->centralwidget);
  __tutorialOverlay = overlay;

  QObject::connect(ui->tabWidget, &QTabWidget::currentChanged, this, [overlay](int index) {
    qDebug() << "[DEBUG] tabWidget currentChanged index=" << index
             << " tutorialActive=" << globalProjectManager->tutorialManager.isActive();

    if (!globalProjectManager->tutorialManager.isActive())
    {
      overlay->hideInstruction();
    }
  });

  auto* generic = new GenericComponentToolbox(*__circuitworkspace, ui->toolbar);
  ui->toolbar->layout()->addWidget(generic);
  __tutorialToobox = new TutorialToolbox(*__circuitworkspace,ui->toolbar);
  ui->toolbar->layout()->addWidget(__tutorialToobox);
  auto* stretch = new QSpacerItem(
    40,                      
    20,                      
    QSizePolicy::Expanding,  
    QSizePolicy::Minimum     
  );
  ui->toolbar->layout()->addItem(stretch);

  QTimer::singleShot(0, this, [generic, section]()
  {
    auto syncToolboxVisibility = [generic]()
    {
      const bool tutorialActive = globalProjectManager->tutorialManager.isActive();
      generic->setVisible(!tutorialActive);
      __tutorialToobox->setVisible(tutorialActive);
    };

    syncToolboxVisibility();

    QObject::connect(&globalProjectManager->tutorialManager, &TutorialManager::tutorialStarted, generic, syncToolboxVisibility);
    QObject::connect(&globalProjectManager->tutorialManager, &TutorialManager::tutorialCompleted, generic, syncToolboxVisibility);
    QObject::connect(&globalProjectManager->tutorialManager, &TutorialManager::tutorialCancelled, generic, syncToolboxVisibility);

    globalProjectManager->initiateSimulatorUIPropertyManager(section);

    QObject::connect(__circuitworkspace, &CircuitWorkspace::itemSelected, __circuitworkspace, [](AbstractPropagator* propagator)
    {
      globalProjectManager->onPropagatorSelected(propagator);
    });

    QObject::connect(__circuitworkspace, &CircuitWorkspace::selectionCleared, __circuitworkspace, []()
    {
      globalProjectManager->onSelectionCleared();
    });

    simulatorCircuitToolbox->addPinButton([]()
    {
      globalProjectManager->workspace->startPlacingPin();
    });
  });
  

  repolishVariants(this);

  globalHotkeyManager->registerActionHotkey(
    "global",
    QKeySequence(Qt::CTRL | Qt::Key_S),
    "Save current circuit",
    this,
    [this]() {
      if (this->ui->simulatorPage->isVisible()) {
        globalProjectManager->saveCurrentComponent();
        globalNotificationManager->notify("Save", std::format("Successfully saved component \"{}\" to \"{}\"", globalProjectManager->currentOpenComponent->getName(), globalProjectManager->currentOpenComponent->getFilePath().string()));
      } else {
        qDebug("NAHHH!!");
      }
    }
  );
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


void MainWindow::on_AddCurveButton_clicked()
{
    globalProjectManager->styleWorkspace->setToolAddCurve();
    ui->AddCurveButton->setChecked(true);
    ui->AddLineButton->setChecked(false);
    ui->AddLabelButton->setChecked(false);
}


void MainWindow::on_AddLineButton_clicked()
{
  globalProjectManager->styleWorkspace->setToolAddLine();
  ui->AddCurveButton->setChecked(false);
  ui->AddLineButton->setChecked(true);
  ui->AddLabelButton->setChecked(false);
}


void MainWindow::on_AddLabelButton_clicked()
{
  globalProjectManager->styleWorkspace->setToolAddLabel();
  ui->AddCurveButton->setChecked(false);
  ui->AddLineButton->setChecked(false);
  ui->AddLabelButton->setChecked(true);
}
void MainWindow::on_OpenProjectButton_clicked()
{

}


void MainWindow::on_CreateProjectButton_clicked()
{

}


void MainWindow::on_SettingsButton_clicked()
{

}


void MainWindow::on_getStartedButton_clicked()
{

}

