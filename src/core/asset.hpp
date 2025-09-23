#include <string>

namespace core
{
    class Asset
    {
        public:
            const std::string& getName() const { return name; }
            const std::string& getPath() const { return path; }
        protected:
            std::string name;
            std::string path;
    };
} // namespace core