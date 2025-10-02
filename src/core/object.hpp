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
            Object(Object&&) = default;
            Object& operator=(Object&&) = default;

            std::string name;
        
            template<class T>
            static std::unique_ptr<T> Instantiate()
            {
                static id_t next_id = 0;
                return std::make_unique<Object>(next_id++);
            }
            static void Destroy();
        private:
            Object(id_t id) : instanceID(id) {};
            id_t instanceID;
    };
}