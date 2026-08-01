#ifndef COMPONENT_H
#define COMPONENT_H

#include <QObject>
#include <QPointF>
#include <unordered_set>
#include <filesystem>
#include <fstream>

enum States
{
    LOW = 0,
    HIGH = 1,
    FLOATING = 2,
    CONFLICT = 3,
    ERROR = 4
};

class Position
{
public:
    int x = 0;
    int y = 0;

    Position(int _x, int _y) : x(_x), y(_y) {}
    Position() {}

    inline int getX() const {return x;}
    inline int getY() const {return y;}
};

class Component;

class Propagator
{
protected:
    static States evaluateTwoStates(const States& stateA, const States& stateB);
    static uint32_t findIdOfPropagatorPointer(Propagator* propagator, const std::unordered_map<Propagator*, uint32_t>& map);
public:
    enum Kinds {
        WIRE,
        PIN
    };
    int tickPropagationDelay = 1;
    States effectingState = States::FLOATING;
    std::unordered_set<Propagator*> effectors; // those that this propagator effects
    std::unordered_set<Propagator*> affectors; // those that this propagator is affected by
    //TODO SEE DEF FOR NOTE
    // Defaults to effectors = affectors.
    virtual void evaluateEffectingState(); //allow warn for multi low/high, technically not conflict but....
    // Defaults to effectors = affectors.
    // virtual void evaluateEffectors();

    //evaluates effecting state and ques effectors for evaluation.
    void propagate(Propagator* excludedPropagator = nullptr);

    void forget(Propagator* propagator);
    virtual Kinds getKind() const {return Kinds::WIRE;}
    virtual uint32_t getUint32sToSave() const = 0;
    virtual uint32_t getUint32sToSaveEffectorsAndAffectors() const;
    //begins at data.
    virtual void saveToAddress(uint32_t* data, const std::unordered_map<Propagator*, uint32_t>& map) const = 0;
    //begins at data.
    virtual void saveEffectorsAndAffectorsToAddres(uint32_t* data, const std::unordered_map<Propagator*, uint32_t>& map) const;

    virtual void addToGrid() const;
    virtual void removeFromGrid() const;
    virtual void moveAlongGrid() const;

    void copyEffectorsAndEffectors(const Propagator& propagator, std::unordered_map<Propagator*, Propagator*>& oldNewPropagatorPointerMap);

    Propagator() {};
    Propagator(const Propagator& propagator);
    virtual ~Propagator() = default;
};

class Wire : public Propagator
{
public:
    std::vector<Position> anchors;
    virtual uint32_t getUint32sToSave() const override;
    virtual void saveToAddress(uint32_t* data, const std::unordered_map<Propagator*, uint32_t>& map) const override;

    // does not sync effectors and affectors.
    Wire(const Wire& wireToCopy);
    Wire() {}
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

    Operations effectorOperation = Operations::BUFFER;
    Position relPosition;
    Position gridPosition() const;
    QPointF qGridPosition() const;
    static QPointF gridAlignPoint(const QPointF& point);
    std::string name;
    Component& parent;
    // state should be inherited by wire, begins nullptr assuming no connected wire, beware.
    States* state = nullptr; //unused...
    virtual void evaluateEffectingState() override;
    virtual Kinds getKind() const override {return Kinds::PIN;}
    virtual uint32_t getUint32sToSave() const override;
    virtual void saveToAddress(uint32_t* data, const std::unordered_map<Propagator*, uint32_t>& map) const override;
    Pin(Component& _parent, States* _state = nullptr) : parent(_parent), state(_state) {}
    // does not sync effectors and affectors.
    Pin(const Pin& pinToCopy, Component& _parent);
private:

};

class Component : public QObject
{
    Q_OBJECT
private:
    struct PropagatorIdentity {
    public:
        Propagator* propagator = nullptr;
        std::unordered_set<uint32_t> effectorIds;
        std::unordered_set<uint32_t> affectorIds;
        PropagatorIdentity(Propagator* _propagator) : propagator(_propagator) {}
    };
public:
    std::string name;
    std::filesystem::path filePath;
    std::vector<std::unique_ptr<Propagator>> propagators;
    void loadFromFile(const std::filesystem::path& path);
    void saveToFile(const std::filesystem::path& path);

    static Propagator* findPropagatorPointerOfId(uint32_t id, const std::unordered_map<uint32_t, PropagatorIdentity>& map);

    void debugPrintPropagators() const;


    Component();
    Component(const Component& componentToCopy);
};

#endif // COMPONENT_H
