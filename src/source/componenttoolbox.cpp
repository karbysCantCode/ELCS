#include "componenttoolbox.h"

#include "ui_componentToolboxElement.h"

componentToolbox::componentToolbox(QWidget* parent)
    : QScrollArea(parent)
{
    
}


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