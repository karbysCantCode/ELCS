#include "scheduler.h"

#include <cassert>

Scheduler* globalScheduler = new Scheduler(0);

void Tick::run() {
    for (const auto& ptr : callbacks) {
        ptr();
    }
}
template<typename _typename, typename _typenameIndex>
_typename* LinkedList<_typename, _typenameIndex>::operator[](_typenameIndex index) const {
    Element* top = _front;
    while (top && *top->value < index) {
        top = top->next;
    }
    return top ? (*top->value == index ? top->value : nullptr) : nullptr;
}

template<typename _typename, typename _typenameIndex>
std::pair<bool, _typename*> LinkedList<_typename, _typenameIndex>::emplaceAt(_typenameIndex index) {
    Element* top = _front;
    Element* last = top;
    while (top && *top->value < index) {
        top = top->next;
        last = top;
    }

    //top null at front
    // top null at end
    // top not null  == or !=

    //top null at front
    if (!last) {
        Element* ptr = new Element(new _typename, nullptr);
        _front = ptr;
        return {true, ptr->value};
    }
    // top null at end
    //(!top && last) except these both should be true always when together...
    if (!top) {
        Element* ptr = new Element(new _typename, nullptr);
        last->next = ptr;
        return {true, ptr->value};
    }
    // top not null !=
    if (*top->value != index) {
        Element* ptr = new Element(new _typename, top->next);
        top->next = ptr;
        return {true, ptr->value};
    }
    // top not null  ==
    return {false, top->value};
}
template<typename _typename, typename _typenameIndex>
void LinkedList<_typename, _typenameIndex>::popFront() {
    if (_front) {
        //_typename* ptr = _front->value; freed by destrotor of element front
        delete _front;
        _front = _front->next;
    }
}
template<typename _typename, typename _typenameIndex>
typename LinkedList<_typename, _typenameIndex>::Iterator&
LinkedList<_typename, _typenameIndex>::Iterator::operator++() {
    if (current) {
        current = current->next;
    }
    return *this;
}

template<typename _typename, typename _typenameIndex>
typename LinkedList<_typename, _typenameIndex>::Iterator&
LinkedList<_typename, _typenameIndex>::Iterator::operator++(int) {
    if (current) {
        current = current->next;
    }
    return *this;
}

template<typename _typename, typename _typenameIndex>
bool LinkedList<_typename, _typenameIndex>::Iterator::operator==(void* rhs) {
    if (current) {
        return rhs == current->value;
    } else {
        return rhs == nullptr;
    }
}

template<typename _typename, typename _typenameIndex>
bool LinkedList<_typename, _typenameIndex>::Iterator::operator!=(void* rhs) {
    if (current) {
        return rhs != current->value;
    } else {
        return rhs != nullptr;
    }
}

void Tick::registerCallback(CallbackFunc func) {
    callbacks.push_back(func);
}

void Scheduler::registerCallback(CallbackFunc func, int ticksUntilExecute) {
    std::lock_guard<std::mutex> lock(listMutex);
    auto [inserted, ptr] = ticks.emplaceAt(ticksUntilExecute);
    ptr->registerCallback(func);
    ticksCV.notify_one();
}

void Scheduler::registerCallback(std::vector<Propagator*>& propagators, Propagator* excludedPropagator, int ticksUntilExecute) {
    std::lock_guard<std::mutex> lock(listMutex);
    auto [inserted, ptr] = ticks.emplaceAt(ticksUntilExecute);
    for (auto* propagator : propagators) {
        if (propagator == excludedPropagator)
            continue;
        ptr->registerCallback([propagator]() {
            propagator->propagate();
        });
    }
    ticksCV.notify_one();
}

void Scheduler::registerCallback(std::unordered_set<Propagator*>& propagators, Propagator* excludedPropagator, int ticksUntilExecute) {
    std::lock_guard<std::mutex> lock(listMutex);
    auto [inserted, ptr] = ticks.emplaceAt(ticksUntilExecute);
    for (auto* propagator : propagators) {
        if (propagator == excludedPropagator)
            continue;
        ptr->registerCallback([propagator]() {
            propagator->propagate();
        });
    }
    ticksCV.notify_one();
}

void Scheduler::workerRunTicks(int tickCount) {
    if (tickCount == 0) {
        while (running) {
            std::unique_lock<std::mutex> lock(listMutex);
            ticksCV.wait(lock, [this] { return !running || !ticks.empty(); });
            lock.unlock();

            if (!running)
                break;

            runTick();
        }
    } else {
        while (tickCount > 0 && running) {
            tickCount--;
            runTick();
        }
    }
    running = false;
}

void Scheduler::runTicks(int tickCount) {
    if (worker) {
        stopTicks();
        worker->join();
    }
    running = true;
    worker = std::make_unique<std::thread>(&Scheduler::workerRunTicks, this, tickCount);
}

void Scheduler::stopTicks() {
    running = false;
    ticksCV.notify_all();
}

void Scheduler::runTick() {
    currentTick++;

    auto it = ticks.getIterator();
    while (it != nullptr) {
        std::lock_guard<std::mutex> lock(listMutex);
        if ((*(*it)) == 0) {
            (*(*it)).run();
            it++;
            // assert(ticks.front() == *it); //shouldnt.... TODO DEBUG (can remove production)
            ticks.popFront();
        } else {
            (*it)->ticksTillDeath--;
            it++;
        }
    }
}

void Scheduler::clear() {
  ticks.clear();
}