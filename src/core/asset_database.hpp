#include <string>
#include <unordered_map>
#include <../graphics/texture.hpp>
#include <../graphics/mesh.hpp>

namespace core
{
    class AssetDatabase
    {
    public:
        AssetDatabase() = default;
        ~AssetDatabase() = default;
        AssetDatabase(const AssetDatabase&) = delete;
        AssetDatabase& operator=(const AssetDatabase&) = delete;

        void addAsset(const std::string& name, const std::string& path);
        void removeAsset(const std::string& name);
        const std::string& getAssetPath(const std::string& name) const;
        bool assetExists(const std::string& name) const;

    private:
        
        std::unordered_map<std::string, std::string> assets;
    };
}