#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace core
{
    class Object
    {
        public:
            using id_t = uint64_t;
            Object() = delete;
            Object(const Object&) = delete;
            Object& operator=(const Object&) = delete;
            Object(Object&&) = delete;
            Object& operator=(Object&&) = delete;

            std::string name;
            id_t getInstanceID() const { return instanceID; }
        
            static void Destroy();
        protected:
            Object(id_t id) : instanceID(id) {};
        private:
            id_t instanceID;
            friend class ObjectManager;
    };
}