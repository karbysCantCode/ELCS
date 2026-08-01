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
  workspace->scene()->clear();
}

void ProjectManager::addCurrentComponentToWorkspace() {
  for (auto& propagator : currentOpenComponent->propagators) {
    if (propagator->getKind() == Propagator::Kinds::PIN) {
      auto pin = (Pin*)propagator.get();
      PinGraphicsItem* item = new PinGraphicsItem(*pin);
      item->setZValue(1);
      item->setPos(pin->qGridPosition());
      workspace->scene()->addItem(item);
      
    }
  }
}

void ProjectManager::saveCurrentComponent() {
  if (currentOpenComponent) {
    currentOpenComponent->saveToFile(currentOpenComponent->filePath);
  } else {
    globalNotificationManager->notify("Couldn't Save Circuit.", "Couldn't find a currently opened component to save.");
  }
}