#include "component.h"
#include "componentappearance.h"

#include <memory>
#include <string>


namespace CoreGates
{
    // ------------------------------------------------------------
    // Helper
    // ------------------------------------------------------------

    Pin* addPin(
        SentinelComponent& component,
        const std::string& name,
        const Position& appearancePosition,
        Pin::Operations operation = Pin::Operations::BUFFER
    )
    {
        auto pin = std::make_unique<Pin>(component);

        pin->setName(name);
        pin->setAppearancePosition(appearancePosition);
        pin->setEffectorOperation(operation);

        Pin* result = pin.get();

        component.addPropagator(std::move(pin));

        return result;
    }


    void connect(Pin& source, Pin& destination)
    {
        source.addEffector(&destination);
        destination.addAffector(&source);
    }

SentinelComponent makeAND()
{
    SentinelComponent component("AND");

    Pin* A = addPin(
        component,
        "A",
        Position(-2, -1)
    );

    Pin* B = addPin(
        component,
        "B",
        Position(-2, 1)
    );

    Pin* OUT = addPin(
        component,
        "OUT",
        Position(2, 0),
        Pin::Operations::AND
    );

    connect(*A, *OUT);
    connect(*B, *OUT);


    // Left side
    component.getAppearance().addLine(
        Position(-2, -2),
        Position(0, -2)
    );

    component.getAppearance().addLine(
        Position(-2, 2),
        Position(0, 2)
    );

    component.getAppearance().addLine(
        Position(0, -2),
        Position(0, 2)
    );

    // Right side
    component.getAppearance().addCurve(
        Position(0, -2),
        Position(3, -2),
        Position(3, 2),
        Position(0, 2)
    );

    component.getAppearance().addLine(
        Position(0, 0),
        Position(2, 0)
    );

    component.getAppearance().addLabel(
        "AND",
        Position(-1, 0)
    );

    return component;
}


// ============================================================
// NAND
// ============================================================

SentinelComponent makeNAND()
{
    SentinelComponent component("NAND");

    Pin* A = addPin(
        component,
        "A",
        Position(-2, -1)
    );

    Pin* B = addPin(
        component,
        "B",
        Position(-2, 1)
    );

    Pin* OUT = addPin(
        component,
        "OUT",
        Position(2, 0),
        Pin::Operations::NAND
    );

    connect(*A, *OUT);
    connect(*B, *OUT);


    // AND body
    component.getAppearance().addLine(
        Position(-2, -2),
        Position(0, -2)
    );

    component.getAppearance().addLine(
        Position(-2, 2),
        Position(0, 2)
    );

    component.getAppearance().addLine(
        Position(0, -2),
        Position(0, 2)
    );

    component.getAppearance().addCurve(
        Position(0, -2),
        Position(3, -2),
        Position(3, 2),
        Position(0, 2)
    );


    // Inversion bubble
    component.getAppearance().addCurve(
        Position(2, 0),
        Position(2, -1),
        Position(3, -1),
        Position(3, 0)
    );

    component.getAppearance().addCurve(
        Position(2, 0),
        Position(2, 1),
        Position(3, 1),
        Position(3, 0)
    );


    component.getAppearance().addLabel(
        "NAND",
        Position(-1, 0)
    );

    return component;
}


// ============================================================
// OR
// ============================================================

SentinelComponent makeOR()
{
    SentinelComponent component("OR");

    Pin* A = addPin(
        component,
        "A",
        Position(-2, -1)
    );

    Pin* B = addPin(
        component,
        "B",
        Position(-2, 1)
    );

    Pin* OUT = addPin(
        component,
        "OUT",
        Position(2, 0),
        Pin::Operations::OR
    );

    connect(*A, *OUT);
    connect(*B, *OUT);


    // Main OR curves
    component.getAppearance().addCurve(
        Position(-3, -2),
        Position(-1, -1),
        Position(0, 1),
        Position(-3, 2)
    );

    component.getAppearance().addCurve(
        Position(-3, 2),
        Position(0, 2),
        Position(1, 1),
        Position(2, 0)
    );

    component.getAppearance().addCurve(
        Position(-3, -2),
        Position(-1, -2),
        Position(1, -1),
        Position(2, 0)
    );


    component.getAppearance().addLabel(
        "OR",
        Position(-1, 0)
    );

    return component;
}


// ============================================================
// NOR
// ============================================================

SentinelComponent makeNOR()
{
    SentinelComponent component("NOR");

    Pin* A = addPin(
        component,
        "A",
        Position(-2, -1)
    );

    Pin* B = addPin(
        component,
        "B",
        Position(-2, 1)
    );

    Pin* OUT = addPin(
        component,
        "OUT",
        Position(2, 0),
        Pin::Operations::NOR
    );

    connect(*A, *OUT);
    connect(*B, *OUT);


    // OR body -- exactly following supplied design
    component.getAppearance().addCurve(
        Position(-3, -2),
        Position(-1, -1),
        Position(0, 1),
        Position(-3, 2)
    );

    component.getAppearance().addCurve(
        Position(-3, 2),
        Position(0, 2),
        Position(1, 1),
        Position(2, 0)
    );

    component.getAppearance().addCurve(
        Position(-3, -2),
        Position(-1, -2),
        Position(1, -1),
        Position(2, 0)
    );


    // Output inversion bubble
    component.getAppearance().addCurve(
        Position(2, 0),
        Position(2, -1),
        Position(3, -1),
        Position(3, 0)
    );

    component.getAppearance().addCurve(
        Position(2, 0),
        Position(2, 1),
        Position(3, 1),
        Position(3, 0)
    );


    component.getAppearance().addLabel(
        "NOR",
        Position(-1, 0)
    );

    return component;
}


// ============================================================
// XOR
// ============================================================

SentinelComponent makeXOR()
{
    SentinelComponent component("XOR");

    Pin* A = addPin(
        component,
        "A",
        Position(-2, 1)
    );

    Pin* B = addPin(
        component,
        "B",
        Position(-2, -1)
    );

    Pin* OUT = addPin(
        component,
        "OUT",
        Position(2, 0),
        Pin::Operations::XOR
    );

    connect(*A, *OUT);
    connect(*B, *OUT);


    // Extra XOR curve
    component.getAppearance().addCurve(
        Position(-3, 2),
        Position(-1, 1),
        Position(0, -1),
        Position(-3, -2)
    );

    // Main upper curve
    component.getAppearance().addCurve(
        Position(-2, 2),
        Position(0, 1),
        Position(1, 1),
        Position(2, 0)
    );

    // Main lower curve
    component.getAppearance().addCurve(
        Position(-2, -2),
        Position(0, -2),
        Position(1, -1),
        Position(2, 0)
    );


    component.getAppearance().addLabel(
        "XOR",
        Position(-1, 0)
    );

    return component;
}


// ============================================================
// XNOR
// ============================================================

SentinelComponent makeXNOR()
{
    SentinelComponent component("XNOR");

    Pin* A = addPin(
        component,
        "A",
        Position(-2, 1)
    );

    Pin* B = addPin(
        component,
        "B",
        Position(-2, -1)
    );

    Pin* OUT = addPin(
        component,
        "OUT",
        Position(2, 0),
        Pin::Operations::XNOR
    );

    connect(*A, *OUT);
    connect(*B, *OUT);


    // Extra XOR curve
    component.getAppearance().addCurve(
        Position(-3, 2),
        Position(-1, 1),
        Position(0, -1),
        Position(-3, -2)
    );

    // Main upper curve
    component.getAppearance().addCurve(
        Position(-2, 2),
        Position(0, 1),
        Position(1, 1),
        Position(2, 0)
    );

    // Main lower curve
    component.getAppearance().addCurve(
        Position(-2, -2),
        Position(0, -2),
        Position(1, -1),
        Position(2, 0)
    );


    // Output inversion bubble
    component.getAppearance().addCurve(
        Position(2, 0),
        Position(2, -1),
        Position(3, -1),
        Position(3, 0)
    );

    component.getAppearance().addCurve(
        Position(2, 0),
        Position(2, 1),
        Position(3, 1),
        Position(3, 0)
    );


    component.getAppearance().addLabel(
        "XNOR",
        Position(-1, 0)
    );

    return component;
}


// ============================================================
// NOT
// ============================================================

SentinelComponent makeNOT()
{
    SentinelComponent component("NOT");

    Pin* IN = addPin(
        component,
        "IN",
        Position(-2, 0)
    );

    Pin* OUT = addPin(
        component,
        "OUT",
        Position(2, 0),
        Pin::Operations::NOT
    );

    connect(*IN, *OUT);


    // Triangle
    component.getAppearance().addLine(
        Position(-2, -2),
        Position(-2, 2)
    );

    component.getAppearance().addLine(
        Position(-2, 2),
        Position(2, 0)
    );

    component.getAppearance().addLine(
        Position(2, 0),
        Position(-2, -2)
    );


    // Supplied design's extra line
    component.getAppearance().addLine(
        Position(0, -6),
        Position(2, -6)
    );


    // Output inversion bubble
    component.getAppearance().addCurve(
        Position(2, 0),
        Position(2, -1),
        Position(3, -1),
        Position(3, 0)
    );

    component.getAppearance().addCurve(
        Position(2, 0),
        Position(2, 1),
        Position(3, 1),
        Position(3, 0)
    );


    component.getAppearance().addLabel(
        "NOT",
        Position(-1, 0)
    );

    return component;
}


// ============================================================
// BUFFER
// ============================================================

SentinelComponent makeBUFFER()
{
    SentinelComponent component("BUFFER");

    Pin* IN = addPin(
        component,
        "IN",
        Position(-2, 0)
    );

    Pin* OUT = addPin(
        component,
        "OUT",
        Position(2, 0),
        Pin::Operations::BUFFER
    );

    connect(*IN, *OUT);


    // Standard buffer triangle
    component.getAppearance().addLine(
        Position(-2, -2),
        Position(-2, 2)
    );

    component.getAppearance().addLine(
        Position(-2, 2),
        Position(2, 0)
    );

    component.getAppearance().addLine(
        Position(2, 0),
        Position(-2, -2)
    );


    component.getAppearance().addLabel(
        "BUFFER",
        Position(-1, 0)
    );

    return component;
}

}
