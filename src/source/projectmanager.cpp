#include "projectmanager.h"
#include "notifications.h"
#include "pingraphicsitem.h"

#include <format>

ProjectManager* globalProjectManager = nullptr;
 CircuitWorkspace* __circuitworkspace = nullptr;

ProjectManager::ProjectManager() {
  
}

void ProjectManager::dummyLoad() {
  loadNewComponent(std::filesystem::path(RESOURCES_PATH)/"and.csf");
  openComponent("AND");
  components["AND"].debugPrintPropagators();
  components["AND"].saveToFile(std::filesystem::path(RESOURCES_PATH)/"savedand.csf");
  
}

bool ProjectManager::createNewComponent(const std::string& name) {
  auto [element,success] = components.try_emplace(name);
  return success;
}
bool ProjectManager::loadNewComponent(const std::filesystem::path& path) {
  Component component;
  component.loadFromFile(path);
  auto [element,success] = components.emplace(component.name, std::move(component));
  return success;
}

void ProjectManager::registerCallbackOnNewComponent(std::function<void()>* func) {
  newComponentCallbacks.emplace(func);
}

void ProjectManager::removeCallbackOnNewComponent(std::function<void()>* func) {
  newComponentCallbacks.erase(func);
}

void ProjectManager::openComponent(const std::string& name) {
  auto it = components.find(name);
  if (it == components.end()) {
    globalNotificationManager->notify("Error Opening Circuit", std::format("Couldn't find circuit \"{}\" in the current project.", name));
    return;
  }
  currentOpenComponent = &it->second;
  // other processes to update...
  removeExistingComponentFromWorkspace();
  addCurrentComponentToWorkspace();
}

void ProjectManager::removeExistingComponentFromWorkspace() {
  gridManager.reset();
  workspace->reset();
}

void ProjectManager::addCurrentComponentToWorkspace() {
   std::vector<Propagator*> rawVec;

   for (const auto& ptr : currentOpenComponent->propagators)
   {
    	rawVec.push_back(ptr.get());
   }
  for (auto& propagator : rawVec) {
    visuallyRegisterPropagator(propagator);
  }
}

void ProjectManager::saveCurrentComponent() {
  if (currentOpenComponent) {
    currentOpenComponent->saveToFile(currentOpenComponent->filePath);
  } else {
    globalNotificationManager->notify("Couldn't Save Circuit.", "Couldn't find a currently opened component to save.");
  }
}

Propagator* ProjectManager::addNewPropagator(std::unique_ptr<Propagator> propagator) {
    auto ptr = propagator.get();
    currentOpenComponent->propagators.push_back(std::move(propagator));

    if (visuallyRegisterPropagator(ptr)) {
        // FIX: visuallyRegisterPropagator() can recurse back into
        // addNewPropagator() (e.g. Wire::trimForCollidingWires splitting a
        // wire into fragments and registering each one). Those recursive
        // calls push additional entries onto `propagators` *after* ours,
        // so by the time we get here `ptr` is no longer guaranteed to be
        // at the back — pop_back() would silently delete the wrong
        // (possibly perfectly valid) propagator instead of this one.
        //
        // Find and erase the specific entry that owns `ptr` instead.
        auto& propagators = currentOpenComponent->propagators;
        auto it = std::find_if(
            propagators.begin(),
            propagators.end(),
            [ptr](const std::unique_ptr<Propagator>& p) { return p.get() == ptr; }
        );

        if (it != propagators.end()) {
            propagators.erase(it);
        }

        // NOTE: `ptr` is dangling after this point, same as it already was
        // in the original code's pop_back() path. Callers that discard the
        // return value (as trimForCollidingWires and the wiring code do)
        // are fine; anything that uses the return value when registration
        // fails needs to check for that case first.
        return nullptr;
    }

    return ptr;
}

//returns true if it needs to be destroyed
bool ProjectManager::visuallyRegisterPropagator(Propagator* ptr) {
    if (ptr->getKind() == Propagator::Kinds::PIN) {
        auto np = (Pin*)ptr;
        globalProjectManager->gridManager.addToGrid(np->relPosition, np);
        auto* item = new PinGraphicsItem(*np);
        workspace->scene()->addItem(item);
        item->setZValue(1);
        item->setPos(np->relPosition.getGridScaledCopy().getQPointF());
    } else if (ptr->getKind() == Propagator::Kinds::WIRE) {
        auto np = (Wire*)ptr;
        auto touchingElements = globalProjectManager->gridManager.addToGrid(np->segments, np);
        
        std::unordered_set<Wire *> tempDeathReg;
				std::unordered_set<Propagator *> excludeSet;
        np->mergeCollidingWires(tempDeathReg, excludeSet, &touchingElements);
        
        np->graphicsItem = new SegmentGraphicsItem(*np);
        workspace->scene()->addItem(np->graphicsItem);
        np->graphicsItem->setZValue(1);

				int i = 0;
				for (const auto& propagator : currentOpenComponent->propagators) {
					if (propagator->getKind() == Propagator::Kinds::WIRE) i++;
				}
				qDebug() << "wire count =" << i;
    }
    return false;
}