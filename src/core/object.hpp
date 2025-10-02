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
        
            template<class T>
            static std::unique_ptr<T> Instantiate(std::string name = "New Object")
            {
                static id_t next_id = 0;
                return std::unique_ptr<T>(new T(next_id++));
            }
            static void Destroy();
        protected:
            id_t instanceID;
            static id_t getNextID()
            {
                static id_t next_id = 0;
                return next_id++;
            }
            Object(id_t id) : instanceID(id) {};
        private:
    };
}