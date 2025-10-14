#pragma once
#include <string>
#include <memory>

#include "object.hpp"
#include "utils/smart_reference.hpp"
#include "asset_manager.hpp"

namespace core
{
    template<class T>
    class Asset : public SmartRef<T>
    {
        protected:
            using SmartRef<T>::ptr;
        public:
            using SmartRef<T>::SmartRef;
            Asset(const std::string &path)
            {
                ptr = AssetManager::Instantiate<T>(path);
            }
    };
} // namespace core