#pragma once
#include <string>
#include <memory>

#include "object.hpp"

namespace core
{
    class Asset : public Object
    {
    public:
        Asset(const Asset&) = delete;
        Asset& operator=(const Asset&) = delete;
        Asset(Asset&&) = delete;
        Asset& operator=(Asset&&) = delete;

        static std::unique_ptr<Asset> Instantiate(std::string name = "New Asset") = delete;
        
        // Allow instantiation of generic assets
        static std::unique_ptr<Asset> Instantiate(id_t uuid, std::string name = "New Asset")
        {
            // TODO: Find file, instantiate
        }
        static std::unique_ptr<Asset> Instantiate(std::string path, std::string name = "New Asset")
        {
            std::unique_ptr<Asset> parent = Object::Instantiate<Asset>(name);
            return std::move(parent); // TODO: Put somewhere
        }

    protected:
        id_t UUID; // Unique file ID
        std::string path; // Located based on file ID
        
        Asset(id_t newID) : Object(newID) {}
        friend std::unique_ptr<Asset> Object::Instantiate<Asset>(std::string);
    };
} // namespace core