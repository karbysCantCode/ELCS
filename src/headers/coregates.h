#ifndef COREGATES_H
#define COREGATES_H

#include "component.h"
#include "componentappearance.h"

namespace CoreGates {
    Pin* addPin(
        SentinelComponent& component,
        const std::string& name,
        const Position& appearancePosition,
        Pin::Operations operation = Pin::Operations::BUFFER
    );

    void connect(Pin& source, Pin& destination);
    SentinelComponent makeOR();
    SentinelComponent makeAND();
    SentinelComponent makeXOR();
    SentinelComponent makeNAND();
    SentinelComponent makeNOR();
    SentinelComponent makeXNOR();
    SentinelComponent makeNOT();
    SentinelComponent makeBUFFER();
}

#endif