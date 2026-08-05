#include "component.h"

#include "scheduler.h"
#include "filehelper.h"
#include <iostream>
#include <cstdint>

#include "notifications.h"
#include "projectmanager.h"

Component::Component() {}

struct SegmentHash
{
    std::size_t operator()(const Segment& segment) const
    {
        std::size_t h1 = std::hash<int>{}(segment.begin.x) ^
                         (std::hash<int>{}(segment.begin.y) << 1);

        std::size_t h2 = std::hash<int>{}(segment.end.x) ^
                         (std::hash<int>{}(segment.end.y) << 1);

        return h1 ^ (h2 << 1);
    }
};

//if attending to destroy, ensure destroy is correct in mergeCollidingWires.
Wire::~Wire() {
  qDebug(std::format("destroyingguhhhh 0x{:x}",(unsigned long long)graphicsItem).c_str());
  if (graphicsItem) {
    qDebug("graphicsItem");
    globalProjectManager->workspace->scene()->removeItem(graphicsItem);
    delete graphicsItem;
  }
}
// bool Position::doesPositionSitBetweenManhattenLines(bool targetLineHorizontal, const Position& positionA, const Position& positionB) const {
// 	if (targetLineHorizontal) {
// 		if (y != positionA.y) return false;
// 		return x <= std::max(positionA.x,positionB.x) && x >= std::min(positionA.x,positionB.x);
// 	} else {
// 		if (x != positionA.x) return false;
// 		return y <= std::max(positionA.y,positionB.y) && y >= std::min(positionA.y,positionB.y);
// 	}
// }

std::pair<bool,bool> Segment::doesSegmentEndsSitAlongSegment(
    bool targetLineHorizontal,
    const Segment& segment
) const
{
    bool beginHit = false;
    bool endHit = false;

    if (targetLineHorizontal)
    {
			int xmin = std::min(segment.begin.x, segment.end.x);
			int xmax = std::max(segment.begin.x, segment.end.x);

			beginHit = begin.y == segment.begin.y &&
									begin.x >= xmin &&
									begin.x <= xmax;

			endHit = end.y == segment.begin.y &&
								end.x >= xmin &&
								end.x <= xmax;
    }
    else
    {
			int ymin = std::min(segment.begin.y, segment.end.y);
			int ymax = std::max(segment.begin.y, segment.end.y);

			beginHit = begin.x == segment.begin.x &&
									begin.y >= ymin &&
									begin.y <= ymax;

			endHit = end.x == segment.begin.x &&
								end.y >= ymin &&
								end.y <= ymax;
    }

    return {beginHit, endHit};
}
// struct CollidingSegmentData {
// 	Wire* merger;
//   CollidingSegmentData(Wire* _merger) : merger(_merger) {}
// };

//return true if its destruction is expected
std::pair<bool, std::unordered_set<Segment, SegmentHash>> Wire::trimCollidingAgainstWire(Wire* other) {
  bool merges = false;
  std::vector<int> segmentsToRemove;
  for (int i = 0; i < segments.size(); i++) {
    auto& segment  = segments[i];
    bool thisIsHorizontal = segment.begin.y == segment.end.y;

    for (int j = 0; j < other->segments.size(); j++) {
      auto& otherSegment = other->segments[j];
      bool otherIsHorizontal = otherSegment.begin.y == otherSegment.end.y;

      if (thisIsHorizontal != otherIsHorizontal) {
        //detect ends along line
        const auto [beginCollide, endCollide] = segment.doesSegmentEndsSitAlongSegment(otherIsHorizontal, otherSegment);
				const auto [otherBeginCollide, otherEndCollide] = otherSegment.doesSegmentEndsSitAlongSegment(thisIsHorizontal, segment);

        if (beginCollide || endCollide || otherBeginCollide || otherEndCollide)
          merges = true;
      } else {
        if (thisIsHorizontal) {
          if (otherSegment.begin.y != segment.begin.y) continue;
        } else {
          if (otherSegment.begin.x != segment.begin.x) continue;
        }
        //check partial/full overlap

        int otherMax = 0;
        int thisMax = 0;

        int otherMin = 0;
        int thisMin = 0;

        bool thisMinBegin = false;
        bool thisMaxBegin = false;

        bool otherMinBegin = false;
        bool otherMaxBegin = false;

        if (thisIsHorizontal) {
          otherMax = std::max(otherSegment.begin.x,otherSegment.end.x);
          thisMax = std::max(segment.begin.x,segment.end.x);
          thisMaxBegin = thisMax == segment.begin.x;
          otherMaxBegin = otherMax == otherSegment.begin.x;

          otherMin = std::min(otherSegment.begin.x,otherSegment.end.x);
          thisMin = std::min(segment.begin.x,segment.end.x);
          thisMinBegin = thisMin == segment.begin.x;
          otherMinBegin = otherMin == otherSegment.begin.x;
        } else {
          otherMax = std::max(otherSegment.begin.y,otherSegment.end.y);
          thisMax = std::max(segment.begin.y,segment.end.y);
          thisMaxBegin = thisMax == segment.begin.y;
          otherMaxBegin = otherMax == otherSegment.begin.y;

          otherMin = std::min(otherSegment.begin.y,otherSegment.end.y);
          thisMin = std::min(segment.begin.y,segment.end.y);
          thisMinBegin = thisMin == segment.begin.y;
          otherMinBegin = otherMin == otherSegment.begin.y;
        }

        bool thisMaxInsideOther = thisMax <= otherMax;
        bool thisMinInsideOther = thisMin >= otherMin;

        if (thisMin >= otherMin && thisMax <= otherMax) {
          //full

          merges = true;
          segmentsToRemove.push_back(i);
          globalProjectManager->gridManager.addToGrid(otherSegment, this);
					globalProjectManager->gridManager.addToGrid(otherSegment, other);
          break;
        } else if (thisMin <= otherMax && thisMax >= otherMin) {
          //partial
          int newMin = std::min(thisMin, otherMin);
          int newMax = std::max(thisMax, otherMax);
          if (thisIsHorizontal)
          {
              otherSegment.begin.x = newMin;
              otherSegment.end.x = newMax;
          }
          else
          {
              otherSegment.begin.y = newMin;
              otherSegment.end.y = newMax;
          }

          merges = true;
          segmentsToRemove.push_back(i);
          globalProjectManager->gridManager.addToGrid(otherSegment, this);
					globalProjectManager->gridManager.addToGrid(otherSegment, other);
          break;
        }
        
      }
    }
  }

  int offset = 0;
  for (const auto& i : segmentsToRemove) {
    segments.erase(segments.begin() + i - offset);
    offset++;
  }

  return {merges,{}};
}

void Wire::mergeCollidingWires(std::unordered_set<Wire *>& deathRegistry, std::unordered_set<Propagator *>* collidingSet, std::unordered_set<Propagator *> excludeSet) {
  excludeSet.emplace(this);
  std::unordered_map<Wire*, std::unordered_set<Segment, SegmentHash>> mergingWires;

  bool wasNewList = false;
  if (collidingSet == nullptr) {
    wasNewList = true;
    collidingSet = new std::unordered_set<Propagator*>(globalProjectManager->gridManager.getOccupied(segments, excludeSet));
  }
	qDebug() << "colliding count =" << collidingSet->size();
	qDebug() << "this" << this;
	for (auto *p : *collidingSet)
			qDebug() << p;
	qDebug() << "excluding count =" << excludeSet.size();
	for (auto *p : excludeSet)
			qDebug() << p;
	for (const auto& propagator : *collidingSet) {
		if (propagator->getKind() != Propagator::Kinds::WIRE) continue;
		Wire* other = (Wire*)propagator;

    auto [colliding, excludedSegments] = trimCollidingAgainstWire(other);
    if (colliding) {
      mergingWires.emplace(other,excludedSegments);
      other->mergeCollidingWires(deathRegistry, nullptr, excludeSet);
    }
	}

  for (auto& [merger, excludedSegments] : mergingWires) {
    if (deathRegistry.find(merger) != deathRegistry.end()) continue;
    deathRegistry.emplace(merger);
    globalProjectManager->gridManager.removeFromGrid(merger->segments, merger);

    for (auto& seggy : merger->segments) { //seggy meant 2 be segment but i thought it was already used but it isnt..
      if(excludedSegments.find(seggy) != excludedSegments.end()) continue;
      segments.push_back(std::move(seggy));
    }
    for (int i = 0; i < globalProjectManager->currentOpenComponent->propagators.size(); i++) {
      auto& propagator = globalProjectManager->currentOpenComponent->propagators[i];
      if (propagator.get() == merger) {
        globalProjectManager->currentOpenComponent->propagators[i].release();
        globalProjectManager->currentOpenComponent->propagators.erase(globalProjectManager->currentOpenComponent->propagators.begin() + i);
        delete merger;
        break;
      }
    }
  }

  if (wasNewList)
    delete collidingSet;
}

Position Position::getGridScaledCopy(int offset) const {
    return {x*10+offset,y*10+offset};
}

void Wire::reset() {
    segments.clear();
}

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
    globalScheduler->registerCallback(effectors, excludedPropagator);
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
        0x0000: number of seg (4b)
        0x0004: first seg pos of segs
            seg layout:
            0x0000: int ax (4b)
            0x0004: int ay (4b)
            0x0000: int bx (4b)
            0x0004: int by (4b)
        0x....: number of effectors (4b)
        0x....: number of affectors (4b)
        0x....: first effector id (4b)
        0x....: first affector id (4b)
    0x....: first non-wire of non-wires (pins...?)
        pin layout:
        0x0000: effector Operation (4b)
        0x0004: relPosition x (4b)
        0x0008: relPosition y (4b)
        0x....: nameLength 4b
        name...
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
    if (namei != 0 && name.length() > 0) {
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
    propagatorsById.insert(wiresById.begin(), wiresById.end());
    propagatorsById.insert(pinsById.begin(), pinsById.end());

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
    filePath = path;
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
            wire->segments.emplace_back();
            wire->segments.back().begin.x = *bufferPos++;
            wire->segments.back().begin.y = *bufferPos++;
            wire->segments.back().end.x = *bufferPos++;
            wire->segments.back().end.y = *bufferPos++;
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

        const uint32_t pNameLength = *bufferPos++;
        for (uint32_t i = 0; i < pNameLength; i++) {
          const unsigned char c = (*bufferPos >> ((i % 4)*8)) & 0xff;
          pin->name.push_back(c);
          if (i % 4 == 3) {
            bufferPos++;
          }
        }
        if (pNameLength % 4 != 0) bufferPos++;

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
    *(data++) = relPosition.y;
    *data++ = name.length();
    uint32_t namei = 0;
    uint32_t nameBuffer = 0;
    for (unsigned char c : name) {
      uint32_t nc = c << namei*8;
      nameBuffer |= nc;
      if (namei == 3) {
        *data++ = nameBuffer;
        nameBuffer = 0;
        namei = 0;
      } else {
        namei++;
      }
    }
    if (namei != 0 && name.length() > 0) {
      *data++ = nameBuffer;
      nameBuffer = 0;
      namei = 0;
    }
    saveEffectorsAndAffectorsToAddres(data, map);
}

uint32_t Pin::getUint32sToSave() const {
    uint32_t count = 3;
    count++; //name
    count += ceil(((float)name.length())/4);
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
    count += segments.size()*4;
    count += getUint32sToSaveEffectorsAndAffectors();
    return count;
}

void Wire::saveToAddress(uint32_t* data, const std::unordered_map<Propagator*, uint32_t>& map) const {
    *data++ = segments.size();
    for (uint32_t i = 0; i < segments.size(); i++) {
        *data++ = segments[i].begin.x;
        *data++ = segments[i].begin.y;
        *data++ = segments[i].end.x;
        *data++ = segments[i].end.y;
    }
    saveEffectorsAndAffectorsToAddres(data, map);
}

Wire::Wire(const Wire& wireToCopy) : segments(wireToCopy.segments), Propagator(wireToCopy) {
}

Component::Component(const Component& componentToCopy) 
    :   name(componentToCopy.name),
    filePath(componentToCopy.filePath) 
    {
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
    name(pinToCopy.name),
    Propagator(pinToCopy)
    {

}

    Propagator::Propagator(const Propagator& propagator)
    : tickPropagationDelay(propagator.tickPropagationDelay),
    effectingState(propagator.effectingState)
    {}

Position Pin::gridPosition() const {
  return {relPosition.x * 10 + 5, relPosition.y * 10 + 5};
}

QPointF Pin::qGridPosition() const {
  return {(qreal)(relPosition.x * 10 + 5), (qreal)(relPosition.y * 10 + 5)};
}

QPointF Pin::gridAlignPoint(const QPointF& point) {
  return {(qreal)(int(point.x() / 10) + 5), (qreal)(int(point.y() / 10) + 5)};
}

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

            qDebug() << "Segment count:" << wire->segments.size();

            for (size_t j = 0; j < wire->segments.size(); ++j)
            {
                qDebug() << "   Anchor" << j
                         << "("
                         << wire->segments[j].begin.x
                         << ","
                         << wire->segments[j].begin.y
                         << ")"
												 << ":"
												 << "("
                         << wire->segments[j].end.x
                         << ","
                         << wire->segments[j].end.y
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