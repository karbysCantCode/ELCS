#include "projectmanager.h"

ProjectManager::ProjectManager() {
    components.try_emplace("AND");
    components["AND"].loadFromFile(std::filesystem::path(RESOURCES_PATH)/"and.csf");
    components["AND"].debugPrintPropagators();
}
