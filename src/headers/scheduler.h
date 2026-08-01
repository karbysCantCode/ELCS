#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <vector>
#include <thread>
#include <utility>
#include <mutex>

#include "component.h"

class Propagator;

using CallbackFunc = std::function<void()>;

struct Tick
{
    int ticksTillDeath = 0;
    std::vector<CallbackFunc> callbacks;
    void run();
    void registerCallback(CallbackFunc func);

    Tick() {}
    Tick(int _ticksTillDeath) : ticksTillDeath(_ticksTillDeath) {}

    bool operator<(const int rhs) const {return ticksTillDeath < rhs;}
    bool operator!=(const int rhs) const {return ticksTillDeath != rhs;}
    bool operator==(const int rhs) const {return ticksTillDeath == rhs;}
};
// requrires _typename implements > and == operators for _typenameIndex.
template<typename _typename, typename _typenameIndex>
struct LinkedList
{
private:
    struct Element {
        _typename* value = nullptr;
        Element* next = nullptr;

        Element(_typename* _value, Element* _next) : next(_next), value(_value) {}
        ~Element() {delete value; if (next) delete next;}
    };
    Element* _front = nullptr;

    struct Iterator {
    public:
        Element* current = nullptr;

        Iterator& operator++();
        Iterator& operator++(int);
        bool operator==(void* rhs);
        bool operator!=(void* rhs);
        _typename* operator*() { return current ? current->value : nullptr; }

        Iterator(LinkedList* list) : current(list->_front) {}
    };
public:
    _typename* front() const {return _front ? _front->value : nullptr;}
    _typename* operator[](_typenameIndex index) const;
    std::pair<bool, _typename*> emplaceAt(_typenameIndex index);
    void popFront();
    Iterator getIterator() {return Iterator(this);}

    ~LinkedList() {if (_front) {delete _front; _front = nullptr;}}
    void clear() {if (_front) {delete _front; _front = nullptr;}}
};

class Scheduler
{
private:
    int nsPerTick = 0;
    long long currentTick = 0;
    bool running = false;
    std::unique_ptr<std::thread> worker;
    std::mutex listMutex;

    void runTick();
    void workerRunTicks(int tickCount = 0);
public:
    Scheduler(int _nsPerTick) : nsPerTick(_nsPerTick) {}


    LinkedList<Tick, int> ticks;
    void registerCallback(CallbackFunc func, int ticksUntilExecute = 1);
    void registerCallback(std::vector<Propagator*>& propagators, Propagator* excludedPropagator, int ticksUntilExecute = 1);
    void registerCallback(std::unordered_set<Propagator*>& propagators, Propagator* excludedPropagator = nullptr, int ticksUntilExecute = 1);
    void runTicks(int tickCount = 0);
    void stopTicks();
    void clear();
};

//static defined in scheduler.h
extern Scheduler* globalScheduler;

#endif // SCHEDULER_H
