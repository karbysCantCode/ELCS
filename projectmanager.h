#ifndef PROJECTMANAGER_H
#define PROJECTMANAGER_H

#include <string>
#include <unordered_map>

#include "component.h"

class ProjectManager
{
public:
    std::unordered_map<std::string, Component> components;

    ProjectManager();
};

static ProjectManager globalProjectManager;

#endif // PROJECTMANAGER_H
