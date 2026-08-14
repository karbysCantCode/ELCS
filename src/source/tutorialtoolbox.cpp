#include "tutorialtoolbox.h"

#include "circuitworkspace.h"
#include "component.h"

#include <QVBoxLayout>
#include <QPushButton>

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

    // Pull the trailing stretch back out so new buttons land above
    // it, then put it back at the end once they're added.
    delete buttonLayout->takeAt(buttonLayout->count() - 1);

    for (SentinelComponent* component : components)
    {
        if (!component)
            continue;

        auto* button = new QPushButton(QString::fromStdString(component->getName()), this);

        connect(button, &QPushButton::clicked, this, [this, component]()
        {
            workspace.onComponentSelected(*component);
            emit componentButtonClicked(component);
        });

        buttonLayout->addWidget(button);
    }

    if (includePinButton)
    {
        auto* pinButton = new QPushButton("Pin", this);

        connect(pinButton, &QPushButton::clicked, this, [this]()
        {
            workspace.startPlacingPin();
            emit pinButtonClicked();
        });

        buttonLayout->addWidget(pinButton);
    }

    buttonLayout->addStretch();
}