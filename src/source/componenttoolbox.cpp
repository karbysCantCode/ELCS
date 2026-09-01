#include "componenttoolbox.h"


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
const char* const TOOLBOX_PIN_SENTINEL = "\x01__elcs_pin__\x01";

componentToolbox* simulatorCircuitToolbox = nullptr;
componentToolbox* styleCircuitToolbox = nullptr;

componentToolbox::componentToolbox(QWidget* parent)
    : QScrollArea(parent)
{

}















ToolboxElement::ToolboxElement(
    SentinelComponent& component,
    QWidget* parent
)
    : QFrame(parent),
      component(component)
{
  const QString displayName = component.getAppearanceName().empty()
      ? QString::fromStdString(component.getName())
      : QString::fromStdString(component.getAppearanceName());

  nameLabel = new QLabel(
    displayName,
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

PinToolboxButton::PinToolboxButton(QWidget* parent)
    : QFrame(parent)
{
  auto* label = new QLabel("Pin", this);

  setStyleSheet(STYLESHEET_TOOLBOX_ITEM);
  setAttribute(Qt::WA_Hover);
  label->setStyleSheet(STYLESHEET_TOOLBOX_ITEM_LABEL);
  label->setAlignment(Qt::AlignCenter);
  label->setWordWrap(true);
  label->setAttribute(Qt::WA_TransparentForMouseEvents);

  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(14, 12, 14, 12);
  layout->addWidget(label);

  setMinimumHeight(48);
  setFrameShape(QFrame::StyledPanel);
  setCursor(Qt::PointingHandCursor);
}

void PinToolboxButton::mousePressEvent(QMouseEvent* event)
{
  if (event->button() == Qt::LeftButton)
  {
    dragStartPosition = event->pos();
  }

  QFrame::mousePressEvent(event);
}

void PinToolboxButton::mouseMoveEvent(QMouseEvent* event)
{
  if (!(event->buttons() & Qt::LeftButton))
    return;

  if ((event->pos() - dragStartPosition).manhattanLength() < QApplication::startDragDistance())
    return;

  auto* mimeData = new QMimeData();
  mimeData->setData(TOOLBOX_COMPONENT_MIME_TYPE, QByteArray(TOOLBOX_PIN_SENTINEL));

  auto* drag = new QDrag(this);
  drag->setMimeData(mimeData);
  drag->setPixmap(grab());
  drag->setHotSpot(event->pos());

  drag->exec(Qt::CopyAction);
}

void PinToolboxButton::mouseReleaseEvent(QMouseEvent* event)
{
  if (event->button() == Qt::LeftButton &&
      (event->pos() - dragStartPosition).manhattanLength() < QApplication::startDragDistance())
  {
    emit pinSelected();
  }

  QFrame::mouseReleaseEvent(event);
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

void componentToolbox::addPinButton(std::function<void()> onClick)
{
    if (pinButton || !scrollArea || !layout)
        return;

    pinButton = new PinToolboxButton(scrollArea);

    QObject::connect(pinButton, &PinToolboxButton::pinSelected, scrollArea, std::move(onClick));

    layout->insertWidget(0, pinButton);
}