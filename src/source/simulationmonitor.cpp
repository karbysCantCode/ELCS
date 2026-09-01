#include "simulationmonitor.h"
#include "component.h"
#include "scheduler.h"
#include "projectmanager.h"
#include "notifications.h"

#include <QTimer>
#include <QSettings>

namespace {
constexpr int POLL_INTERVAL_MS = 300;
}

SimulationMonitor* globalSimulationMonitor = nullptr;

SimulationMonitor::SimulationMonitor(QObject* parent)
    : QObject(parent)
{
    pollTimer = new QTimer(this);
    pollTimer->setInterval(POLL_INTERVAL_MS);

    connect(pollTimer, &QTimer::timeout, this, &SimulationMonitor::poll);

    pollTimer->start();
}

bool SimulationMonitor::alertOnFloatingPins() const
{
    QSettings settings("ELCS", "ELCS");
    return settings.value("simulation/alertOnFloatingPins", false).toBool();
}

void SimulationMonitor::setAlertOnFloatingPins(bool enabled)
{
    QSettings settings("ELCS", "ELCS");
    settings.setValue("simulation/alertOnFloatingPins", enabled);

    knownFloatingPins.clear();

    emit settingsChanged();
}

bool SimulationMonitor::pauseOnConflict() const
{
    QSettings settings("ELCS", "ELCS");
    return settings.value("simulation/pauseOnConflict", false).toBool();
}

void SimulationMonitor::setPauseOnConflict(bool enabled)
{
    QSettings settings("ELCS", "ELCS");
    settings.setValue("simulation/pauseOnConflict", enabled);

    conflictAlertShown = false;

    emit settingsChanged();
}

void SimulationMonitor::scan(
    Component* container,
    std::vector<Pin*>& floatingPins,
    std::vector<Propagator*>& conflictedPropagators
) const
{
    if (!container)
        return;

    for (const auto& propagatorPtr : container->getPropagators())
    {
        AbstractPropagator* abstractPropagator = propagatorPtr.get();

        if (abstractPropagator->isAbstract())
        {
            scan(static_cast<Component*>(abstractPropagator), floatingPins, conflictedPropagators);
            continue;
        }

        auto* propagator = static_cast<Propagator*>(abstractPropagator);
        const States state = propagator->getEffectingState();

        if (state == States::CONFLICT || state == States::ERROR)
            conflictedPropagators.push_back(propagator);

        if (state == States::FLOATING && propagator->getKind() == Propagator::Kinds::PIN)
            floatingPins.push_back(static_cast<Pin*>(propagator));
    }
}

void SimulationMonitor::poll()
{
    if (!globalProjectManager || !globalProjectManager->currentOpenComponent)
        return;

    const bool checkFloating = alertOnFloatingPins();
    const bool checkConflict = pauseOnConflict();

    if (!checkFloating && !checkConflict)
        return;

    std::vector<Pin*> floatingPins;
    std::vector<Propagator*> conflictedPropagators;

    scan(
        static_cast<Component*>(globalProjectManager->currentOpenComponent),
        floatingPins,
        conflictedPropagators
    );

    if (checkFloating)
    {
        std::unordered_set<Pin*> currentFloating(floatingPins.begin(), floatingPins.end());

        size_t newlyFloating = 0;
        for (Pin* pin : currentFloating)
        {
            if (knownFloatingPins.find(pin) == knownFloatingPins.end())
                newlyFloating++;
        }

        if (newlyFloating > 0)
        {
            globalNotificationManager->notify(
                "Unconnected pin",
                newlyFloating == 1
                    ? "A pin is floating (unconnected)."
                    : std::to_string(newlyFloating) + " pins are floating (unconnected).",
                2500
            );
        }

        knownFloatingPins = std::move(currentFloating);
    }

    if (checkConflict)
    {
        if (!conflictedPropagators.empty())
        {
            if (globalScheduler->isRunning())
                globalScheduler->stopTicks();

            if (!conflictAlertShown)
            {
                globalNotificationManager->notify(
                    "Simulation paused",
                    conflictedPropagators.size() == 1
                        ? "Paused: a conflict/error was detected on one pin/wire."
                        : "Paused: conflicts/errors were detected on " + std::to_string(conflictedPropagators.size()) + " pins/wires.",
                    3500
                );

                conflictAlertShown = true;
            }
        }
        else
        {
            conflictAlertShown = false;
        }
    }
}