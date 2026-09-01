#ifndef SIMULATIONMONITOR_H
#define SIMULATIONMONITOR_H

#include <QObject>

#include <unordered_set>
#include <vector>

class QTimer;
class Component;
class Pin;
class Propagator;

class SimulationMonitor : public QObject
{
    Q_OBJECT

public:
    explicit SimulationMonitor(QObject* parent = nullptr);

    bool alertOnFloatingPins() const;
    void setAlertOnFloatingPins(bool enabled);

    bool pauseOnConflict() const;
    void setPauseOnConflict(bool enabled);

signals:
    void settingsChanged();

private slots:
    void poll();

private:
    QTimer* pollTimer = nullptr;

    std::unordered_set<Pin*> knownFloatingPins;
    bool conflictAlertShown = false;

    void scan(
        Component* container,
        std::vector<Pin*>& floatingPins,
        std::vector<Propagator*>& conflictedPropagators
    ) const;
};

extern SimulationMonitor* globalSimulationMonitor;

#endif // SIMULATIONMONITOR_H