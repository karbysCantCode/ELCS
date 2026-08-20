#ifndef PROJECTMANAGER_H
#define PROJECTMANAGER_H

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include "component.h"
#include "circuitworkspace.h"
#include "componentholder.h"
#include "propertysection.h"
#include "circuitstyleworkspace.h"
#include "tutorialmanager.h"

class CircuitStyleWorkspace;
class ProjectManager
{
public:
    std::unordered_map<std::string, std::unique_ptr<SentinelComponent>> components;
    SentinelComponent* currentOpenComponent = nullptr;
    CircuitWorkspace* workspace = nullptr;
    CircuitStyleWorkspace* styleWorkspace = nullptr;
    ComponentHolder gridManager;
    std::unordered_set<SentinelComponent*> unresolvedSentinelComponents;
    TutorialManager tutorialManager;

    void dummyLoad();
    bool createNewComponent(const std::string& name);
    std::pair<SentinelComponent*, bool> loadNewComponent(const std::filesystem::path& path);
    void registerCallbackOnNewComponent(std::function<void()>* func);
    void removeCallbackOnNewComponent(std::function<void()>* func);
    void openComponent(const std::string& name);
    void saveCurrentComponent();
    void initiateSimulatorUIPropertyManager(PropertySection* _propertySection);
    AbstractPropagator* addNewPropagator(std::unique_ptr<AbstractPropagator> propagator);
    ProjectManager();

    void updateToolboxes() const;

    void onComponentEditRequested(const SentinelComponent& component);

    void onPropagatorSelected(AbstractPropagator* propagator);
    void onSelectionCleared();

private:
  void removeExistingComponentFromWorkspace();
  void addCurrentComponentToWorkspace();
  void visuallyRegisterPropagator(AbstractPropagator* _ptr);
  void populatePinProperties(Pin* pin);
  void populateComponentProperties(Component* component);
  void populateWireProperties(Wire* wire);
  std::unordered_set<std::function<void()>*> newComponentCallbacks;

  PropertySection* simulatorPropertySection = nullptr;
};

extern ProjectManager* globalProjectManager;
extern CircuitWorkspace* __circuitworkspace;
extern CircuitStyleWorkspace* __circuitStyleWorkspace;
extern TutorialOverlay* __tutorialOverlay;
extern TutorialToolbox* __tutorialToobox;
#endif // PROJECTMANAGER_H