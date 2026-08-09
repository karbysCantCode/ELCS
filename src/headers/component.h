#ifndef COMPONENT_H
#define COMPONENT_H

#include <QObject>
#include <QPointF>
#include <QGraphicsItem>
#include <unordered_set>
#include <filesystem>
#include <fstream>

// #include "segmentgraphicsobject.h"
#include "componentappearance.h"
#include "position.h"

class ComponentGraphicsObject;
class SegmentGraphicsObject;
class PinGraphicsObject;

enum States
{
    LOW = 0,
    HIGH = 1,
    FLOATING = 2,
    CONFLICT = 3,
    ERROR = 4
};

class Component;
class ComponentAppearance;

class AbstractPropagator {
public:
  AbstractPropagator() {}
  AbstractPropagator(const AbstractPropagator& abstract) :
    gridPosition(abstract.getGridPosition())
  {}
  virtual ~AbstractPropagator() = default;
  virtual constexpr bool isAbstract() const {return true;}
  Position getGridPosition() const {return gridPosition;}
  void setGridPosition(const Position& _position) {gridPosition = _position;}
  
  virtual void addRelatedPropagator(AbstractPropagator* abstract) = 0;
  virtual void forgetPropagator(AbstractPropagator* abstract) = 0;
  virtual size_t getUint32sToSave() const = 0;
  virtual void saveToAddress(uint32_t* address) const = 0;
protected:
    Position gridPosition;
};

class Propagator : public AbstractPropagator
{
protected:
    static States evaluateTwoStates(const States& stateA, const States& stateB);
    static uint32_t findIdOfPropagatorPointer(Propagator* propagator, const std::unordered_map<Propagator*, uint32_t>& map);

    int tickPropagationDelay = 1;
    States effectingState = States::FLOATING;
    std::unordered_set<Propagator*> effectors; // those that this propagator effects
    std::unordered_set<Propagator*> affectors; // those that this propagator is affected by
    bool acceptsEffects = true;
public:
    virtual constexpr bool isAbstract() const override {return false;}
    enum Kinds {
        WIRE,
        PIN
    };

    void setTickPropagationDelay(int delay) {tickPropagationDelay=delay;}
    int getTickPropagationDelay() const {return tickPropagationDelay;}

    States getEffectingState() const {return effectingState;}

    void setAcceptsEffects(bool accepts) {acceptsEffects=accepts;}
    bool getAcceptsEffects() const {return acceptsEffects;}

    void addEffector(Propagator* effector) {if (!acceptsEffects)return; effectors.emplace(effector);}
    void addAffector(Propagator* affector) {affectors.emplace(affector);}
    void addEffectors(std::unordered_set<Propagator*> _effectors) {if (!acceptsEffects)return; effectors.insert(_effectors.begin(), _effectors.end());}
    void addAffectors(std::unordered_set<Propagator*> _affectors) {affectors.insert(_affectors.begin(), _affectors.end());}
    std::unordered_set<Propagator*> getEffectors() const {return effectors;}
    std::unordered_set<Propagator*> getAffectors() const {return affectors;}
    void forget(Propagator* propagator) {effectors.erase(propagator); affectors.erase(propagator);}
    virtual void addRelatedPropagator(AbstractPropagator* abstract) override;
    virtual void forgetPropagator(AbstractPropagator* abstract) override;
    //TODO SEE DEF FOR NOTE
    // Defaults to effectors = affectors.
    virtual void evaluateEffectingState(); //allow warn for multi low/high, technically not conflict but....
    // Defaults to effectors = affectors.
    // virtual void evaluateEffectors();

    //evaluates effecting state and ques effectors for evaluation.
    void propagate(Propagator* excludedPropagator = nullptr);

    virtual Kinds getKind() const = 0;

    // virtual uint32_t getUint32sToSave() const = 0;
    // virtual uint32_t getUint32sToSaveEffectorsAndAffectors() const;
    //begins at data.
    // virtual void saveToAddress(uint32_t* data) const = 0;
    //begins at data.
    // virtual void saveEffectorsAndAffectorsToAddres(uint32_t* data, const std::unordered_map<Propagator*, uint32_t>& map) const;

    void copyEffectorsAndEffectors(const Propagator& propagator, std::unordered_map<Propagator*, Propagator*>& oldNewPropagatorPointerMap);

    Propagator() {};
    Propagator(const Propagator& propagator);
    virtual ~Propagator() = default;
};

struct Segment {
    Position begin;
    Position end;

    Segment(int ax, int ay, int bx, int by) : begin(ax,ay), end(bx,by) {}
    Segment(const Position& _begin, const Position& _end) : begin(_begin), end(_end) {}
    Segment() {}

     bool operator==(const Segment& other) const
      {
        return begin == other.begin &&
              end == other.end;
      }

    // first = begin, second = end
    std::pair<bool,bool> doesSegmentEndsSitAlongSegment(bool targetLineHorizontal, const Segment& segment) const;
    
};

struct SegmentHash;

class Wire : public Propagator
{
public:
    std::vector<Segment> segments;
    SegmentGraphicsObject* graphicsObject = nullptr;
    virtual size_t getUint32sToSave() const override;
    virtual void saveToAddress(uint32_t* data) const override;
    //returns true if it needs to be destroyed
    void mergeCollidingWires(std::unordered_set<Wire *>& deathRegistry, std::unordered_set<Propagator *>& excludeSet, std::unordered_set<Propagator *>* collidingSet = nullptr);
    std::pair<bool, std::unordered_set<Segment, SegmentHash>> trimCollidingAgainstWire(Wire* other);
    void reset();

    void setGraphicsObject(SegmentGraphicsObject* object)  {graphicsObject=object;}

    // does not sync effectors and affectors.
    Wire(const Wire& wireToCopy);
    Wire() {}

    ~Wire();

    Kinds getKind() const override {return Kinds::WIRE;}
    void updateJunctions() {cachedJunctions=junctionPoints();}
    std::vector<Position> cachedJunctions;
    bool junctionsDirty = true;
    void markJunctionsDirty() { junctionsDirty = true; }
private:    
    std::vector<Position> junctionPoints() const;
};

class Pin : public Propagator
{
public:
    enum Operations {
        OR,
        AND,
        XOR,
        NOT,

        NOR,
        NAND,
        XNOR,
        BUFFER
    };

    
    Operations getEffectorOperation() const { return effectorOperation; }
    void setEffectorOperation(Operations value) { effectorOperation = value; }
    
    // Position& getRelPosition() { return relPosition; }
    // const Position& getRelPosition() const { return relPosition; }
    // void setRelPosition(const Position& value) { relPosition = value; }
    
    Position getAppearancePosition() const {return appearancePosition;}
    void setAppearancePosition(const Position& _appearancePosition) {appearancePosition = _appearancePosition;}
    
    Position gridPosition() const;
    QPointF qGridPosition() const;
    
    static QPointF gridAlignPoint(const QPointF& point);
    
    const std::string& getName() const { return name; }
    void setName(const std::string& newName) { name = newName; }
    
    PinGraphicsObject* getGraphicsObject() const {return graphicsObject;}
    void setGraphicsObject(PinGraphicsObject* object)  {graphicsObject=object;}

    Component& getParent() { return parent; }
    const Component& getParent() const { return parent; }
    
    virtual void evaluateEffectingState() override;
    virtual Kinds getKind() const override {return Kinds::PIN;}
    virtual size_t getUint32sToSave() const override;
    virtual void saveToAddress(uint32_t* data) const override;
    Pin(Component& _parent, States* _state = nullptr) : parent(_parent), state(_state) {}
    // does not sync effectors and affectors.
    Pin(const Pin& pinToCopy, Component& _parent);
private:
    Operations effectorOperation = Operations::BUFFER;
    // Position relPosition;
    Position appearancePosition;
    std::string name;
    Component& parent;
    PinGraphicsObject* graphicsObject = nullptr;
    // state should be inherited by wire, begins nullptr assuming no connected wire, beware.
    States* state = nullptr; //unused...
};

class Component : public AbstractPropagator
{
    protected:
    struct PropagatorIdentity {
    public:
        Propagator* propagator = nullptr;
        std::unordered_set<uint32_t> effectorIds;
        std::unordered_set<uint32_t> affectorIds;
        PropagatorIdentity(Propagator* _propagator) : propagator(_propagator) {}
    };
public:

    // name
    const std::string& getName() const { return name; }
    void setName(const std::string& value) { name = value; }

    // filePath
    const std::filesystem::path& getFilePath() const { return filePath; }
    void setFilePath(const std::filesystem::path& value) { filePath = value; }

    // propagators
    const std::vector<std::unique_ptr<AbstractPropagator>>& getPropagators() const {
        return propagators;
    }

    std::vector<std::unique_ptr<AbstractPropagator>>& getPropagators() {
        return propagators;
    }

    std::vector<Pin*> getPins() {
        std::vector<Pin*> pins;
        for (auto& propagator : propagators) {
            if (!propagator->isAbstract() && ((Propagator*)propagator.get())->getKind() == Propagator::Kinds::PIN)
                pins.push_back((Pin*)propagator.get());
        }
        return pins;
    }

    std::vector<const Pin*> getPins() const
    {
        std::vector<const Pin*> pins;

        for (auto& propagator : propagators) {
            if (!propagator->isAbstract() && ((Propagator*)propagator.get())->getKind() == Propagator::Kinds::PIN)
                pins.push_back((Pin*)propagator.get());
        }

        return pins;
    }
    

    void addPropagator(std::unique_ptr<AbstractPropagator> propagator) {
        propagators.push_back(std::move(propagator));
    }

    // appearance
    ComponentAppearance& getAppearance() { return appearance; }
    const ComponentAppearance& getAppearance() const { return appearance; }
    void setAppearance(const ComponentAppearance& value) { appearance = value; }

    // graphicsItem
    ComponentGraphicsObject* getGraphicsObject() const { return graphicsObject; }
    void setGraphicsObject(ComponentGraphicsObject* value) { graphicsObject = value; }

    virtual size_t getUint32sToSave() const override;
    virtual void saveToAddress(uint32_t* data) const override;

    virtual void addRelatedPropagator(AbstractPropagator* abstract) override {};
    virtual void forgetPropagator(AbstractPropagator* abstract) override {};


    // void loadFromFile(const std::filesystem::path& path);
    // void saveToFile(const std::filesystem::path& path);

    static Propagator* findPropagatorPointerOfId(uint32_t id, const std::unordered_map<uint32_t, PropagatorIdentity>& map);

    // void debugPrintPropagators() const;
    uint32_t getUint32Size(uint32_t& propagatorId, std::unordered_map<Propagator*, uint32_t>& wiresById,std::unordered_map<Propagator*, uint32_t>& pinsById, std::vector<Component*>& components, bool canRecurse) const ;

    Component();
    Component(const std::string& _name);
    Component(const Component& componentToCopy);
protected:
    std::string name;
    std::filesystem::path filePath;
    std::vector<std::unique_ptr<AbstractPropagator>> propagators;
    ComponentAppearance appearance;
    ComponentGraphicsObject* graphicsObject = nullptr;
    // Position position;

};

class SentinelComponent : public Component {
    private:
    bool resolved = false;
    std::unordered_map<std::string, std::vector<Position>> unresolvedComponentPositions;
    bool evaluateResolved() {resolved = unresolvedComponentPositions.empty(); return resolved;}
public:
    SentinelComponent() {}
    SentinelComponent(const std::string& _name) : Component(_name) {}
    bool isResolved() {evaluateResolved(); return resolved;}
    void setResolved(bool _resolved) {resolved = _resolved;}
    
    bool informAddedComponentToSeeIfFullyResolved(const std::string& name, const Component& _component);

  bool loadFromFile(const std::filesystem::path& path);
  void saveToFile(const std::filesystem::path& path);

  void createComponent(const Position& _position, const Component& _component);
  std::unique_ptr<AbstractPropagator> createDerivativeComponent(const Position& _position) const;
  void simulateConnections();

  Component getDuplicate() const;
};

#endif // COMPONENT_H
