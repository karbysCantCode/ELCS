#include "componenttoolbox.h"

componentToolbox::componentToolbox(QWidget* parent)
    : QScrollArea(parent)
{}

#include "ui_componentToolboxElement.h"

ToolboxElement::ToolboxElement(QWidget* parent)
    : QWidget(parent),
    ui(new Ui::componentToolboxElement)
{
    ui->setupUi(this);
}

ToolboxElement::~ToolboxElement()
{
    delete ui;
}