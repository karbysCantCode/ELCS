#ifndef TUTORIALTOOLBOX_H
#define TUTORIALTOOLBOX_H

#include <QWidget>

#include <vector>

class QVBoxLayout;
class CircuitWorkspace;
class SentinelComponent;


class TutorialToolbox : public QWidget
{
    Q_OBJECT

public:
    explicit TutorialToolbox(CircuitWorkspace& workspace, QWidget* parent = nullptr);

    void setAvailableComponents(
        const std::vector<SentinelComponent*>& components,
        bool includePinButton = false
    );

    void clear();

signals:
    void componentButtonClicked(SentinelComponent* component);
    void pinButtonClicked();

private:
    CircuitWorkspace& workspace;
    QVBoxLayout* buttonLayout = nullptr;
};

#endif