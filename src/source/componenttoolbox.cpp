#include "componenttoolbox.h"

// #include "ui_componentToolboxElement.h"
#include "projectmanager.h"
#include "circuitworkspace.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QDrag>
#include <QMimeData>
#include <QApplication>
#include "styles.h"

const char* const TOOLBOX_COMPONENT_MIME_TYPE = "application/x-elcs-component";

componentToolbox* simulatorCircuitToolbox = nullptr;
componentToolbox* styleCircuitToolbox = nullptr;

componentToolbox::componentToolbox(QWidget* parent)
    : QScrollArea(parent)
{

}


// ToolboxElement::ToolboxElement(const Component& _component, QWidget* parent)
//     : QWidget(parent),
//     component(_component),
//     ui(new Ui::componentToolboxElement)
// {
//     ui->setupUi(this);
// }

// ToolboxElement::~ToolboxElement()
// {
//     delete ui;
// }

ToolboxElement::ToolboxElement(
    SentinelComponent& component,
    QWidget* parent
)
    : QFrame(parent),
      component(component)
{
  nameLabel = new QLabel(
    QString::fromStdString(component.getName()),
    this
  );

  this->setStyleSheet(STYLESHEET_TOOLBOX_ITEM);
  this->setAttribute(Qt::WA_Hover);
  nameLabel->setStyleSheet(STYLESHEET_TOOLBOX_ITEM_LABEL);
  nameLabel->setAlignment(Qt::AlignCenter);
  nameLabel->setWordWrap(true);
  nameLabel->setAttribute(Qt::WA_TransparentForMouseEvents);

  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(14, 12, 14, 12);
  layout->addWidget(nameLabel);

  setMinimumHeight(48);
  setFrameShape(QFrame::StyledPanel);
  setCursor(Qt::PointingHandCursor);
}

const SentinelComponent& ToolboxElement::getComponent() const
{
  return component;
}

void ToolboxElement::mousePressEvent(QMouseEvent* event)
{
  if (event->button() == Qt::LeftButton)
  {
    dragStartPosition = event->pos();
  }

  QFrame::mousePressEvent(event);
}

void ToolboxElement::mouseMoveEvent(QMouseEvent* event)
{
  if (!(event->buttons() & Qt::LeftButton))
    return;

  if ((event->pos() - dragStartPosition).manhattanLength() < QApplication::startDragDistance())
    return;

  auto* mimeData = new QMimeData();
  mimeData->setData(TOOLBOX_COMPONENT_MIME_TYPE, QByteArray::fromStdString(component.getName()));

  auto* drag = new QDrag(this);
  drag->setMimeData(mimeData);
  drag->setPixmap(grab());
  drag->setHotSpot(event->pos());

  drag->exec(Qt::CopyAction);
}

void ToolboxElement::mouseReleaseEvent(QMouseEvent* event)
{
  if (event->button() == Qt::LeftButton &&
      (event->pos() - dragStartPosition).manhattanLength() < QApplication::startDragDistance())
  {
    emit componentSelected(component);
  }

  QFrame::mouseReleaseEvent(event);
}

void ToolboxElement::mouseDoubleClickEvent(QMouseEvent* event)
{
  if (event->button() == Qt::LeftButton)
  {
    emit componentEditRequested(component);
  }
}

void componentToolbox::initScrollArea(QWidget* _scrollArea) {
  scrollArea = _scrollArea;
  layout = new QVBoxLayout(scrollArea);

  layout->setContentsMargins(4, 4, 4, 4);
  layout->setSpacing(4);

  layout->addStretch();

  scrollArea->setStyleSheet(STYLESHEET_WIDGET_SECONDARY);
}

void componentToolbox::registerConnection(
    ToolboxConnectionFunction connection
)
{
    registeredConnections.push_back(
        std::move(connection)
    );
}

void componentToolbox::applyConnections(
    ToolboxElement* element
)
{
    for (const auto& connection : registeredConnections)
    {
        connection(element);
    }
}

void componentToolbox::updateElements()
{
    for (const auto& [name, component]
         : globalProjectManager->components)
    {
        if (toolboxElements.find(name)
            == toolboxElements.end())
        {
            auto* ptr =
                new ToolboxElement(
                    *component.get(),
                    scrollArea
                );

            toolboxElements.emplace(
                name,
                ptr
            );

            layout->insertWidget(
                layout->count() - 1,
                ptr
            );

            applyConnections(ptr);
        }
    }

    for (auto it = toolboxElements.begin();
         it != toolboxElements.end();)
    {
        const auto& name = it->first;

        if (globalProjectManager->components.find(name)
            == globalProjectManager->components.end())
        {
            layout->removeWidget(it->second);
            it->second->deleteLater();

            it = toolboxElements.erase(it);
        }
        else
        {
            ++it;
        }
    }
}