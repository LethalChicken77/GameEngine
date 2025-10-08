#include "object_manager.hpp"

namespace core
{
ObjectManager::id_t ObjectManager::currentID = 0;
std::unordered_map<ObjectManager::id_t, Object*> objectIDDictionary{};
std::vector<std::unique_ptr<Object>> objectList{};

} // namespace core