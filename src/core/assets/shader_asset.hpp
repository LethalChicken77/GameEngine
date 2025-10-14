#include "core/asset_data.hpp"

namespace core
{
class ShaderAsset : public AssetData
{

};

class Shader : public Asset<ShaderAsset>
{
    public:
        using Asset::Asset;
};
} // namespace core