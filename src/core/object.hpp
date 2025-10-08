#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include "engine_types.hpp"

namespace core
{
    class Object
    {
        public:
            static constexpr const char* className = "Object";
            virtual const char* GetClassName() const { return className; }
            
            Object() = delete;
            Object(const Object&) = delete;
            Object& operator=(const Object&) = delete;
            Object(Object&&) = delete;
            Object& operator=(Object&&) = delete;
            
            virtual ~Object() = default;
            

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