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
    explicit ToolboxElement(QWidget* parent = nullptr);
        ~ToolboxElement();

    
private:
    Ui::componentToolboxElement* ui;
};

class componentToolbox : public QScrollArea
{
    Q_OBJECT
private:
    //void add
public:

    componentToolbox(QWidget* parent = nullptr);
};

#endif // COMPONENTTOOLBOX_H
