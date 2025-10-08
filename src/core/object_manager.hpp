#pragma once
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <memory>
#include "object.hpp"

namespace core
{
    class ObjectManager
    {
        public:
            using id_t = uint64_t; // Move to types header

            static Object *getObject(id_t objectID);
            // static Object *getObject(std::string objectName);

            template<class T>
            static T *Instantiate(const std::string& name = "New Object")
            {
                static_assert(std::is_base_of<Object, T>::value, "Instantiated objects must derive from Object");
                std::unique_ptr<T> t = std::make_unique<T>(currentID++);
                objectList.push_back(std::unique_ptr<Object>(std::move(t)));
                T* objPtr = static_cast<std::unique_ptr<T>>(objectList.back()).get(); // New object is always at the end of the vector
                objPtr->name = name;
                objectIDDictionary[objPtr->getInstanceID()] = objPtr;
                return objPtr;
            }

            static bool deleteObject(id_t objID); // TODO: Implement
        private:
            static id_t currentID;
            static std::unordered_map<id_t, Object*> objectIDDictionary; // Easy accessing via index
            // std::unordered_map<std::string, Object*> objectNameDictionary; // Easy accessing via index // TODO: Implement later to account for renaming
            static std::vector<std::unique_ptr<Object>> objectList; // Where objects live in memory
    };
} // namespace core