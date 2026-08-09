#include "projectmanager.h"
#include "notifications.h"
#include "pingraphicsobject.h"
#include "segmentgraphicsobject.h"
#include "componentgraphicsobject.h"
#include "component.h"
#include "componenttoolbox.h"
#include "filehelper.h"

#include <format>

ProjectManager* globalProjectManager = nullptr;
CircuitWorkspace* __circuitworkspace = nullptr;

ProjectManager::ProjectManager() {
  
}

void ProjectManager::onComponentEditRequested(const SentinelComponent& component) {
  currentOpenComponent->saveToFile(currentOpenComponent->getFilePath());
  openComponent(component.getName());
}

void ProjectManager::dummyLoad() {
  auto files = getFilesInDirectory(std::filesystem::path(RESOURCES_PATH));
  for (const auto& file : files) {
    if (file.extension() != SAVE_FILE_EXTENSION) continue;
    auto [component, success] = loadNewComponent(file);
    if (success) {
      openComponent(component->getName());
    }
  }
  simulatorCircuitToolbox->updateElements();
  components["AND"]->saveToFile(std::filesystem::path(RESOURCES_PATH)/"savedand.csf");

}

bool ProjectManager::createNewComponent(const std::string& name) {
  auto [element, success] =
    components.try_emplace(name,std::make_unique<SentinelComponent>(name));
  simulatorCircuitToolbox->updateElements();
  qDebug("ee");
  auto path = std::filesystem::path(RESOURCES_PATH)/(name+SAVE_FILE_EXTENSION);
  if (!doesFileExist(path)) {
    qDebug("ee2");
    createFile(path);
    element->second->setFilePath(path);
    element->second->saveToFile(element->second->getFilePath());
  }
  return success;
}
std::pair<SentinelComponent*, bool> ProjectManager::loadNewComponent(const std::filesystem::path& path) {
  auto component = std::make_unique<SentinelComponent>();
  component->loadFromFile(path);
  auto [element,success] = components.emplace(component->getName(), std::move(component));
  simulatorCircuitToolbox->updateElements();
  return {element->second.get(), success};
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
  currentOpenComponent = it->second.get();
  // other processes to update...
  removeExistingComponentFromWorkspace();
  addCurrentComponentToWorkspace();
}

void ProjectManager::removeExistingComponentFromWorkspace() {
  gridManager.reset();
  workspace->reset();
}

void ProjectManager::addCurrentComponentToWorkspace() {
   std::vector<AbstractPropagator*> rawVec;

   for (const auto& ptr : currentOpenComponent->getPropagators())
   {
    	rawVec.push_back(ptr.get());
   }
  for (auto& propagator : rawVec) {
    visuallyRegisterPropagator(propagator);
  }
}

void ProjectManager::saveCurrentComponent() {
  if (currentOpenComponent) {
    currentOpenComponent->saveToFile(currentOpenComponent->getFilePath());
  } else {
    globalNotificationManager->notify("Couldn't Save Circuit.", "Couldn't find a currently opened component to save.");
  }
}

AbstractPropagator* ProjectManager::addNewPropagator(std::unique_ptr<AbstractPropagator> propagator) {
    auto ptr = propagator.get();
    currentOpenComponent->addPropagator(std::move(propagator));
    visuallyRegisterPropagator(ptr);
    return ptr;
}

//returns true if it needs to be destroyed
void ProjectManager::visuallyRegisterPropagator(AbstractPropagator* _ptr) {
  if (_ptr->isAbstract()) {
    auto ptr = (Component*)_ptr;
    auto pins = ptr->getPins();
    for (auto* pin : pins) {
      globalProjectManager->gridManager.addToGrid(pin->gridPosition(), pin);
    }

  } else {
    auto ptr = (Propagator*)_ptr;
    if (ptr->getKind() == Propagator::Kinds::PIN) {
        auto np = (Pin*)ptr;
        globalProjectManager->gridManager.addToGrid(np->getGridPosition(), np);
        auto* item = new PinGraphicsObject(*np);
        workspace->scene()->addItem(item);
        item->setZValue(1);
        item->setPos(np->getGridPosition().getGridScaledCopy().getQPointF());
    } else if (ptr->getKind() == Propagator::Kinds::WIRE) {
        auto np = (Wire*)ptr;
        auto touchingElements = globalProjectManager->gridManager.addToGrid(np->segments, np);
        
        std::unordered_set<Wire *> tempDeathReg;
				std::unordered_set<Propagator *> excludeSet;
        np->mergeCollidingWires(tempDeathReg, excludeSet, &touchingElements);
        
        np->graphicsObject = new SegmentGraphicsObject(*np);
        workspace->scene()->addItem(np->graphicsObject);
        np->graphicsObject->setZValue(1);

				// int i = 0;
				// for (const auto& propagator : currentOpenComponent->propagators) {
				// 	if (propagator->getKind() == Propagator::Kinds::WIRE) i++;
				// }
				// qDebug() << "wire count =" << i;
    }
  }
}

void ProjectManager::initiateSimulatorUIPropertyManager(PropertySection* _propertySection) {
	simulatorPropertySection = _propertySection;
}