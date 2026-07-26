#include "component.h"

#include "scheduler.h"
#include "filehelper.h"
#include <iostream>
#include <cstdint>

Component::Component() {}

void Propagator::evaluateEffectingState() {
    auto it = affectors.begin();
    States currentState = affectors.end() != it ? (*it)->effectingState : States::FLOATING;
    it++;
    while (it != affectors.end()) {
        currentState = evaluateTwoStates(currentState, (*it)->effectingState);
    }
    effectingState = currentState;
}

// void Propagator::evaluateEffectors() {

// }

void Propagator::propagate(Propagator* excludedPropagator) {
    evaluateEffectingState();
    globalScheduler.registerCallback(effectors, excludedPropagator);
}

void Propagator::forget(Propagator* propagator) {
    effectors.erase(propagator);
    affectors.erase(propagator);
}

void Pin::evaluateEffectingState() {
    switch (effectorOperation) {
        case Operations::OR:
            switch (affectors.size()) {
            case 0:
                effectingState = States::FLOATING;
                break;
            case 1:
                effectingState = (*(*affectors.begin())).effectingState;
                break;
            default:
                effectingState = States::FLOATING;
                for (const auto* affector : affectors) {
                    const States state = (States)(affector->effectingState | effectingState);
                    effectingState = state > 1 ? std::max(effectingState, affector->effectingState) : state;
                }
                break;
            }
            break;
        case Operations::AND:
            switch (affectors.size()) {
            case 0:
                effectingState = States::FLOATING;
                break;
            case 1:
                effectingState = (*(*affectors.begin())).effectingState;
                break;
            default:
                effectingState = States::FLOATING;
                for (const auto* affector : affectors) {
                    effectingState = (affector->effectingState | effectingState) > 1 ? std::max(effectingState, affector->effectingState) : (States)(affector->effectingState & effectingState);
                }
                break;
            }
            break;
        case Operations::XOR:
            switch (affectors.size()) {
            case 0:
                effectingState = States::FLOATING;
                break;
            case 1:
                effectingState = (*(*affectors.begin())).effectingState;
                break;
            default:
                effectingState = States::FLOATING;
                for (const auto* affector : affectors) {
                    effectingState = (affector->effectingState | effectingState) > 1 ? std::max(effectingState, affector->effectingState) : (States)(affector->effectingState ^ effectingState);
                }
                break;
            }
            break;
        case Operations::NOT:
            switch (affectors.size()) {
            case 0:
                effectingState = States::FLOATING;
                break;
            case 1:
                effectingState = (*(*affectors.begin())).effectingState;
                break;
            default:
                effectingState = States::FLOATING;
                for (const auto* affector : affectors) {
                    effectingState = (affector->effectingState != effectingState) ? States::CONFLICT : effectingState == States::FLOATING ? States::FLOATING : (States)(!affector->effectingState);
                }
                break;
            }
            break;
        case Operations::NOR:
            switch (affectors.size()) {
            case 0:
                effectingState = States::FLOATING;
                break;
            case 1:
                effectingState = (*(*affectors.begin())).effectingState;
                break;
            default:
                effectingState = States::FLOATING;
                for (const auto* affector : affectors) {
                    const States state = (States)(affector->effectingState | effectingState);
                    effectingState = state > 1 ? std::max(effectingState, affector->effectingState) : (States)((~state)&1);
                }
                break;
            }
            break;
        case Operations::NAND:
            switch (affectors.size()) {
            case 0:
                effectingState = States::FLOATING;
                break;
            case 1:
                effectingState = (*(*affectors.begin())).effectingState;
                break;
            default:
                effectingState = States::FLOATING;
                for (const auto* affector : affectors) {
                    effectingState = (affector->effectingState | effectingState) > 1 ? std::max(effectingState, affector->effectingState) : (States)((~(affector->effectingState & effectingState))&1);
                }
                break;
            }
            break;
        case Operations::XNOR:
            switch (affectors.size()) {
            case 0:
                effectingState = States::FLOATING;
                break;
            case 1:
                effectingState = (*(*affectors.begin())).effectingState;
                break;
            default:
                effectingState = States::FLOATING;
                for (const auto* affector : affectors) {
                    effectingState = (affector->effectingState | effectingState) > 1 ? std::max(effectingState, affector->effectingState) : (States)(~(affector->effectingState ^ effectingState)&1);
                }
                break;
            }
            break;
        case Operations::BUFFER:
        {
            auto it = affectors.begin();
            States currentState = affectors.end() != it ? (*it)->effectingState : States::FLOATING;
            it++;
            while (it != affectors.end()) {
                currentState = evaluateTwoStates(currentState, (*it)->effectingState);
            }
            effectingState = currentState;
            break;
        }
    }
}

States Propagator::evaluateTwoStates(const States& stateA, const States& stateB) {
    if (stateA == stateB) return stateA;
    switch (stateA) {
    case States::FLOATING:
        return stateB;
    case States::LOW:
        switch (stateB) {
        case States::FLOATING:
            return stateA;
        case States::LOW:
            std::cout << "WARNING DOUBLE SAME COMPONENT.CPP" << std::endl;
            return stateB;
        case States::HIGH:
            return States::CONFLICT;
        case States::CONFLICT:
            return States::CONFLICT;
        case States::ERROR:
            return States::ERROR;
        }
    case States::HIGH:
        switch (stateB) {
        case States::FLOATING:
            return stateA;
        case States::LOW:
            return States::CONFLICT;
        case States::HIGH:
            std::cout << "WARNING DOUBLE SAME COMPONENT.CPP" << std::endl;
            return stateB;
        case States::CONFLICT:
            return States::CONFLICT;
        case States::ERROR:
            return States::ERROR;
        }
    case States::CONFLICT:
        return States::CONFLICT;
    case States::ERROR:
        return States::ERROR;
    }
}

void Component::saveToFile(const std::filesystem::path& path) {
    //saving to file...

    //file layout:
    /*
    0x0000 component save version major
    0x0004 component save version minor
    0x.... name length (4b)
    0x.... name string
    0x0008: propagator count (.size) (4b)
    0x000c: number of wires (4b)
    0x0010: first wire of wires:
        wire layout:
        0x0000: number of anchors (4b)
        0x0004: first anchor pos of anchors
            anchor layout:
            0x0000: int x (4b)
            0x0004: int y (4b)
        0x....: number of effectors (4b)
        0x....: number of affectors (4b)
        0x....: first effector id (4b)
        0x....: first affector id (4b)
    0x....: first non-wire of non-wires (pins...?)
        pin layout:
        0x0000: effector Operation (4b)
        0x0004: relPosition x (4b)
        0x0008: relPosition y (4b)
        0x000c: number of effectors (4b)
        0x0010: number of affectors (4b)
        0x0014: first effector id (4b)
        0x.... first affector id (4b)

    */

    std::vector<uint32_t> buffer;
    buffer.push_back(COMPONENT_SAVE_VERSION_MAJOR);
    buffer.push_back(COMPONENT_SAVE_VERSION_MINOR);
    buffer.push_back(name.length());
    uint32_t namei = 0;
    uint32_t nameBuffer = 0;
    for (unsigned char c : name) {
      uint32_t nc = c << namei*8;
      nameBuffer |= nc;
      if (namei == 3) {
        buffer.push_back(nameBuffer);
        nameBuffer = 0;
        namei = 0;
      } else {
        namei++;
      }
    }
    if (namei == 0 && name.length() > 0) {
      buffer.push_back(nameBuffer);
      nameBuffer = 0;
      namei = 0;
    }
    buffer.push_back(propagators.size());
    uint32_t propagatorId = 0;
    std::unordered_map<Propagator*, uint32_t> wiresById;
    uint32_t wiresUint32Size = 0;
    std::unordered_map<Propagator*, uint32_t> pinsById;
    uint32_t pinsUint32Size = 0;

    for (auto& propagator : propagators) {
        if (propagator->getKind() == Propagator::Kinds::PIN) {
            pinsById.emplace(propagator.get(), propagatorId++);
            pinsUint32Size += propagator->getUint32sToSave();
        } else if (propagator->getKind() == Propagator::Kinds::WIRE) {
            wiresById.emplace(propagator.get(), propagatorId++);
            wiresUint32Size += propagator->getUint32sToSave();
        } else {
            std::cout << "unknown propagator type component.cpp" << std::endl;
        }
    }

    std::unordered_map<Propagator*, uint32_t> propagatorsById;
    propagatorsById.merge(wiresById);
    propagatorsById.merge(pinsById);

    buffer.push_back(wiresById.size());
    auto propagatorIndex = buffer.size();
    buffer.resize(buffer.size() + wiresUint32Size + pinsUint32Size);

    for (const auto& [wire, id] : wiresById) {
        wire->saveToAddress(buffer.data() + propagatorIndex, propagatorsById);
        propagatorIndex += wire->getUint32sToSave();
    }
    for (const auto& [pin, id] : pinsById) {
        pin->saveToAddress(buffer.data() + propagatorIndex, propagatorsById);
        propagatorIndex += pin->getUint32sToSave();
    }

    writeUint32VectorToFile(path, buffer);
}

void Component::loadFromFile(const std::filesystem::path& path) {
    std::vector<uint32_t> buffer = openFileToUint32Vector(path);
    
    uint32_t* bufferPos = buffer.data() + 2;

    const uint32_t nameLength = *bufferPos++;
    for (uint32_t i = 0; i < nameLength; i++) {
      const unsigned char c = (*bufferPos >> ((i % 4)*8)) & 0xff;
      name.push_back(c);
      if (i % 4 == 3) {
        bufferPos++;
      }
    }
    if (nameLength % 4 != 0) bufferPos++;

    const uint32_t totalPropagators = *bufferPos++;
    const uint32_t totalWires = *bufferPos++;
    const uint32_t totalPins = totalPropagators - totalWires;

    std::unordered_map<uint32_t, PropagatorIdentity> propagators;


    for (uint32_t i = 0; i < totalWires; i++) {

        auto wire = std::make_unique<Wire>();

        uint32_t anchorCount = *bufferPos++;
        for (uint32_t j = 0; j < anchorCount; j++) {
            wire->anchors.emplace_back();
            wire->anchors.back().x = *bufferPos++;
            wire->anchors.back().y = *bufferPos++;
        }
        uint32_t effectorCount = *bufferPos++;
        uint32_t affectorCount = *bufferPos++;

        auto [kvpair, success] = propagators.emplace(propagators.size(), wire.get());
        for (uint32_t j = 0; j < effectorCount; j++) {
            kvpair->second.effectorIds.emplace(*bufferPos++);
        }
        for (uint32_t j = 0; j < affectorCount; j++) {
            kvpair->second.affectorIds.emplace(*bufferPos++);
        }

        this->propagators.push_back(std::move(wire));
    }

    for (uint32_t i = 0; i < totalPins; i++) {

        auto pin = std::make_unique<Pin>(*this);

        pin->effectorOperation =(Pin::Operations)*bufferPos++;
        pin->relPosition.x = *bufferPos++;
        pin->relPosition.y = *bufferPos++;

        uint32_t effectorCount = *bufferPos++;
        uint32_t affectorCount = *bufferPos++;

        auto [kvpair, success] = propagators.emplace(propagators.size(), pin.get());
        for (uint32_t j = 0; j < effectorCount; j++) {
            kvpair->second.effectorIds.emplace(*bufferPos++);
        }
        for (uint32_t j = 0; j < affectorCount; j++) {
            kvpair->second.affectorIds.emplace(*bufferPos++);
        }

        this->propagators.push_back(std::move(pin));
    }

    for (const auto& [id, identity] : propagators) {
        for (const auto pid : identity.effectorIds) {
            Propagator* ptr = findPropagatorPointerOfId(pid, propagators);
            if (!ptr) continue;
            identity.propagator->effectors.emplace(ptr);
        }
        for (const auto pid : identity.affectorIds) {
            Propagator* ptr = findPropagatorPointerOfId(pid, propagators);
            if (!ptr) continue;
            identity.propagator->affectors.emplace(ptr);
        }
    }


}

Propagator* Component::findPropagatorPointerOfId(uint32_t id, const std::unordered_map<uint32_t, PropagatorIdentity>& map) {
    auto it = map.find(id);
    if (it == map.end()) {
        std::cout << "did not find uint32 id in map." << std::endl;
        return nullptr;
    }
    return it->second.propagator;
}

uint32_t Propagator::findIdOfPropagatorPointer(Propagator* propagator, const std::unordered_map<Propagator*, uint32_t>& map) {
    auto it = map.find(propagator);
    if (it == map.end()) {
        std::cout << "could not find propagator id." << std::endl;
        return UINT32_MAX;
    }
    return it->second;
}

void Propagator::saveEffectorsAndAffectorsToAddres(uint32_t* data, const std::unordered_map<Propagator*, uint32_t>& map) const {
    *data = effectors.size();
    *(data+1) = affectors.size();

    uint32_t dataOffset = 2;
    for (auto* effector : effectors) {
        *(data+dataOffset++) = findIdOfPropagatorPointer(effector, map);
    }
    for (auto* affector : affectors) {
        *(data+dataOffset++) = findIdOfPropagatorPointer(affector, map);
    }
}

void Pin::saveToAddress(uint32_t* data, const std::unordered_map<Propagator*, uint32_t>& map) const {
    *data++ = effectorOperation;
    *(data++) = relPosition.x;
    *(data++) = relPosition.x;
    saveEffectorsAndAffectorsToAddres(data, map);
}

uint32_t Pin::getUint32sToSave() const {
    uint32_t count = 3;
    count += getUint32sToSaveEffectorsAndAffectors();
    return count;
}

uint32_t Propagator::getUint32sToSaveEffectorsAndAffectors() const {
    uint32_t count = 2; //effector affector count
    count += effectors.size() + affectors.size();
    return count;
}

uint32_t Wire::getUint32sToSave() const {
    uint32_t count = 1;
    count += anchors.size()*2;
    count += getUint32sToSaveEffectorsAndAffectors();
    return count;
}

void Wire::saveToAddress(uint32_t* data, const std::unordered_map<Propagator*, uint32_t>& map) const {
    *data++ = anchors.size();
    for (uint32_t i = 0; i < anchors.size(); i++) {
        *(data) = anchors[i].x;
        *(data+1) = anchors[i].y;
        data += 2;
    }
    saveEffectorsAndAffectorsToAddres(data, map);
}

Wire::Wire(const Wire& wireToCopy) : anchors(wireToCopy.anchors), Propagator(wireToCopy) {}

Component::Component(const Component& componentToCopy) 
    :   name(componentToCopy.name) {
    std::unordered_map<Propagator*, Propagator*> oldNewPropagatorPointerMap;
    std::unordered_map<Pin*, Propagator*> pins;
    std::unordered_map<Wire*, Propagator*> wires;
    for (const auto& propagator : componentToCopy.propagators) {
        switch (propagator->getKind()) {
        case Propagator::Kinds::PIN:{
            auto ptr = std::make_unique<Pin>(*(Pin*)propagator.get(), *this);
            auto [element, success] = pins.emplace(ptr.get(), propagator.get());
            oldNewPropagatorPointerMap.emplace(element->second, element->first);
            propagators.push_back(std::move(ptr));
            break;
        }
        case Propagator::Kinds::WIRE:{
            auto ptr = std::make_unique<Wire>(*(Wire*)propagator.get());
            auto [element, success] = wires.emplace(ptr.get(), propagator.get());
            oldNewPropagatorPointerMap.emplace(element->second, element->first);
            propagators.push_back(std::move(ptr));
            break;
        }
        }
    }

    for (const auto [pin, propagator] : pins) {
        pin->copyEffectorsAndEffectors(*propagator, oldNewPropagatorPointerMap);
    }

    for (const auto [wire, propagator] : wires) {
        wire->copyEffectorsAndEffectors(*propagator, oldNewPropagatorPointerMap);
    }


}

void Propagator::copyEffectorsAndEffectors(const Propagator& propagator, std::unordered_map<Propagator*, Propagator*>& oldNewPropagatorPointerMap) {
    for (auto* effector : propagator.effectors) {
        auto it = oldNewPropagatorPointerMap.find(effector);
        if (it != oldNewPropagatorPointerMap.end()) {
            effectors.emplace(it->second);
        }
    }
    for (auto* affector : propagator.affectors) {
        auto it = oldNewPropagatorPointerMap.find(affector);
        if (it != oldNewPropagatorPointerMap.end()) {
            affectors.emplace(it->second);
        }
    }
}

Pin::Pin(const Pin& pinToCopy, Component& _parent)
    : parent(_parent),
    effectorOperation(pinToCopy.effectorOperation),
    relPosition(pinToCopy.relPosition),
    Propagator(pinToCopy)
    {

}

    Propagator::Propagator(const Propagator& propagator)
    : tickPropagationDelay(propagator.tickPropagationDelay),
    effectingState(propagator.effectingState)
    {}


#include <QDebug>

void Component::debugPrintPropagators() const
{
    qDebug() << "========== Component Debug ==========";
    qDebug() << "Name:" << QString::fromStdString(name);
    qDebug() << "Propagator count:" << propagators.size();

    for (size_t i = 0; i < propagators.size(); ++i)
    {
        const Propagator* p = propagators[i].get();

        qDebug() << "";
        qDebug() << "Propagator" << i;
        qDebug() << "--------------------------------";
        qDebug() << "Address:" << p;

        // Type
        switch (p->getKind())
        {
        case Propagator::WIRE:
            qDebug() << "Kind: Wire";
            break;
        case Propagator::PIN:
            qDebug() << "Kind: Pin";
            break;
        }

        // Common properties
        qDebug() << "Tick propagation delay:" << p->tickPropagationDelay;
        qDebug() << "Effecting state:" << static_cast<int>(p->effectingState);
        qDebug() << "Effectors:" << p->effectors.size();
        qDebug() << "Affectors:" << p->affectors.size();

        qDebug() << "Effector pointers:";
        for (Propagator* effector : p->effectors)
            qDebug() << "   " << effector;

        qDebug() << "Affector pointers:";
        for (Propagator* affector : p->affectors)
            qDebug() << "   " << affector;

        // Type-specific data
        if (p->getKind() == Propagator::WIRE)
        {
            const Wire* wire = static_cast<const Wire*>(p);

            qDebug() << "Anchor count:" << wire->anchors.size();

            for (size_t j = 0; j < wire->anchors.size(); ++j)
            {
                qDebug() << "   Anchor" << j
                         << "("
                         << wire->anchors[j].x
                         << ","
                         << wire->anchors[j].y
                         << ")";
            }
        }
        else if (p->getKind() == Propagator::PIN)
        {
            const Pin* pin = static_cast<const Pin*>(p);

            qDebug() << "Operation:" << static_cast<int>(pin->effectorOperation);
            qDebug() << "Relative position:"
                     << "("
                     << pin->relPosition.x
                     << ","
                     << pin->relPosition.y
                     << ")";

            // Position gp = pin->gridPosition();

            // qDebug() << "Grid position:"
            //          << "("
            //          << gp.x
            //          << ","
            //          << gp.y
            //          << ")";

            qDebug() << "State pointer:" << pin->state;
            if (pin->state)
                qDebug() << "State value:" << static_cast<int>(*pin->state);

            qDebug() << "Parent component:"
                     << QString::fromStdString(pin->parent.name);
        }
    }

    qDebug() << "=====================================";
}