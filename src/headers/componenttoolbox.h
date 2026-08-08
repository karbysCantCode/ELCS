#ifndef COMPONENTTOOLBOX_H
#define COMPONENTTOOLBOX_H

#include <QObject>
#include <QWidget>
#include <QScrollArea>
#include <QLabel>

#include "projectmanager.h"

class ToolboxElement : public QFrame
{
    Q_OBJECT

public:
    explicit ToolboxElement(
        const Component& component,
        QWidget* parent = nullptr
    );

    const Component& getComponent() const;

signals:
    void componentSelected(const Component& component);
    void componentEditRequested(const Component& component);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;

private:
    const Component& component;
    QLabel* nameLabel = nullptr;
};

class componentToolbox : public QScrollArea
{
    Q_OBJECT
private:
    QVBoxLayout* layout;
    QWidget* scrollArea = nullptr;
public:
    std::unordered_map<std::string, ToolboxElement*> toolboxElements;

    void initScrollArea(QWidget* _scrollArea);
    void updateElements();
    componentToolbox(QWidget* parent = nullptr);

    
};

extern componentToolbox* simulatorCircuitToolbox;
extern componentToolbox* styleCircuitToolbox;
#endif // COMPONENTTOOLBOX_H
