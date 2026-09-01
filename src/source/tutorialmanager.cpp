#include "tutorialmanager.h"

#include "circuitworkspace.h"
#include "tutorialoverlay.h"
#include "tutorialtoolbox.h"
#include "truthtablewidget.h"
#include "projectmanager.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QDebug>
#include <QVector>

#include <unordered_set>

TutorialManager::TutorialManager(
    CircuitWorkspace& workspace,
    TutorialOverlay* overlay,
    TutorialToolbox* toolbox,
    QObject* parent
)
    : QObject(parent),
      workspace(workspace),
      overlay(overlay),
      toolbox(toolbox)
{
    connect(&workspace, &CircuitWorkspace::componentPlaced, this, &TutorialManager::onComponentPlaced);
    connect(&workspace, &CircuitWorkspace::pinPlaced, this, &TutorialManager::onPinPlaced);
    connect(&workspace, &CircuitWorkspace::wirePlaced, this, &TutorialManager::onWirePlaced);
    connect(&workspace, &CircuitWorkspace::itemDeleted, this, &TutorialManager::onItemDeleted);

    if (overlay)
        connect(overlay, &TutorialOverlay::dismissRequested, this, &TutorialManager::onDismissRequested);

    labelPollTimer = new QTimer(this);
    labelPollTimer->setInterval(100);
    connect(labelPollTimer, &QTimer::timeout, this, &TutorialManager::updateVariableLabels);
}

bool TutorialManager::loadTutorial(const QString& jsonFilePath)
{
    QFile file(jsonFilePath);

    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "TutorialManager: couldn't open" << jsonFilePath;
        return false;
    }

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);

    if (parseError.error != QJsonParseError::NoError)
    {
        qWarning() << "TutorialManager: JSON parse error in" << jsonFilePath
                   << ":" << parseError.errorString();
        return false;
    }

    return loadTutorialFromJson(doc.object());
}

bool TutorialManager::loadTutorialFromJson(const QJsonObject& root)
{
    steps.clear();
    variables.clear();
    currentStepIndex = -1;

    tutorialTitle = root.value("title").toString();

    const QJsonArray stepsArray = root.value("steps").toArray();

    for (const QJsonValue& stepVal : stepsArray)
    {
        const QJsonObject stepObj = stepVal.toObject();

        TutorialStep step;
        step.instructions = stepObj.value("instructions").toString();
        step.restrictInput = stepObj.value("restrictInput").toBool(false);
        step.toolboxIncludesPin = stepObj.value("toolboxIncludesPin").toBool(false);

        const QJsonValue highlightVal = stepObj.value("highlight");
        if (highlightVal.isArray())
        {
            for (const QJsonValue& h : highlightVal.toArray())
                step.highlightTargets << h.toString();
        }
        else if (highlightVal.isString())
        {
            step.highlightTargets << highlightVal.toString();
        }

        for (const QJsonValue& c : stepObj.value("toolbox").toArray())
            step.toolboxComponents << c.toString();

        for (const QJsonValue& condVal : stepObj.value("conditions").toArray())
            step.conditions.push_back(parseCondition(condVal.toObject()));

        step.truthTable = parseTruthTable(stepObj.value("truthTable").toObject());

        steps.push_back(step);
    }

    if (steps.empty())
    {
        qWarning() << "TutorialManager: tutorial" << tutorialTitle << "has no steps";
        return false;
    }

    return true;
}

TutorialCondition TutorialManager::parseCondition(const QJsonObject& obj) const
{
    TutorialCondition condition;

    const QString typeStr = obj.value("type").toString();

    if (typeStr == "placeComponent") condition.type = TutorialCondition::Type::PlaceComponent;
    else if (typeStr == "placePin") condition.type = TutorialCondition::Type::PlacePin;
    else if (typeStr == "placeWire") condition.type = TutorialCondition::Type::PlaceWire;
    else if (typeStr == "connected") condition.type = TutorialCondition::Type::Connected;
    else if (typeStr == "deleted") condition.type = TutorialCondition::Type::Deleted;
    else qWarning() << "TutorialManager: unknown condition type" << typeStr;

    condition.componentName = obj.value("component").toString();
    condition.count = obj.value("count").toInt(1);

    for (const QJsonValue& b : obj.value("bind").toArray())
        condition.bindNames << b.toString();

    condition.a = obj.value("a").toString();
    condition.b = obj.value("b").toString();
    condition.target = obj.value("target").toString();

    return condition;
}

TruthTableSpec TutorialManager::parseTruthTable(const QJsonObject& obj) const
{
    TruthTableSpec spec;

    if (obj.isEmpty())
        return spec;

    spec.enabled = true;

    auto parseColumns = [](const QJsonArray& array) {
        QVector<TruthTableColumnSpec> columns;

        for (const QJsonValue& v : array)
        {
            const QJsonObject o = v.toObject();

            TruthTableColumnSpec column;
            column.ref = o.value("ref").toString();
            column.label = o.value("label").toString();

            columns.push_back(column);
        }

        return columns;
    };

    spec.inputs = parseColumns(obj.value("inputs").toArray());
    spec.outputs = parseColumns(obj.value("outputs").toArray());

    for (const QJsonValue& rowVal : obj.value("expected").toArray())
    {
        QVector<QString> row;

        for (const QJsonValue& cellVal : rowVal.toArray())
            row.push_back(cellVal.toString());

        spec.expectedOutputRows.push_back(row);
    }

    return spec;
}

void TutorialManager::start()
{
    currentStepIndex = -1;
    variables.clear();

    for (TutorialStep& step : steps)
        for (TutorialCondition& condition : step.conditions)
        {
            condition.satisfied = false;
            condition.matchedCount = 0;
        }

    labelPollTimer->start();

    emit tutorialStarted();
    advanceStep();
}

void TutorialManager::cancel()
{
    currentStepIndex = -1;

    labelPollTimer->stop();

    if (overlay)
    {
        overlay->hideInstruction();
        overlay->setRestrictInputToHighlight(false);
    }

    if (toolbox)
        toolbox->clear();

    emit tutorialCancelled();
}

bool TutorialManager::isActive() const
{
    return currentStepIndex >= 0 && currentStepIndex < static_cast<int>(steps.size());
}

TutorialValue TutorialManager::variable(const QString& name) const
{
    auto it = variables.find(name);
    return it != variables.end() ? it.value() : TutorialValue();
}

TutorialValue TutorialManager::resolveReference(const QString& ref) const
{
    if (ref.startsWith('$'))
        return variable(ref.mid(1));

    return TutorialValue();
}

void TutorialManager::bindNext(TutorialCondition& condition, const TutorialValue& value)
{
    if (condition.matchedCount < condition.bindNames.size())
        variables[condition.bindNames[condition.matchedCount]] = value;

    condition.matchedCount++;

    if (condition.matchedCount >= condition.count)
        condition.satisfied = true;
}

void TutorialManager::onDismissRequested()
{
    
    
    
    
    if (isActive() && currentStepIndex == static_cast<int>(steps.size()) - 1)
        finishTutorial();
    else
        cancel();
}

void TutorialManager::finishTutorial()
{
    currentStepIndex = -1;

    labelPollTimer->stop();

    if (overlay)
    {
        overlay->hideInstruction();
        overlay->setRestrictInputToHighlight(false);
    }

    if (toolbox)
        toolbox->clear();

    emit tutorialCompleted();
}

void TutorialManager::updateVariableLabels()
{
    if (!overlay)
        return;

    QVector<QPair<QString, QPointF>> labels;

    for (auto it = variables.constBegin(); it != variables.constEnd(); ++it)
    {
        const TutorialValue& value = it.value();

        Position pos;
        bool havePos = false;

        switch (value.kind)
        {
            case TutorialValue::Kind::Component:
                if (value.component) { pos = value.component->getGridPosition(); havePos = true; }
                break;

            case TutorialValue::Kind::Pin:
                if (value.pin) { pos = value.pin->getGridPosition(); havePos = true; }
                break;

            case TutorialValue::Kind::Wire:
                if (value.wire && !value.wire->segments.empty()) { pos = value.wire->segments.front().begin; havePos = true; }
                break;

            default:
                break;
        }

        if (havePos)
            labels.append({it.key(), pos.getGridScaledCopy().getQPointF()});
    }

    overlay->setVariableLabels(&workspace, labels);
}

void TutorialManager::onComponentPlaced(Component* component)
{
    if (!isActive() || !component)
        return;

    TutorialStep& step = steps[static_cast<size_t>(currentStepIndex)];

    for (TutorialCondition& condition : step.conditions)
    {
        if (condition.satisfied || condition.type != TutorialCondition::Type::PlaceComponent)
            continue;

        if (!condition.componentName.isEmpty())
        {
            SentinelComponent* source = component->getSourceSentinel();
            if (!source || QString::fromStdString(source->getName()) != condition.componentName)
                continue;
        }

        bindNext(condition, TutorialValue::of(component));
    }

    reevaluateConnectivityConditions();
    updateVariableLabels();
    checkStepCompletion();
}

void TutorialManager::onPinPlaced(Pin* pin)
{
    if (!isActive() || !pin)
        return;

    TutorialStep& step = steps[static_cast<size_t>(currentStepIndex)];

    for (TutorialCondition& condition : step.conditions)
    {
        if (condition.satisfied || condition.type != TutorialCondition::Type::PlacePin)
            continue;

        bindNext(condition, TutorialValue::of(pin));
    }

    reevaluateConnectivityConditions();
    updateVariableLabels();
    checkStepCompletion();
}

void TutorialManager::onWirePlaced(Wire* wire)
{
    if (!isActive() || !wire)
        return;

    TutorialStep& step = steps[static_cast<size_t>(currentStepIndex)];

    for (TutorialCondition& condition : step.conditions)
    {
        if (condition.satisfied || condition.type != TutorialCondition::Type::PlaceWire)
            continue;

        bindNext(condition, TutorialValue::of(wire));
    }

    reevaluateConnectivityConditions();
    updateVariableLabels();
    checkStepCompletion();
}

void TutorialManager::onItemDeleted(AbstractPropagator* propagator)
{
    if (!isActive() || !propagator)
        return;

    TutorialStep& step = steps[static_cast<size_t>(currentStepIndex)];

    for (TutorialCondition& condition : step.conditions)
    {
        if (condition.satisfied || condition.type != TutorialCondition::Type::Deleted)
            continue;

        const TutorialValue target = resolveReference(condition.target);

        if (target.propagator() == propagator)
            condition.satisfied = true;
    }

    
    
    
    
    for (auto it = variables.begin(); it != variables.end(); ++it)
        if (it.value().propagator() == propagator)
            it.value() = TutorialValue();

    updateVariableLabels();
    checkStepCompletion();
}

std::vector<Propagator*> TutorialManager::collectPropagators(const TutorialValue& value) const
{
    std::vector<Propagator*> result;

    switch (value.kind)
    {
        case TutorialValue::Kind::Component:
            if (value.component)
                for (Pin* pin : value.component->getPins())
                    result.push_back(pin);
            break;

        case TutorialValue::Kind::Pin:
            if (value.pin)
                result.push_back(value.pin);
            break;

        case TutorialValue::Kind::Wire:
            if (value.wire)
                result.push_back(value.wire);
            break;

        default:
            break;
    }

    return result;
}

bool TutorialManager::arePropagatorsConnected(const TutorialValue& a, const TutorialValue& b) const
{
    const std::vector<Propagator*> starts = collectPropagators(a);

    std::unordered_set<Propagator*> targets;
    for (Propagator* p : collectPropagators(b))
        targets.insert(p);

    if (starts.empty() || targets.empty())
        return false;

    std::unordered_set<Propagator*> visited;
    std::vector<Propagator*> stack(starts.begin(), starts.end());

    while (!stack.empty())
    {
        Propagator* current = stack.back();
        stack.pop_back();

        if (!current || visited.count(current))
            continue;

        visited.insert(current);

        if (targets.count(current))
            return true;

        for (Propagator* next : current->getEffectors())
            stack.push_back(next);

        for (Propagator* next : current->getAffectors())
            stack.push_back(next);
    }

    return false;
}

void TutorialManager::reevaluateConnectivityConditions()
{
    if (!isActive())
        return;

    TutorialStep& step = steps[static_cast<size_t>(currentStepIndex)];

    for (TutorialCondition& condition : step.conditions)
    {
        if (condition.satisfied || condition.type != TutorialCondition::Type::Connected)
            continue;

        const TutorialValue a = resolveReference(condition.a);
        const TutorialValue b = resolveReference(condition.b);

        if (a.isValid() && b.isValid() && arePropagatorsConnected(a, b))
            condition.satisfied = true;
    }
}

void TutorialManager::checkStepCompletion()
{
    if (!isActive())
        return;

    const TutorialStep& step = steps[static_cast<size_t>(currentStepIndex)];

    for (const TutorialCondition& condition : step.conditions)
        if (!condition.satisfied)
            return;

    advanceStep();
}

void TutorialManager::advanceStep()
{
    currentStepIndex++;

    if (currentStepIndex >= static_cast<int>(steps.size()))
    {
        finishTutorial();
        return;
    }

    presentCurrentStep();
}

void TutorialManager::presentCurrentStep()
{
    const TutorialStep& step = steps[static_cast<size_t>(currentStepIndex)];

    if (overlay)
    {
        overlay->setRestrictInputToHighlight(step.restrictInput);

        QVector<QWidget*> widgets;

        for (const QString& target : step.highlightTargets)
        {
            if (target == "toolbox" && toolbox)
                widgets.append(toolbox);
            else if (target == "workspace")
                widgets.append(&workspace);
        }

        if (widgets.isEmpty())
            overlay->clearHighlight();
        else
            overlay->highlightWidgets(widgets);

        overlay->showInstruction(step.instructions);

        if (step.truthTable.enabled)
        {
            auto resolveColumns = [this](const QVector<TruthTableColumnSpec>& specs) {
                QVector<TruthTableColumn> columns;

                for (const TruthTableColumnSpec& colSpec : specs)
                {
                    const TutorialValue value = resolveReference(colSpec.ref);

                    TruthTableColumn column;
                    column.label = colSpec.label;
                    column.pin = value.kind == TutorialValue::Kind::Pin ? value.pin : nullptr;

                    columns.push_back(column);
                }

                return columns;
            };

            overlay->showTruthTable(
                resolveColumns(step.truthTable.inputs),
                resolveColumns(step.truthTable.outputs),
                step.truthTable.expectedOutputRows
            );
        }
        else
        {
            overlay->hideTruthTable();
        }
    }

    if (toolbox)
    {
        std::vector<SentinelComponent*> sentinels;

        for (const QString& name : step.toolboxComponents)
        {
            auto it = globalProjectManager->components.find(name.toStdString());
            if (it != globalProjectManager->components.end())
                sentinels.push_back(it->second.get());
            else
                qWarning() << "TutorialManager: step references unknown component" << name;
        }

        toolbox->setAvailableComponents(sentinels, step.toolboxIncludesPin);
    }

    emit stepChanged(currentStepIndex, step.instructions);

    updateVariableLabels();
}