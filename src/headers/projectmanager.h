#ifndef PROJECTMANAGER_H
#define PROJECTMANAGER_H

#include <string>
#include <unordered_map>
#include <functional>
#include "component.h"
#include "circuitworkspace.h"
#include "componentholder.h"

class ProjectManager
{
private:
  std::unordered_set<std::function<void()>*> newComponentCallbacks;
public:
    std::unordered_map<std::string, Component> components;
    Component* currentOpenComponent = nullptr;
    CircuitWorkspace* workspace = nullptr;
    ComponentHolder gridManager;

    void dummyLoad();
    bool createNewComponent(const std::string& name);
    bool loadNewComponent(const std::filesystem::path& path);
    void registerCallbackOnNewComponent(std::function<void()>* func);
    void removeCallbackOnNewComponent(std::function<void()>* func);
    void openComponent(const std::string& name);
    void saveCurrentComponent();
    Propagator* addNewPropagator(std::unique_ptr<Propagator> propagator);
    ProjectManager();

private:
  void removeExistingComponentFromWorkspace();
  void addCurrentComponentToWorkspace();
  bool visuallyRegisterPropagator(Propagator* ptr);
};

extern ProjectManager* globalProjectManager;
extern CircuitWorkspace* __circuitworkspace;
#endif // PROJECTMANAGER_H
