#include "object_manager.hpp"

namespace core
{
id_t ObjectManager::currentID = 0;
std::unordered_map<id_t, Object*> ObjectManager::objectIDDictionary{};
std::vector<std::unique_ptr<Object>> ObjectManager::objectList{};

void ObjectManager::drawImGui()
{
    ImGui::Begin("Internal Objects");
    for(const std::unique_ptr<Object>& obj : objectList)
    {
        std::string className = obj->GetClassName();
        std::string str = std::to_string(obj->instanceID) + " [" + className + "] " + obj->name;
        ImGui::TextUnformatted(str.c_str());
    }
    ImGui::End();
}
} // namespace core