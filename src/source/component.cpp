#include "component.h"

#include "scheduler.h"
#include "filehelper.h"
#include <iostream>
#include <cstdint>
#include <cstring>
#include <functional>
#include <algorithm>

#include "notifications.h"
#include "projectmanager.h"
#include "componentholder.h"

#include "segmentgraphicsobject.h"
#include "pingraphicsobject.h"
#include "componentgraphicsobject.h"

namespace
{
    constexpr uint32_t COMPONENT_APPEARANCE_SECTION_MAGIC = 0x41505052; // 'APPR'

    constexpr uint32_t MAX_SANE_COUNT = 1'000'000;

    void checkSaneCount(uint32_t value, const char* what)
    {
        if (value > MAX_SANE_COUNT)
        {
            throw std::runtime_error(
                std::string("Corrupt save file: unreasonable count for ") + what
            );
        }
    }


    size_t packedStringUint32s(const std::string& text)
    {
        return 1 + (text.length() + 3) / 4;
    }

    void writePackedString(std::vector<uint32_t>& buffer, const std::string& text)
    {
        buffer.push_back(static_cast<uint32_t>(text.length()));

        uint32_t word = 0;
        uint32_t shift = 0;

        for (unsigned char c : text)
        {
            word |= (static_cast<uint32_t>(c) << (shift * 8));

            if (shift == 3)
            {
                buffer.push_back(word);
                word = 0;
                shift = 0;
            }
            else
            {
                ++shift;
            }
        }

        if (shift != 0)
            buffer.push_back(word);
    }

    size_t writePackedStringToAddress(uint32_t* address, const std::string& text)
    {
        uint32_t* start = address;

        *address++ = static_cast<uint32_t>(text.length());

        uint32_t word = 0;
        uint32_t shift = 0;

        for (unsigned char c : text)
        {
            word |= (static_cast<uint32_t>(c) << (shift * 8));

            if (shift == 3)
            {
                *address++ = word;
                word = 0;
                shift = 0;
            }
            else
            {
                ++shift;
            }
        }

        if (shift != 0)
            *address++ = word;

        return static_cast<size_t>(address - start);
    }

    std::string readPackedString(const std::function<uint32_t(bool)>& readU32)
    {
        const uint32_t length = readU32(true);
        checkSaneCount(length, "string length");

        std::string result;
        result.reserve(length);

        for (uint32_t i = 0; i < length; ++i)
        {
            const unsigned char c =
                (readU32(false) >> ((i % 4) * 8)) & 0xff;

            result.push_back(static_cast<char>(c));

            if (i % 4 == 3)
                readU32(true);
        }

        if (length % 4 != 0)
            readU32(true);

        return result;
    }
}

Component::Component() {}
Component::Component(const std::string& _name) : name(_name) {}

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

Wire::~Wire() {
  qDebug(std::format("destroyingguhhhh 0x{:x}",(unsigned long long)graphicsObject).c_str());
  if (graphicsObject) {
    qDebug("graphicsItem");
    globalProjectManager->workspace->scene()->removeItem(graphicsObject);
    delete graphicsObject;
  }
}

std::vector<Position> Wire::junctionPoints() const {
    std::unordered_map<Position, int, PositionHash> degree;

    for (const auto& segment : segments) {
        degree[segment.begin] += 1;
        degree[segment.end]   += 1;
    }

    for (size_t i = 0; i < segments.size(); i++) {
        const auto& segment = segments[i];
        bool horizontal = segment.begin.y == segment.end.y;

        int lo = horizontal ? std::min(segment.begin.x, segment.end.x)
                             : std::min(segment.begin.y, segment.end.y);
        int hi = horizontal ? std::max(segment.begin.x, segment.end.x)
                             : std::max(segment.begin.y, segment.end.y);

        for (size_t j = 0; j < segments.size(); j++) {
            if (i == j) continue;
            const auto& other = segments[j];

            for (const Position& p : {other.begin, other.end}) {
                if (horizontal) {
                    if (p.y != segment.begin.y) continue;
                    if (p.x <= lo || p.x >= hi) continue; 
                } else {
                    if (p.x != segment.begin.x) continue;
                    if (p.y <= lo || p.y >= hi) continue;
                }
                degree[p] += 2;
            }
        }
    }

    std::vector<Position> junctions;
    for (const auto& [pos, count] : degree) {
        if (count >= 3) junctions.push_back(pos);
    }
    return junctions;
}

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
	if (merges) {
		for (auto& seggy : other->segments) {
			segments.push_back(seggy);
      globalProjectManager->gridManager.addToGrid(seggy, this);
		}
	}

  return {merges,{}};
}

void Wire::mergeCollidingWires(std::unordered_set<Wire *>& deathRegistry, std::unordered_set<Propagator *>& excludeSet, std::unordered_set<Propagator *>* collidingSet) {
  excludeSet.emplace(this);
  std::unordered_map<Wire*, std::unordered_set<Segment, SegmentHash>> mergingWires;

  bool wasNewList = false;
  if (collidingSet == nullptr) {
    wasNewList = true;
    collidingSet = new std::unordered_set<Propagator*>(globalProjectManager->gridManager.getOccupied(segments, excludeSet));
  }
	for (const auto& propagator : *collidingSet) {
		if (propagator->getKind() != Propagator::Kinds::WIRE) continue;
		Wire* other = (Wire*)propagator;

    auto [colliding, excludedSegments] = trimCollidingAgainstWire(other);
		if (colliding) {
			markJunctionsDirty();
      mergingWires.emplace(other,excludedSegments);
      other->mergeCollidingWires(deathRegistry, excludeSet);
    }
	}

  for (auto& [merger, excludedSegments] : mergingWires) {
    if (deathRegistry.find(merger) != deathRegistry.end()) continue;
    deathRegistry.emplace(merger);
    globalProjectManager->gridManager.removeFromGrid(merger->segments, merger);

    for (int i = 0; i < globalProjectManager->currentOpenComponent->getPropagators().size(); i++) {
      auto& propagator = globalProjectManager->currentOpenComponent->getPropagators()[i];
      if (propagator.get() == merger) {
        globalProjectManager->currentOpenComponent->getPropagators()[i].release();
        globalProjectManager->currentOpenComponent->getPropagators().erase(globalProjectManager->currentOpenComponent->getPropagators().begin() + i);
        delete merger;
        break;
      }
    }
  }

  if (wasNewList)
    delete collidingSet;
}

void Wire::reset() {
    segments.clear();
}

bool Wire::removeSegmentAt(size_t index) {
    if (index >= segments.size())
        return false;

    segments.erase(segments.begin() + static_cast<long>(index));
    markJunctionsDirty();
    return true;
}

int Wire::nearestSegmentIndex(const Position& point) const {
    if (segments.empty())
        return -1;

    int bestIndex = 0;
    long long bestDistSq = -1;

    for (size_t i = 0; i < segments.size(); ++i) {
        const auto& segment = segments[i];

        int clampedX;
        int clampedY;

        if (segment.begin.y == segment.end.y) {
            // horizontal segment
            const int minX = std::min(segment.begin.x, segment.end.x);
            const int maxX = std::max(segment.begin.x, segment.end.x);
            clampedX = std::clamp(point.x, minX, maxX);
            clampedY = segment.begin.y;
        } else {
            // vertical segment
            const int minY = std::min(segment.begin.y, segment.end.y);
            const int maxY = std::max(segment.begin.y, segment.end.y);
            clampedX = segment.begin.x;
            clampedY = std::clamp(point.y, minY, maxY);
        }

        const long long dx = point.x - clampedX;
        const long long dy = point.y - clampedY;
        const long long distSq = dx * dx + dy * dy;

        if (bestDistSq < 0 || distSq < bestDistSq) {
            bestDistSq = distSq;
            bestIndex = static_cast<int>(i);
        }
    }

    return bestIndex;
}

void Propagator::addRelatedPropagator(AbstractPropagator* abstract) {
  if (abstract->isAbstract()) {
    qDebug("Not sure what to do or how it would happen but um,,,, component?");
  } else {
    addEffector((Propagator*)abstract);
    addAffector((Propagator*)abstract);
  }
}

Propagator::~Propagator() {

    for (Propagator* effector : effectors) {
        effector->affectors.erase(this);
    }
    for (Propagator* affector : affectors) {
        affector->effectors.erase(this);
    }
}

void Propagator::forgetPropagator(AbstractPropagator* abstract) {
  if (abstract->isAbstract()) {
    qDebug("Not sure what to do or how it would happen but um,,,, component?");
  } else {
    forget((Propagator*)abstract);
  }
}

void Propagator::evaluateEffectingState() {
    auto it = affectors.begin();
    States currentState = affectors.end() != it ? (*it)->getEffectingState() : States::FLOATING;
    it++;
    while (it != affectors.end()) {
        currentState = evaluateTwoStates(currentState, (*it)->getEffectingState());
    }
    effectingState = currentState;
}

void Propagator::propagate(Propagator* excludedPropagator) {
    evaluateEffectingState();
    globalScheduler->registerCallback(effectors, excludedPropagator);
}

void Pin::evaluateEffectingState() {
    switch (effectorOperation) {
        case Operations::OR:
            switch (affectors.size()) {
            case 0:
                effectingState = States::FLOATING;
                break;
            case 1:
                effectingState = (*(*affectors.begin())).getEffectingState();
                break;
            default:
                effectingState = States::FLOATING;
                for (const auto* affector : affectors) {
                    const States state = (States)(affector->getEffectingState() | effectingState);
                    effectingState = state > 1 ? std::max(effectingState, affector->getEffectingState()) : state;
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
                effectingState = (*(*affectors.begin())).getEffectingState();
                break;
            default:
                effectingState = States::FLOATING;
                for (const auto* affector : affectors) {
                    effectingState = (affector->getEffectingState() | effectingState) > 1 ? std::max(effectingState, affector->getEffectingState()) : (States)(affector->getEffectingState() & effectingState);
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
                effectingState = (*(*affectors.begin())).getEffectingState();
                break;
            default:
                effectingState = States::FLOATING;
                for (const auto* affector : affectors) {
                    effectingState = (affector->getEffectingState() | effectingState) > 1 ? std::max(effectingState, affector->getEffectingState()) : (States)(affector->getEffectingState() ^ effectingState);
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
                effectingState = (*(*affectors.begin())).getEffectingState();
                break;
            default:
                effectingState = States::FLOATING;
                for (const auto* affector : affectors) {
                    effectingState = (affector->getEffectingState() != effectingState) ? States::CONFLICT : effectingState == States::FLOATING ? States::FLOATING : (States)(!affector->getEffectingState());
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
                effectingState = (*(*affectors.begin())).getEffectingState();
                break;
            default:
                effectingState = States::FLOATING;
                for (const auto* affector : affectors) {
                    const States state = (States)(affector->getEffectingState() | effectingState);
                    effectingState = state > 1 ? std::max(effectingState, affector->getEffectingState()) : (States)((~state)&1);
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
                effectingState = (*(*affectors.begin())).getEffectingState();
                break;
            default:
                effectingState = States::FLOATING;
                for (const auto* affector : affectors) {
                    effectingState = (affector->getEffectingState() | effectingState) > 1 ? std::max(effectingState, affector->getEffectingState()) : (States)((~(affector->getEffectingState() & effectingState))&1);
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
                effectingState = (*(*affectors.begin())).getEffectingState();
                break;
            default:
                effectingState = States::FLOATING;
                for (const auto* affector : affectors) {
                    effectingState = (affector->getEffectingState() | effectingState) > 1 ? std::max(effectingState, affector->getEffectingState()) : (States)(~(affector->getEffectingState() ^ effectingState)&1);
                }
                break;
            }
            break;
        case Operations::BUFFER:
        {
            auto it = affectors.begin();
            States currentState = affectors.end() != it ? (*it)->getEffectingState() : States::FLOATING;
            it++;
            while (it != affectors.end()) {
                currentState = evaluateTwoStates(currentState, (*it)->getEffectingState());
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



bool SentinelComponent::saveToFile(const std::filesystem::path& path) const
{
    try
    {
        std::vector<uint32_t> buffer;

        buffer.push_back(COMPONENT_SAVE_VERSION_MAJOR);
        buffer.push_back(COMPONENT_SAVE_VERSION_MINOR);

        writePackedString(buffer, name);

        buffer.push_back(static_cast<uint32_t>(propagators.size()));

        uint32_t propagatorId = 0;
        std::unordered_map<Propagator*, uint32_t> wiresById;
        std::unordered_map<Propagator*, uint32_t> pinsById;
        std::vector<Component*> components;

        const uint32_t uint32Size = getUint32Size(propagatorId, wiresById, pinsById, components, true);

        buffer.push_back(static_cast<uint32_t>(wiresById.size()));
        buffer.push_back(static_cast<uint32_t>(pinsById.size()));
        buffer.push_back(static_cast<uint32_t>(components.size()));

        const size_t propagatorSectionOffset = buffer.size();
        buffer.resize(buffer.size() + uint32Size);

        size_t propagatorIndex = propagatorSectionOffset;

        for (const auto& [wire, id] : wiresById) {
            wire->saveToAddress(buffer.data() + propagatorIndex);
            propagatorIndex += wire->getUint32sToSave();
        }
        for (const auto& [pin, id] : pinsById) {
            pin->saveToAddress(buffer.data() + propagatorIndex);
            propagatorIndex += pin->getUint32sToSave();
        }
        for (const auto* component : components) {
            component->saveToAddress(buffer.data() + propagatorIndex);
            propagatorIndex += component->getUint32sToSave();
        }

        buffer.push_back(COMPONENT_APPEARANCE_SECTION_MAGIC);

        const size_t appearanceUint32s = appearance.getUint32sToSave();
        const size_t appearanceOffset = buffer.size();
        buffer.resize(buffer.size() + appearanceUint32s);
        appearance.saveToAddress(buffer.data() + appearanceOffset);

        writeUint32VectorToFile(path, buffer);

        return true;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Failed to save component \"" << name << "\": " << e.what() << '\n';
        return false;
    }
}

bool SentinelComponent::loadFromFile(const std::filesystem::path& path) {
  try
  {
    filePath = path;
    std::vector<uint32_t> buffer = openFileToUint32Vector(path);

    size_t pos = 0;

    auto readU32 = [&](bool inc = true) -> uint32_t {
        if (pos >= buffer.size()) {
            qDebug() << "READ PAST END OF BUFFER!";
            qDebug() << "pos =" << pos;
            qDebug() << "size =" << buffer.size();
            throw std::runtime_error("Unexpected EOF while loading component");
        }
        if (inc)
          return buffer[pos++];
        else 
          return buffer[pos];
    };

    const uint32_t majorVersion = readU32();
    const uint32_t minorVersion = readU32();
    (void)minorVersion;

    if (majorVersion != COMPONENT_SAVE_VERSION_MAJOR) {
      qDebug() << "Component save file has an unexpected major version ("
                << majorVersion << "vs expected" << COMPONENT_SAVE_VERSION_MAJOR
                << ") -- attempting to load anyway.";
    }

    name = readPackedString(readU32);

    const uint32_t totalPropagators = readU32();
    checkSaneCount(totalPropagators, "propagator count");
    (void)totalPropagators; // layout placeholder, not otherwise used

    const uint32_t totalWires = readU32();
    const uint32_t totalPins = readU32();
    const uint32_t totalComponents = readU32();

    checkSaneCount(totalWires, "wire count");
    checkSaneCount(totalPins, "pin count");
    checkSaneCount(totalComponents, "nested component count");

    for (uint32_t i = 0; i < totalWires; i++) {

        auto wire = std::make_unique<Wire>();

        const uint32_t segmentCount = readU32();
        checkSaneCount(segmentCount, "wire segment count");

        wire->segments.reserve(segmentCount);

        for (uint32_t j = 0; j < segmentCount; j++) {
            Segment segment;
            segment.begin.x = static_cast<int>(readU32());
            segment.begin.y = static_cast<int>(readU32());
            segment.end.x = static_cast<int>(readU32());
            segment.end.y = static_cast<int>(readU32());
            wire->segments.push_back(segment);
        }

        addPropagator(std::move(wire));
    }

    for (uint32_t i = 0; i < totalPins; i++) {

        auto pin = std::make_unique<Pin>(*this);

        pin->setEffectorOperation(static_cast<Pin::Operations>(readU32()));
        pin->setGridPosition({static_cast<int>(readU32()), static_cast<int>(readU32())});
        pin->setAppearancePosition({static_cast<int>(readU32()), static_cast<int>(readU32())});
        pin->setName(readPackedString(readU32));

        addPropagator(std::move(pin));
    }

    for (uint32_t i = 0; i < totalComponents; i++) {

        const std::string compName = readPackedString(readU32);

        Position compPos;
        compPos.x = static_cast<int>(readU32());
        compPos.y = static_cast<int>(readU32());

        auto it = globalProjectManager->components.find(compName);
        if (it != globalProjectManager->components.end()) {
          createComponent(compPos, *it->second);
        } else {
          globalProjectManager->unresolvedSentinelComponents.emplace(this);
          unresolvedComponentPositions[compName].push_back(compPos);
        }
    }

    isResolved(); 


    if (pos < buffer.size() && buffer[pos] == COMPONENT_APPEARANCE_SECTION_MAGIC) {
      pos++;
      appearance.loadFromReader(readU32);
    } else {
      qDebug() << "No appearance section found for"
               << QString::fromStdString(name)
               << "-- using default appearance (older save file?).";
    }

    qDebug() << "propagators size =" << propagators.size();
  }
  catch(const std::exception& e)
  {
    std::cerr << "Failed to load component from \"" << path.string() << "\": " << e.what() << '\n';
    return false;
  }

  return true;
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

void Pin::saveToAddress(uint32_t* data) const {
    *data++ = effectorOperation;
    *data++ = static_cast<uint32_t>(getGridPosition().x);
    *data++ = static_cast<uint32_t>(getGridPosition().y);
    *data++ = static_cast<uint32_t>(appearancePosition.x);
    *data++ = static_cast<uint32_t>(appearancePosition.y);
    writePackedStringToAddress(data, name);
}

size_t Pin::getUint32sToSave() const {
    // operation + gridPosition.x/.y + appearancePosition.x/.y
    return 5 + packedStringUint32s(name);
}

size_t Wire::getUint32sToSave() const {
    uint32_t count = 1;
    count += segments.size()*4;
    return count;
}

void Wire::saveToAddress(uint32_t* data) const {
    *data++ = segments.size();
    for (uint32_t i = 0; i < segments.size(); i++) {
        *data++ = segments[i].begin.x;
        *data++ = segments[i].begin.y;
        *data++ = segments[i].end.x;
        *data++ = segments[i].end.y;
    }
}

Wire::Wire(const Wire& wireToCopy) : segments(wireToCopy.segments), Propagator(wireToCopy) {
}


Component::~Component()
{
    if (sourceSentinel) {
        sourceSentinel->unregisterInstance(this);
    }

    if (graphicsObject &&
        globalProjectManager &&
        globalProjectManager->workspace &&
        globalProjectManager->workspace->scene())
    {
        globalProjectManager->workspace->scene()->removeItem(graphicsObject);
        delete graphicsObject;
        graphicsObject = nullptr;
    }
}

SentinelComponent::~SentinelComponent()
{
    for (Component* instance : instances) {
        instance->clearSourceSentinel();
    }
}

Component::Component(const Component& componentToCopy) 
    :   name(componentToCopy.name),
    filePath(componentToCopy.filePath),
    appearance(componentToCopy.getAppearance())
    {
    std::unordered_map<Propagator*, Propagator*> oldNewPropagatorPointerMap;
    std::unordered_map<Pin*, Propagator*> pins;
    std::unordered_map<Wire*, Propagator*> wires;
    for (const auto& __propagator : componentToCopy.propagators) {
      if (!__propagator->isAbstract()) {
        auto propagator = (Propagator*)__propagator.get();
        switch (propagator->getKind()) {
          case Propagator::Kinds::PIN:{
              auto ptr = std::make_unique<Pin>(*(Pin*)propagator, *this);
              auto [element, success] = pins.emplace(ptr.get(), propagator);
              oldNewPropagatorPointerMap.emplace(element->second, element->first);
              addPropagator(std::move(ptr));
              break;
          }
          case Propagator::Kinds::WIRE:{
              auto ptr = std::make_unique<Wire>(*(Wire*)propagator);
              auto [element, success] = wires.emplace(ptr.get(), propagator);
              oldNewPropagatorPointerMap.emplace(element->second, element->first);
              addPropagator(std::move(ptr));
              break;
          }
        }
      } else {
        auto component = (Component*)__propagator.get();
        auto cptr = new Component(*component);
        cptr->setGridPosition(component->getGridPosition());
        std::unique_ptr<Component> uptr(cptr);
        addPropagator(std::move(uptr));
        size_t i = 0;
        for (const auto& _propagator : cptr->propagators) {
          if (!_propagator->isAbstract() && ((Propagator*)_propagator.get())->getKind() == Propagator::Kinds::PIN) {
            auto [element, success] = pins.emplace((Pin*)(component->propagators[i].get()), (Pin*)_propagator.get());
            oldNewPropagatorPointerMap.emplace(element->second, element->first);
          }
          i++;
        }
      }
    }

    for (const auto [pin, propagator] : pins) {
        pin->copyEffectorsAndEffectors(*propagator, oldNewPropagatorPointerMap);
    }

    for (const auto [wire, propagator] : wires) {
        wire->copyEffectorsAndEffectors(*propagator, oldNewPropagatorPointerMap);
    }

    sourceSentinel = componentToCopy.sourceSentinel;
    if (sourceSentinel) {
        sourceSentinel->registerInstance(this);
    }
}

bool Component::removePropagator(AbstractPropagator* propagator)
{
    for (size_t i = 0; i < propagators.size(); ++i) {
        if (propagators[i].get() == propagator) {
            propagators.erase(propagators.begin() + i);
            return true;
        }
    }

    return false;
}

ComponentAppearance& Component::getAppearance()
{
    if (sourceSentinel) {
        return sourceSentinel->getAppearance();
    }

    return appearance;
}

const ComponentAppearance& Component::getAppearance() const
{
    if (sourceSentinel) {
        return sourceSentinel->getAppearance();
    }

    return appearance;
}

void Component::setAppearance(const ComponentAppearance& value)
{
    if (sourceSentinel) {
        qDebug() << "Component::setAppearance() ignored -- this is a placed"
                    " instance; edit its source sentinel's appearance instead.";
        return;
    }

    appearance = value;
}

void Component::resyncFromSentinel()
{
    if (!sourceSentinel)
        return;

    const auto templatePins = sourceSentinel->getPins();

    std::unordered_map<std::string, Pin*> existingByName;
    for (Pin* pin : getPins()) {
        existingByName.emplace(pin->getName(), pin);
    }

    std::unordered_set<std::string> templateNames;
    templateNames.reserve(templatePins.size());

    for (const Pin* templatePin : templatePins) {
        templateNames.insert(templatePin->getName());

        auto it = existingByName.find(templatePin->getName());

        if (it != existingByName.end()) {
            it->second->setEffectorOperation(templatePin->getEffectorOperation());
            it->second->setAppearancePosition(templatePin->getAppearancePosition());
            it->second->setGridPosition(templatePin->getGridPosition());
        } else {
            auto newPin = std::make_unique<Pin>(*this);
            newPin->setName(templatePin->getName());
            newPin->setEffectorOperation(templatePin->getEffectorOperation());
            newPin->setAppearancePosition(templatePin->getAppearancePosition());
            newPin->setGridPosition(templatePin->getGridPosition());
            addPropagator(std::move(newPin));
        }
    }

    for (size_t i = 0; i < propagators.size(); ) {
        auto* propagator = propagators[i].get();

        if (!propagator->isAbstract() &&
            static_cast<Propagator*>(propagator)->getKind() == Propagator::Kinds::PIN &&
            templateNames.find(static_cast<Pin*>(propagator)->getName()) == templateNames.end())
        {
            propagators.erase(propagators.begin() + i);
        } else {
            ++i;
        }
    }

    if (graphicsObject) {
        graphicsObject->refresh();
    }

    notifyChildStructureChanged();
}

void Component::notifyChildStructureChanged()
{
    reevaluateConnections();

    if (parentComponent) {
        parentComponent->notifyChildStructureChanged();
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

uint32_t Component::getUint32Size(uint32_t& propagatorId, std::unordered_map<Propagator*, uint32_t>& wiresById,std::unordered_map<Propagator*, uint32_t>& pinsById, std::vector<Component*>& components, bool canRecurse = false) const {
  uint32_t uint32Size = 0;

  for (auto& _propagator : propagators) {
    if (!_propagator->isAbstract()) {
      auto* propagator = (Propagator*)_propagator.get();
      if (propagator->getKind() == Propagator::Kinds::PIN) {
        pinsById.emplace(propagator, propagatorId++);
        uint32Size += propagator->getUint32sToSave();
      } else if (propagator->getKind() == Propagator::Kinds::WIRE && canRecurse) {
        wiresById.emplace(propagator, propagatorId++);
        uint32Size += propagator->getUint32sToSave();
      }
    } else if (canRecurse) {
      auto* component = (Component*)_propagator.get();
      components.push_back(component);
      uint32Size += component->getUint32sToSave();
    }
      
  }

  return uint32Size;
}

Pin::Pin(const Pin& pinToCopy, Component& _parent)
    : parent(_parent),
    effectorOperation(pinToCopy.effectorOperation),
    appearancePosition(pinToCopy.appearancePosition),
    name(pinToCopy.name),
    Propagator(pinToCopy)
    {

}

    Propagator::Propagator(const Propagator& propagator)
    : tickPropagationDelay(propagator.tickPropagationDelay),
    effectingState(propagator.getEffectingState()),
    AbstractPropagator(propagator)
    {}

Position Pin::gridPosition() const {
  return {getGridPosition().x * 10 + 5, getGridPosition().y * 10 + 5};
}

QPointF Pin::qGridPosition() const {
  return {(qreal)(getGridPosition().x * 10 + 5), (qreal)(getGridPosition().y * 10 + 5)};
}

Pin::~Pin() {
    if (graphicsObject &&
        globalProjectManager &&
        globalProjectManager->workspace &&
        globalProjectManager->workspace->scene())
    {
        globalProjectManager->workspace->scene()->removeItem(graphicsObject);
        delete graphicsObject;
        graphicsObject = nullptr;
    }
}

QPointF Pin::gridAlignPoint(const QPointF& point) {
  return {(qreal)(int(point.x() / 10) + 5), (qreal)(int(point.y() / 10) + 5)};
}

bool SentinelComponent::informAddedComponentToSeeIfFullyResolved(const std::string& name, const SentinelComponent& _component) {
  auto it = unresolvedComponentPositions.find(name);
  if (it != unresolvedComponentPositions.end()) {
    for (const auto& _position : it->second) {
      createComponent(_position, _component);
    }
    unresolvedComponentPositions.erase(name);
  }
  return evaluateResolved();
}

size_t Component::getUint32sToSave() const {
  return packedStringUint32s(name) + 2; // + gridPosition.x/.y
}

void Component::saveToAddress(uint32_t* dataPtr) const {
  dataPtr += writePackedStringToAddress(dataPtr, name);
  *dataPtr++ = static_cast<uint32_t>(gridPosition.x);
  *dataPtr++ = static_cast<uint32_t>(gridPosition.y);
}

void SentinelComponent::createComponent(const Position& _position, const SentinelComponent& _component) {
  auto cptr = new Component(_component);
  cptr->setGridPosition(_position);
  cptr->sourceSentinel = const_cast<SentinelComponent*>(&_component);
  _component.registerInstance(cptr);

  std::unique_ptr<Component> uptr(cptr);
  addPropagator(std::move(uptr));
}

std::unique_ptr<AbstractPropagator> SentinelComponent::createDerivativeComponent(const Position& _position) const {
  auto cptr = new Component((Component)*this);
  cptr->setGridPosition(_position);
  cptr->sourceSentinel = const_cast<SentinelComponent*>(this);
  registerInstance(cptr);
  return std::unique_ptr<Component>(cptr);
}

Component SentinelComponent::getDuplicate() const {
  return Component(*this);
}

void SentinelComponent::notifyInstancesOfStructureChange()
{
    const auto instancesSnapshot = instances;

    for (Component* instance : instancesSnapshot) {
        instance->resyncFromSentinel();
    }
}

void SentinelComponent::refreshInstanceGraphics()
{
    for (Component* instance : instances) {
        if (auto* item = instance->getGraphicsObject()) {
            item->refresh();
        }
    }
}

void SentinelComponent::simulateConnections() {
  ComponentHolder holderEvaluator;

  for (const auto& _propagator : propagators) {
    if (!_propagator->isAbstract()) {
      auto __propagator = (Propagator*)_propagator.get();
      if (__propagator->getKind() == Propagator::Kinds::PIN) {
        auto propagator = (Pin*)__propagator;
        holderEvaluator.addToGrid(propagator->getGridPosition(), propagator);
      } else if (__propagator->getKind() == Propagator::Kinds::WIRE) {
        auto propagator = (Wire*)__propagator;
        holderEvaluator.addToGrid(propagator->segments, propagator);
      }
    } else {
      auto propagator = (Component*)_propagator.get();
      const auto& anchor = propagator->getAppearance().anchor;

      for (const auto& _pin : propagator->getPropagators()) {
        if (_pin->isAbstract() && ((Propagator*)_pin.get())->getKind() == Propagator::Kinds::PIN) {
          auto* pin = (Pin*)_pin.get();

          holderEvaluator.addToGrid(anchor + pin->getGridPosition(), pin);
        }
      }

    }
  }

  for (const auto& _propagator : propagators) {
    if (!_propagator->isAbstract()) {
      auto __propagator = (Propagator*)_propagator.get();
      if (__propagator->getKind() == Propagator::Kinds::PIN) {
        auto propagator = (Pin*)__propagator;
        holderEvaluator.getOccupied(propagator->getGridPosition(), {propagator});
      } else if (__propagator->getKind() == Propagator::Kinds::WIRE) {
        auto propagator = (Wire*)__propagator;
        holderEvaluator.getOccupied(propagator->segments, {propagator});
      }
    } else {
      auto propagator = (Component*)_propagator.get();
      const auto& anchor = propagator->getAppearance().anchor;

      for (const auto& _pin : propagator->getPropagators()) {
        if (_pin->isAbstract() && ((Propagator*)_pin.get())->getKind() == Propagator::Kinds::PIN) {
          auto* pin = (Pin*)_pin.get();

          auto set = holderEvaluator.getOccupied(anchor + pin->getGridPosition(), {pin});
          pin->addEffectors(set);
          pin->addAffectors(set);
        }
      }
    }
  }
}