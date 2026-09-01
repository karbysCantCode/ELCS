#include "tutorialtoolbox.h"

#include "circuitworkspace.h"
#include "component.h"
#include "componenttoolbox.h"

#include <QVBoxLayout>

TutorialToolbox::TutorialToolbox(CircuitWorkspace& workspace, QWidget* parent)
    : QWidget(parent), workspace(workspace)
{
    buttonLayout = new QVBoxLayout(this);
    buttonLayout->addStretch();
}

void TutorialToolbox::clear()
{
    while (buttonLayout->count() > 0)
    {
        QLayoutItem* item = buttonLayout->takeAt(0);
        if (item->widget())
            delete item->widget();
        delete item;
    }

    buttonLayout->addStretch();
}

void TutorialToolbox::setAvailableComponents(
    const std::vector<SentinelComponent*>& components,
    bool includePinButton
)
{
    clear();

    
    
    delete buttonLayout->takeAt(buttonLayout->count() - 1);

    for (SentinelComponent* component : components)
    {
        if (!component)
            continue;

        auto* element = new ToolboxElement(*component, this);

        connect(element, &ToolboxElement::componentSelected, this, [this](SentinelComponent& selected)
        {
            workspace.onComponentSelected(selected);
            emit componentButtonClicked(&selected);
        });

        buttonLayout->addWidget(element);
    }

    if (includePinButton)
    {
        auto* pinButton = new PinToolboxButton(this);

        connect(pinButton, &PinToolboxButton::pinSelected, this, [this]()
        {
            workspace.startPlacingPin();
            emit pinButtonClicked();
        });

        buttonLayout->addWidget(pinButton);
    }

    buttonLayout->addStretch();
}