#include "componenttoolbox.h"

#include "ui_componentToolboxElement.h"

componentToolbox::componentToolbox(QWidget* parent)
    : QScrollArea(parent)
{

}


ToolboxElement::ToolboxElement(const Component& _component, QWidget* parent)
    : QWidget(parent),
    component(_component),
    ui(new Ui::componentToolboxElement)
{
    ui->setupUi(this);
}

ToolboxElement::~ToolboxElement()
{
    delete ui;
}

void componentToolbox::updateElements() {
    for (const auto& [name, component] : globalProjectManager->components) {
        if (toolboxElements.find(name) == toolboxElements.end()) {
            toolboxElements.emplace(name, component);
        }
    }

    for (auto it = toolboxElements.begin(); it != toolboxElements.end(); ) {
        const auto& name = it->first;

        if (globalProjectManager->components.find(name) == globalProjectManager->components.end()) {
            it = toolboxElements.erase(it);
        } else {
            ++it;
        }
    }
}