#ifndef SETTINGSPANEL_H
#define SETTINGSPANEL_H

#include <QWidget>

#include <unordered_map>
#include <string>

class QFrame;
class QVBoxLayout;
class QHBoxLayout;
class QScrollArea;
class QCheckBox;
class QLabel;
class QPushButton;
class QEvent;
class QMouseEvent;
class QPaintEvent;

class SettingsPanel : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsPanel(QWidget* parent);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    void onShowOverlayToggled(bool checked);
    void onAlertOnFloatingPinsToggled(bool checked);
    void onPauseOnConflictToggled(bool checked);
    void rebuildContextChecks();

private:
    QFrame* card = nullptr;
    QScrollArea* scrollArea = nullptr;
    QVBoxLayout* contentLayout = nullptr;

    QWidget* generalSectionMarker = nullptr;
    QWidget* simulationSectionMarker = nullptr;
    QWidget* hotkeysSectionMarker = nullptr;
    QVBoxLayout* hotkeysSectionLayout = nullptr;

    std::unordered_map<QCheckBox*, std::string> contextChecks;

    QWidget* addSection(const QString& title);
    void addJumpButton(QHBoxLayout* row, const QString& label, QWidget* sectionMarker);
    void jumpToSection(QWidget* sectionMarker);
};

#endif // SETTINGSPANEL_H