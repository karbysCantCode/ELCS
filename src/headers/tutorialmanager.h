#ifndef TUTORIALMANAGER_H
#define TUTORIALMANAGER_H

#include <QObject>
#include <QHash>
#include <QString>

#include <vector>

#include "tutorialtypes.h"

class QJsonObject;
class CircuitWorkspace;
class TutorialOverlay;
class TutorialToolbox;

/*
    Drives a tutorial: loads a step/condition script from JSON,
    listens to CircuitWorkspace's componentPlaced/pinPlaced/
    wirePlaced/itemDeleted signals, evaluates whether the current
    step's conditions are satisfied, and advances -- updating an
    (optional) TutorialOverlay and (optional) TutorialToolbox as it
    goes.

    ------------------------------------------------------------
    JSON format
    ------------------------------------------------------------
    {
      "title": "AND Gates and Wires",
      "steps": [
        {
          "instructions": "Place two AND gates onto the workspace.",
          "highlight": "toolbox",
          "restrictInput": true,
          "toolbox": ["AND"],
          "conditions": [
            { "type": "placeComponent", "component": "AND", "count": 2, "bind": ["gateX", "gateY"] }
          ]
        },
        {
          "instructions": "Connect an output pin of gateX to an input pin of gateY.",
          "highlight": "workspace",
          "conditions": [
            { "type": "connected", "a": "$gateX", "b": "$gateY" }
          ]
        },
        {
          "instructions": "Nice. Now delete gateY to finish up.",
          "conditions": [
            { "type": "deleted", "target": "$gateY" }
          ]
        }
      ]
    }

    A step advances once *all* of its conditions are satisfied.
    Condition types:

      placeComponent  { component?: sentinel name (any if omitted),
                         count?: N (default 1), bind?: [names] }
        Satisfied once N matching components have been placed since
        this step began. Each placement, in order, is assigned to the
        next name in `bind` (extra placements past the bind list just
        count without being named).

      placePin        { count?: N, bind?: [names] }
        Same idea, for pins.

      placeWire       { count?: N, bind?: [names] }
        Same idea, for wires.

      connected        { a: "$var" or a bound value's name, b: "$var" }
        Satisfied once every pin belonging to `a` (or `a` itself, if
        it's a pin/wire) can reach every pin belonging to `b` by
        walking the effector/affector graph. Re-checked after every
        placement signal, not just once.

      deleted          { target: "$var" }
        Satisfied the instant the object bound to `target` is
        reported via itemDeleted().

    A reference like "$gateX" looks up a variable bound by an earlier
    step's `bind` list; references without a leading "$" aren't
    currently resolved to anything (reserved for future literal
    matching, e.g. by name).
*/
class TutorialManager : public QObject
{
    Q_OBJECT

public:
    TutorialManager(
        CircuitWorkspace& workspace,
        TutorialOverlay* overlay = nullptr,
        TutorialToolbox* toolbox = nullptr,
        QObject* parent = nullptr
    );

    bool loadTutorial(const QString& jsonFilePath);
    bool loadTutorialFromJson(const QJsonObject& root);

    void start();
    void cancel();

    bool isActive() const;
    int currentStep() const { return currentStepIndex; }
    QString title() const { return tutorialTitle; }

    // Returns the propagator bound to "$name" (or an invalid value if
    // there's no such binding yet), for host code that wants to poke
    // at tutorial state directly rather than only through JSON.
    TutorialValue variable(const QString& name) const;

signals:
    void stepChanged(int index, const QString& instructions);
    void tutorialCompleted();
    void tutorialCancelled();

private slots:
    void onComponentPlaced(Component* component);
    void onPinPlaced(Pin* pin);
    void onWirePlaced(Wire* wire);
    void onItemDeleted(AbstractPropagator* propagator);

    // Wired to TutorialOverlay::dismissRequested() (the close/"X"
    // button) -- this is what actually gives the user a way to
    // dismiss the tutorial, whether it's mid-progress (treated as a
    // cancel) or sitting on its last step (treated as completion,
    // since a trailing conditionless "you're done!" step otherwise
    // has no way to know it should go away).
    void onDismissRequested();

private:
    CircuitWorkspace& workspace;
    TutorialOverlay* overlay = nullptr;
    TutorialToolbox* toolbox = nullptr;

    QString tutorialTitle;
    std::vector<TutorialStep> steps;
    int currentStepIndex = -1;

    QHash<QString, TutorialValue> variables;

    TutorialCondition parseCondition(const QJsonObject& obj) const;
    TutorialValue resolveReference(const QString& ref) const;
    std::vector<Propagator*> collectPropagators(const TutorialValue& value) const;
    bool arePropagatorsConnected(const TutorialValue& a, const TutorialValue& b) const;

    void bindNext(TutorialCondition& condition, const TutorialValue& value);
    void reevaluateConnectivityConditions();
    void checkStepCompletion();
    void advanceStep();
    void presentCurrentStep();

    // Shared cleanup for "the tutorial is over" (reached past the
    // last step, or dismissed while sitting on it) -- separate from
    // cancel() so the right signal (tutorialCompleted() vs
    // tutorialCancelled()) fires either way.
    void finishTutorial();

    // Pushes every currently-bound variable's on-screen position to
    // the overlay as a name tag (see TutorialOverlay::setVariableLabels()),
    // so e.g. "gateX"/"gateY" are actually visible next to the
    // specific placed gates they refer to, not just usable in JSON.
    void updateVariableLabels();
};

#endif