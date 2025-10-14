#pragma once
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <memory>
#include "object.hpp"
#include "engine_types.hpp"
#include <imgui.h>

namespace core
{
    class AssetData; // Needed to ensure assets are not directly instantiated
    class AssetManager;
    class ObjectManager
    {
        public:
            static Object *getObject(id_t objectID);
            // static Object *getObject(std::string objectName);

            template<class T>
            static T *Instantiate(const std::string& name = "New Object")
            {
                static_assert(!std::is_base_of<AssetData, T>::value, "Assets must be instantiated via AssetManager");
                return InstantiateInternal<T>(name);
            }

            static bool deleteObject(id_t objID); // TODO: Implement

            static void drawImGui();
        private:
            template<class T>
            static T *InstantiateInternal(const std::string& name = "New Object")
            {
                static_assert(std::is_base_of<Object, T>::value, "Instantiated objects must derive from Object");
                std::unique_ptr<T> t = std::unique_ptr<T>(new T(currentID++));
                objectList.push_back(std::unique_ptr<Object>(std::move(t)));
                T* objPtr = static_cast<T*>(objectList.back().get()); // New object is always at the end of the vector
                objPtr->name = name;
                objectIDDictionary[objPtr->getInstanceID()] = objPtr;
                return objPtr;
            }
            static id_t currentID;
            static std::unordered_map<id_t, Object*> objectIDDictionary; // Easy accessing via index
            // std::unordered_map<std::string, Object*> objectNameDictionary; // Easy accessing via index // TODO: Implement later to account for renaming
            static std::vector<std::unique_ptr<Object>> objectList; // Where objects live in memory

            friend class AssetManager;
    };
} // namespace core