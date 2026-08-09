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
        SentinelComponent& component,
        QWidget* parent = nullptr
    );

    const SentinelComponent& getComponent() const;

signals:
    void componentSelected(SentinelComponent& component);
    void componentEditRequested(const SentinelComponent& component);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;

private:
    SentinelComponent& component;
    QLabel* nameLabel = nullptr;
};

using ToolboxConnectionFunction =
    std::function<void(ToolboxElement*)>;
class componentToolbox : public QScrollArea
{
    Q_OBJECT
private:
    QVBoxLayout* layout;
    QWidget* scrollArea = nullptr;


    std::vector<ToolboxConnectionFunction> registeredConnections;

    void applyConnections(ToolboxElement* element);
public:
    void registerConnection(
            ToolboxConnectionFunction connection
    );
    std::unordered_map<std::string, ToolboxElement*> toolboxElements;

    void initScrollArea(QWidget* _scrollArea);
    void updateElements();
    componentToolbox(QWidget* parent = nullptr);

    
};

extern componentToolbox* simulatorCircuitToolbox;
extern componentToolbox* styleCircuitToolbox;
#endif // COMPONENTTOOLBOX_H
