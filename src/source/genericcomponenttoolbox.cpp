#include "genericcomponenttoolbox.h"

#include "componenttoolbox.h"
#include "projectmanager.h"
#include "circuitworkspace.h"
#include "styles.h"

#include <QHBoxLayout>
#include <QMouseEvent>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QTimer>
#include <algorithm>

GenericComponentToolbox::GenericComponentToolbox(CircuitWorkspace& _workspace, QWidget* parent)
    : QWidget(parent), workspace(_workspace)
{
    buttonLayout = new QHBoxLayout(this);
    buttonLayout->addStretch();

    setAcceptDrops(true);

    QTimer::singleShot(0, this, [this]()
    {
        loadLoadout();
    });
}

QString GenericComponentToolbox::loadoutFilePath()
{
    return QString(RESOURCES_PATH) + QString("/toolbar_loadout.json");
}

void GenericComponentToolbox::loadLoadout()
{
    QFile file(loadoutFilePath());

    if (!file.open(QIODevice::ReadOnly))
        return;

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);

    if (parseError.error != QJsonParseError::NoError || !doc.isArray())
        return;

    loadoutOrder.clear();

    for (const auto& value : doc.array())
        loadoutOrder.push_back(value.toString().toStdString());

    rebuildElements();
}

void GenericComponentToolbox::saveLoadout() const
{
    QJsonArray array;

    for (const auto& name : loadoutOrder)
        array.append(QString::fromStdString(name));

    QFile file(loadoutFilePath());

    if (!file.open(QIODevice::WriteOnly))
        return;

    file.write(QJsonDocument(array).toJson());
}

int GenericComponentToolbox::indexOfElementAt(const QPoint& localPos) const
{
    for (size_t i = 0; i < elements.size(); i++)
    {
        if (localPos.y() < elements[i]->geometry().center().y())
            return static_cast<int>(i);
    }

    return static_cast<int>(elements.size());
}

void GenericComponentToolbox::removeFromLoadout(const std::string& name)
{
    auto it = std::find(loadoutOrder.begin(), loadoutOrder.end(), name);

    if (it != loadoutOrder.end())
        loadoutOrder.erase(it);
}

void GenericComponentToolbox::rebuildElements()
{
    for (auto* element : elements)
    {
        buttonLayout->removeWidget(element);
        element->removeEventFilter(this);
        element->deleteLater();
    }
    elements.clear();

    for (const auto& name : loadoutOrder)
    {
        auto it = globalProjectManager->components.find(name);

        if (it == globalProjectManager->components.end())
            continue;

        auto* element = new ToolboxElement(*it->second.get(), this);
        element->installEventFilter(this);

        connect(
            element,
            &ToolboxElement::componentSelected,
            &workspace,
            &CircuitWorkspace::onComponentSelected
        );

        buttonLayout->insertWidget(buttonLayout->count() - 1, element);
        elements.push_back(element);
    }
}

void GenericComponentToolbox::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasFormat(TOOLBOX_COMPONENT_MIME_TYPE))
        event->acceptProposedAction();
}

void GenericComponentToolbox::dropEvent(QDropEvent* event)
{
    if (!event->mimeData()->hasFormat(TOOLBOX_COMPONENT_MIME_TYPE))
        return;

    const std::string name = event->mimeData()->data(TOOLBOX_COMPONENT_MIME_TYPE).toStdString();

    int insertIndex = indexOfElementAt(event->position().toPoint());

    auto existing = std::find(loadoutOrder.begin(), loadoutOrder.end(), name);
    if (existing != loadoutOrder.end())
    {
        const int existingIndex = static_cast<int>(existing - loadoutOrder.begin());
        loadoutOrder.erase(existing);

        if (existingIndex < insertIndex)
            insertIndex--;
    }

    if (insertIndex < 0 || insertIndex > static_cast<int>(loadoutOrder.size()))
        insertIndex = static_cast<int>(loadoutOrder.size());

    loadoutOrder.insert(loadoutOrder.begin() + insertIndex, name);

    rebuildElements();
    saveLoadout();

    event->acceptProposedAction();
}

bool GenericComponentToolbox::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::MouseButtonPress)
    {
        auto* mouseEvent = static_cast<QMouseEvent*>(event);

        if (mouseEvent->button() == Qt::RightButton)
        {
            auto* element = qobject_cast<ToolboxElement*>(watched);

            if (element)
            {
                removeFromLoadout(element->getComponent().getName());
                rebuildElements();
                saveLoadout();
                return true;
            }
        }
    }

    return QWidget::eventFilter(watched, event);
}