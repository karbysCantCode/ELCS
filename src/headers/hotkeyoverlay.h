#ifndef HOTKEYOVERLAY_H
#define HOTKEYOVERLAY_H

#include <QWidget>

#include <string>

class QPushButton;
class QVBoxLayout;

class HotkeyOverlay : public QWidget
{
    Q_OBJECT

public:
    explicit HotkeyOverlay(const std::string& context, QWidget* parent);

    void reposition();

protected:
    void showEvent(QShowEvent* event) override;

private slots:
    void refresh();
    void toggleExpanded();

private:
    std::string p_context;

    QPushButton* toggleButton = nullptr;
    QWidget* panel = nullptr;
    QVBoxLayout* panelLayout = nullptr;

    bool expanded = false;

    void rebuildPanelContents();
};

#endif 