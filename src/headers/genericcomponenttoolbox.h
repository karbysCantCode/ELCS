#ifndef GENERICCOMPONENTTOOLBOX_H
#define GENERICCOMPONENTTOOLBOX_H

#include <QWidget>

#include <string>
#include <vector>

class QHBoxLayout;
class QDragEnterEvent;
class QDropEvent;
class CircuitWorkspace;
class ToolboxElement;

class GenericComponentToolbox : public QWidget
{
    Q_OBJECT

public:
    explicit GenericComponentToolbox(CircuitWorkspace& workspace, QWidget* parent = nullptr);

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    CircuitWorkspace& workspace;
    QHBoxLayout* buttonLayout = nullptr;

    std::vector<std::string> loadoutOrder;
    std::vector<QWidget*> elements;

    static QString loadoutFilePath();
    void loadLoadout();
    void saveLoadout() const;

    int indexOfElementAt(const QPoint& localPos) const;
    void removeFromLoadout(const std::string& name);
    void rebuildElements();
};

#endif