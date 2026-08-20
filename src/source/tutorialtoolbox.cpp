#include "tutorialtoolbox.h"

#include "circuitworkspace.h"
#include "component.h"
#include "componenttoolbox.h"
#include "styles.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QMouseEvent>

#include <functional>

namespace {

class TutorialToolboxPinButton : public QFrame
{
public:
    TutorialToolboxPinButton(std::function<void()> _onClick, QWidget* parent)
        : QFrame(parent), onClick(std::move(_onClick))
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

protected:
    void mouseReleaseEvent(QMouseEvent* event) override
    {
        if (event->button() == Qt::LeftButton && rect().contains(event->pos()))
            onClick();

        QFrame::mouseReleaseEvent(event);
    }

private:
    std::function<void()> onClick;
};

}

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
        auto* pinButton = new TutorialToolboxPinButton([this]()
        {
            workspace.startPlacingPin();
            emit pinButtonClicked();
        }, this);

        buttonLayout->addWidget(pinButton);
    }

    buttonLayout->addStretch();
}