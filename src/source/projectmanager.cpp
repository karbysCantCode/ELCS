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
  for (auto& propagator : currentOpenComponent->propagators) {
    visuallyRegisterPropagator(propagator.get());
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
    visuallyRegisterPropagator(ptr);
    return ptr;
}

void ProjectManager::visuallyRegisterPropagator(Propagator* ptr) {
    if (ptr->getKind() == Propagator::Kinds::PIN) {
        auto np = (Pin*)ptr;
        globalProjectManager->gridManager.addToGrid(np->relPosition, np);
        auto* item = new PinGraphicsItem(*np);
        workspace->scene()->addItem(item);
        item->setZValue(1);
        item->setPos(np->relPosition.getGridScaledCopy().getQPointF());
    } else if (ptr->getKind() == Propagator::Kinds::WIRE) {
        auto np = (Wire*)ptr;
        auto touchingElements = globalProjectManager->gridManager.addToGrid(np->anchors, np);
        auto* item = new WireGraphicsItem(*np);
        workspace->scene()->addItem(item);
        item->setZValue(1);

        np->trimForCollidingWires(touchingElements);
    }
}