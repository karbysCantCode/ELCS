#include "projectmanager.h"
#include "notifications.h"
#include "pingraphicsobject.h"
#include "segmentgraphicsobject.h"
#include "componentgraphicsobject.h"
#include "component.h"
#include "componenttoolbox.h"
#include "filehelper.h"
#include "circuitstyleworkspace.h"
#include "coregates.h"

#include <format>

ProjectManager* globalProjectManager = nullptr;
CircuitWorkspace* __circuitworkspace = nullptr;
CircuitStyleWorkspace* __circuitStyleWorkspace = nullptr;
ProjectManager::ProjectManager() {
  
}

void ProjectManager::onComponentEditRequested(const SentinelComponent& component) {
  if (currentOpenComponent &&
      !currentOpenComponent->saveToFile(currentOpenComponent->getFilePath())) {
    globalNotificationManager->notify(
      "Couldn't Save Circuit.",
      std::format("Failed to save circuit \"{}\" before switching to \"{}\".",
                   currentOpenComponent->getName(), component.getName())
    );
  }
  openComponent(component.getName());
}

void ProjectManager::dummyLoad() {
  auto OR = std::make_unique<SentinelComponent>(CoreGates::makeOR());
  auto AND = std::make_unique<SentinelComponent>(CoreGates::makeAND());
  auto XOR = std::make_unique<SentinelComponent>(CoreGates::makeXOR());
  auto NAND = std::make_unique<SentinelComponent>(CoreGates::makeNAND());
  auto NOR = std::make_unique<SentinelComponent>(CoreGates::makeNOR());
  auto XNOR = std::make_unique<SentinelComponent>(CoreGates::makeXNOR());
  auto NOT = std::make_unique<SentinelComponent>(CoreGates::makeNOT());
  auto BUFFER = std::make_unique<SentinelComponent>(CoreGates::makeBUFFER());

  components.emplace(OR->getName(), std::move(OR));
  components.emplace(AND->getName(), std::move(AND));
  components.emplace(XOR->getName(), std::move(XOR));
  components.emplace(NAND->getName(), std::move(NAND));
  components.emplace(NOR->getName(), std::move(NOR));
  components.emplace(XNOR->getName(), std::move(XNOR));
  components.emplace(NOT->getName(), std::move(NOT));
  components.emplace(BUFFER->getName(), std::move(BUFFER));

  auto files = getFilesInDirectory(std::filesystem::path(RESOURCES_PATH));
  for (const auto& file : files) {
    if (file.extension() != SAVE_FILE_EXTENSION) continue;
    auto [component, success] = loadNewComponent(file);
    if (success) {
      openComponent(component->getName());
    }
  }
  updateToolboxes();

  auto it = components.find("AND");
  if (it != components.end() && !it->second->saveToFile(std::filesystem::path(RESOURCES_PATH)/"savedand.csf")) {
    globalNotificationManager->notify("Couldn't Save Circuit.", "Failed to save \"AND\" during dummy load.");
  }
}

bool ProjectManager::createNewComponent(const std::string& name) {
  auto [element, success] =
    components.try_emplace(name,std::make_unique<SentinelComponent>(name));
  updateToolboxes();
  auto path = std::filesystem::path(RESOURCES_PATH)/(name+SAVE_FILE_EXTENSION);
  if (!doesFileExist(path)) {
    createFile(path);
    element->second->setFilePath(path);
    if (!element->second->saveToFile(element->second->getFilePath())) {
      globalNotificationManager->notify(
        "Couldn't Save Circuit.",
        std::format("Failed to save newly created circuit \"{}\".", name)
      );
    }
  }
  return success;
}

void ProjectManager::updateToolboxes() const {
  simulatorCircuitToolbox->updateElements();
  styleCircuitToolbox->updateElements();
}
std::pair<SentinelComponent*, bool> ProjectManager::loadNewComponent(const std::filesystem::path& path) {
  auto component = std::make_unique<SentinelComponent>();
  bool successFileLoad = component->loadFromFile(path);
  if (!successFileLoad) {
    globalNotificationManager->notify("Failed Loading...", std::format("Failed to load component at file \"{}\"", path.string()), 15000);
    return {nullptr, false};
  }
  auto [element,success] = components.emplace(component->getName(), std::move(component));
  updateToolboxes();

  // Any sentinel that had an unresolved reference to this one (by
  // name) by can now be backfilled with a real instance.
  for (auto* sentinel : unresolvedSentinelComponents) {
    sentinel->informAddedComponentToSeeIfFullyResolved(element->second->getName(), *element->second);
  }

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
    if (!currentOpenComponent->saveToFile(currentOpenComponent->getFilePath())) {
      globalNotificationManager->notify(
        "Couldn't Save Circuit.",
        std::format("Failed to save circuit \"{}\".", currentOpenComponent->getName())
      );
    }
  } else {
    globalNotificationManager->notify("Couldn't Save Circuit.", "Couldn't find a currently opened component to save.");
  }
}

AbstractPropagator* ProjectManager::addNewPropagator(std::unique_ptr<AbstractPropagator> propagator) {
    AbstractPropagator* ptr = propagator.get();
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
      globalProjectManager->gridManager.addToGrid(ptr->getGridPosition() + pin->getAppearancePosition(), pin);
    }

    auto* item = new ComponentGraphicsObject(*ptr);
    ptr->setGraphicsObject(item);
    workspace->scene()->addItem(item);
    item->setZValue(1);
    item->setVisible(true);
    item->setOpacity(1.0);
    item->setZValue(100);
    item->setPos(ptr->getGridPosition().getGridScaledCopy().getQPointF());
    item->refresh();

  } else {
    auto ptr = (Propagator*)_ptr;
    if (ptr->getKind() == Propagator::Kinds::PIN) {
        auto np = (Pin*)ptr;
        globalProjectManager->gridManager.addToGrid(np->getGridPosition(), np);
        auto* item = new PinGraphicsObject(*np);
        np->setGraphicsObject(item);
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
        np->setGraphicsObject(np->graphicsObject);
        workspace->scene()->addItem(np->graphicsObject);
        np->graphicsObject->setZValue(1);
    }
  }
}

void ProjectManager::initiateSimulatorUIPropertyManager(PropertySection* _propertySection) {
	simulatorPropertySection = _propertySection;
}