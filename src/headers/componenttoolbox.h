#ifndef COMPONENTTOOLBOX_H
#define COMPONENTTOOLBOX_H

#include <QObject>
#include <QWidget>
#include <QScrollArea>

#include "projectmanager.h"

namespace Ui {
class componentToolboxElement;
}

class ToolboxElement : public QWidget {
    Q_OBJECT
public:
    const Component& component;

    explicit ToolboxElement(const Component& _component, QWidget* parent = nullptr);
        ~ToolboxElement();

    
private:
    Ui::componentToolboxElement* ui;
};

class componentToolbox : public QScrollArea
{
    Q_OBJECT
private:
    
public:
    std::unordered_map<std::string, ToolboxElement> toolboxElements;

    void updateElements();
    componentToolbox(QWidget* parent = nullptr);

};

#endif // COMPONENTTOOLBOX_H
