#ifndef COMPONENT_H
#define COMPONENT_H

#include <QObject>
#include <QDebug>
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
class SentinelComponent;
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

    void copyEffectorsAndEffectors(const Propagator& propagator, std::unordered_map<Propagator*, Propagator*>& oldNewPropagatorPointerMap);

    Propagator() {};
    Propagator(const Propagator& propagator);
    virtual ~Propagator();
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

    // Removes the segment at `index`. Returns true if a segment was
    // removed (false if index was out of range). Caller is
    // responsible for unregistering the removed segment from the
    // grid first (mirrors how the rest of this class leaves grid
    // bookkeeping to the caller, e.g. mergeCollidingWires()).
    bool removeSegmentAt(size_t index);

    // Index of whichever segment sits closest to `point` (grid
    // space), or -1 if this wire has no segments. Used to figure out
    // which segment a user meant when deleting part of a
    // multi-segment wire (segments are axis-aligned, so this is a
    // simple clamped-point distance check).
    int nearestSegmentIndex(const Position& point) const;

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

    // Cleans up this pin's graphics item, mirroring Wire::~Wire() --
    // needed now that pins can be deleted interactively rather than
    // only ever existing for the lifetime of the whole program.
    ~Pin();
private:
    Operations effectorOperation = Operations::BUFFER;
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


    /*
        Adds a propagator into this component. If it's itself a
        nested Component (isAbstract() == true), its parentComponent
        is pointed back at *this* -- the single place that linkage
        gets established, so every insertion path (load, copy
        construction, interactive placement) gets it for free instead
        of every call site having to remember to set it.
    */
    void addPropagator(std::unique_ptr<AbstractPropagator> propagator) {
        if (propagator->isAbstract()) {
            static_cast<Component*>(propagator.get())->parentComponent = this;
        }
        propagators.push_back(std::move(propagator));
    }

    // Removes (and destroys) a single propagator directly owned by
    // this component, if present. Returns true if something was
    // removed.
    bool removePropagator(AbstractPropagator* propagator);

    /*
        Appearance.

        For a definition (a SentinelComponent, or any Component with
        no source sentinel) this is the component's own shape data.

        For a placed *instance* of a sentinel (sourceSentinel !=
        nullptr) this transparently forwards to the source sentinel's
        appearance, so edits made in the style editor show up on
        every placed copy immediately -- no explicit push/notify step
        needed for appearance changes.

        Pin *positions* are deliberately NOT part of this -- each
        instance keeps its own Pin objects (Pin::appearancePosition),
        kept in sync with the sentinel's template pins via
        resyncFromSentinel() whenever the sentinel's pin layout
        changes, so that per-instance wiring/state (effecting color)
        always reflects *this* instance rather than the template.
    */
    ComponentAppearance& getAppearance();
    const ComponentAppearance& getAppearance() const;

    // No-op (with a debug warning) on a placed instance -- an
    // instance's appearance isn't independently settable, it mirrors
    // its source sentinel. Only meaningful on a definition.
    void setAppearance(const ComponentAppearance& value);

    // graphicsItem
    ComponentGraphicsObject* getGraphicsObject() const { return graphicsObject; }
    void setGraphicsObject(ComponentGraphicsObject* value) { graphicsObject = value; }

    // The component this one is nested inside of (whichever
    // component's propagators list owns this one), or nullptr if
    // this component isn't currently placed inside anything. Set
    // automatically by addPropagator().
    Component* getParentComponent() const { return parentComponent; }

    // The sentinel this component was instantiated from, if any.
    // Null for a SentinelComponent itself (definitions aren't
    // instances of anything) and for any standalone Component that
    // wasn't created via SentinelComponent::createDerivativeComponent()
    // / createComponent().
    SentinelComponent* getSourceSentinel() const { return sourceSentinel; }

    // Called by a SentinelComponent's destructor on any instance
    // still pointing at it, so a dangling instance never dereferences
    // a dead sentinel afterwards.
    void clearSourceSentinel() { sourceSentinel = nullptr; }

    // Re-syncs this instance's pins against its source sentinel's
    // current template pins: adds pins the template has gained,
    // removes ones it's lost, and mirrors operation/appearance
    // position/grid position for ones that still match by name --
    // while preserving that pin's own wiring/effecting state. No-op
    // if this component has no source sentinel. Bubbles a
    // structure-changed notification up through parentComponent
    // afterwards so whatever circuit this instance lives in can
    // re-derive its connections.
    void resyncFromSentinel();

    // Bubbles a "my structure changed" notification up the
    // parentComponent chain, calling reevaluateConnections() at every
    // level on the way up.
    void notifyChildStructureChanged();

    // Overridden by SentinelComponent to actually re-derive
    // connectivity (via simulateConnections()). No-op by default,
    // since a plain nested Component doesn't own an independent
    // simulation pass -- only the sentinel at the root of a
    // parentComponent chain does.
    virtual void reevaluateConnections() {}

    virtual size_t getUint32sToSave() const override;
    virtual void saveToAddress(uint32_t* data) const override;

    virtual void addRelatedPropagator(AbstractPropagator* abstract) override {};
    virtual void forgetPropagator(AbstractPropagator* abstract) override {};

    static Propagator* findPropagatorPointerOfId(uint32_t id, const std::unordered_map<uint32_t, PropagatorIdentity>& map);

    uint32_t getUint32Size(uint32_t& propagatorId, std::unordered_map<Propagator*, uint32_t>& wiresById,std::unordered_map<Propagator*, uint32_t>& pinsById, std::vector<Component*>& components, bool canRecurse) const ;

    Component();
    Component(const std::string& _name);
    Component(const Component& componentToCopy);
    virtual ~Component();

protected:
    std::string name;
    std::filesystem::path filePath;
    std::vector<std::unique_ptr<AbstractPropagator>> propagators;
    ComponentAppearance appearance;
    ComponentGraphicsObject* graphicsObject = nullptr;

    Component* parentComponent = nullptr;
    SentinelComponent* sourceSentinel = nullptr;

    // SentinelComponent needs to reach into arbitrary Component
    // instances' protected sourceSentinel/parentComponent fields
    // (not just its own base subobject) when wiring up newly-created
    // instances -- plain protected access doesn't allow that across
    // sibling objects, hence the friendship.
    friend class SentinelComponent;
};

class SentinelComponent : public Component {
private:
    bool resolved = false;
    std::unordered_map<std::string, std::vector<Position>> unresolvedComponentPositions;
    bool evaluateResolved() {resolved = unresolvedComponentPositions.empty(); return resolved;}

    /*
        Every currently-live Component placed elsewhere that was
        derived from this sentinel (non-owning -- lifetime is owned
        by whatever circuit each instance is placed in). Used to push
        structural updates (pins added/removed/renamed) out to every
        placed copy, and to poke every placed copy's graphics item to
        repaint after an appearance edit.

        Mutable so registration can happen through a const
        SentinelComponent& (creating an instance doesn't logically
        change the sentinel's own definition, just this bookkeeping).
    */
    mutable std::unordered_set<Component*> instances;

public:
    SentinelComponent() {}
    SentinelComponent(const std::string& _name) : Component(_name) {}
    ~SentinelComponent() override;

    bool isResolved() {evaluateResolved(); return resolved;}
    void setResolved(bool _resolved) {resolved = _resolved;}

    bool informAddedComponentToSeeIfFullyResolved(const std::string& name, const SentinelComponent& _component);

    bool loadFromFile(const std::filesystem::path& path);
    bool saveToFile(const std::filesystem::path& path) const;

    void createComponent(const Position& _position, const SentinelComponent& _component);
    std::unique_ptr<AbstractPropagator> createDerivativeComponent(const Position& _position) const;
    void simulateConnections();

    void reevaluateConnections() override { simulateConnections(); }

    Component getDuplicate() const;


    // Instance bookkeeping -- called automatically by
    // createDerivativeComponent()/createComponent(), and by
    // Component's destructor on teardown. Exposed publicly in case
    // other code paths end up creating/destroying instances directly.
    void registerInstance(Component* instance) const { instances.emplace(instance); }
    void unregisterInstance(Component* instance) const { instances.erase(instance); }

    // Pushes a pin-layout resync out to every placed instance. Call
    // this after changing this sentinel's own pins (add/remove/
    // rename/move/re-operation) so every existing copy picks it up.
    // Not needed for pure appearance edits (lines/curves/labels/
    // anchor) -- those are read live via Component::getAppearance().
    void notifyInstancesOfStructureChange();

    // Cheap visual poke for every placed instance's graphics item.
    // Call this after an appearance edit (e.g. from
    // CircuitStyleWorkspace) so on-screen copies repaint immediately
    // instead of waiting for their next unrelated refresh.
    void refreshInstanceGraphics();
};

#endif // COMPONENT_H