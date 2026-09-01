#ifndef TUTORIALTYPES_H
#define TUTORIALTYPES_H

#include <QString>
#include <QStringList>
#include <QVector>

#include <vector>

#include "component.h"

struct TutorialValue
{
    enum class Kind { None, Component, Pin, Wire };

    Kind kind = Kind::None;
    Component* component = nullptr;
    Pin* pin = nullptr;
    Wire* wire = nullptr;

    static TutorialValue of(Component* c) { TutorialValue v; v.kind = Kind::Component; v.component = c; return v; }
    static TutorialValue of(Pin* p) { TutorialValue v; v.kind = Kind::Pin; v.pin = p; return v; }
    static TutorialValue of(Wire* w) { TutorialValue v; v.kind = Kind::Wire; v.wire = w; return v; }

    AbstractPropagator* propagator() const
    {
        switch (kind)
        {
            case Kind::Component: return component;
            case Kind::Pin: return pin;
            case Kind::Wire: return wire;
            default: return nullptr;
        }
    }

    bool isValid() const { return kind != Kind::None; }
};


struct TutorialCondition
{
    enum class Type
    {
        PlaceComponent,
        PlacePin,
        PlaceWire,
        Connected,
        Deleted,
        Unknown
    };

    Type type = Type::Unknown;

    QString componentName;   
    int count = 1;            
    QStringList bindNames;    

    
    QString a;
    QString b;

    
    QString target;

    int matchedCount = 0;
    bool satisfied = false;
};


struct TruthTableColumnSpec
{
    QString ref;
    QString label;
};

struct TruthTableSpec
{
    bool enabled = false;

    QVector<TruthTableColumnSpec> inputs;
    QVector<TruthTableColumnSpec> outputs;

    QVector<QVector<QString>> expectedOutputRows;
};


struct TutorialStep
{
    QString instructions;

    QStringList highlightTargets;

    bool restrictInput = false;

    QStringList toolboxComponents;

    bool toolboxIncludesPin = false;

    std::vector<TutorialCondition> conditions;

    TruthTableSpec truthTable;
};

#endif