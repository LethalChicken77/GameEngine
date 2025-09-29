#include "material.hpp"
#include "containers.hpp"

namespace graphics
{
    Material::Material(id_t mat_id, const Shader *_shader) : id(mat_id), shader(_shader)
    {
        shaderInputs = shader->getInputs();
        inputValues.resize(shaderInputs.size());
        // createShaderInputBuffer();
    }

    void Material::createShaderInputBuffer()
    {
        // std::cout << "Creating shader input buffer for material " << id << std::endl;
        shaderInputs = shader->getInputs();
        inputValues.resize(shaderInputs.size());
        assert(inputValues.size() == shader->getInputs().size() && "Input values size must match shader input size");

        size_t offset = 0;
        size_t bufferSize = 0;
        for (size_t i = 0; i < shaderInputs.size(); ++i)
        {
            const ShaderInput& input = shaderInputs[i];
            const MaterialValue& value = inputValues[i];

            // Get type info for alignment and size
            // std::cout << "Input " << i << ": " << input.name << " Type: " << static_cast<int>(input.type) << std::endl;
            TypeInfo typeInfo = getTypeInfo(input.type);

            // Align the offset
            offset = alignTo(offset, typeInfo.alignment);

            // Resize the buffer to accommodate the new data
            bufferSize += typeInfo.alignment;
        }
        data.resize(bufferSize);
        offset = 0;
        for (size_t i = 0; i < shaderInputs.size(); ++i)
        {
            const ShaderInput& input = shaderInputs[i];
            const MaterialValue& value = inputValues[i];

            // Get type info for alignment and size
            TypeInfo typeInfo = getTypeInfo(input.type);

            // Align the offset
            offset = alignTo(offset, typeInfo.alignment);

            // Resize the buffer to accommodate the new data
            // data.resize(offset + typeInfo.size);

            // Write the value into the buffer
            std::visit([&](auto&& val) {
                using T = std::decay_t<decltype(val)>;

                // Ensure the type matches
                switch (input.type)
                {
                case ShaderInput::DataType::FLOAT:
                    if constexpr (std::is_same_v<T, float>) memcpy(&data[offset], &val, sizeof(float));
                    else throw std::runtime_error("Type mismatch for FLOAT");
                    break;

                case ShaderInput::DataType::VEC2:
                    if constexpr (std::is_same_v<T, glm::vec2>) memcpy(&data[offset], &val, sizeof(glm::vec2));
                    else throw std::runtime_error("Type mismatch for VEC2");
                    break;

                case ShaderInput::DataType::VEC3:
                    if constexpr (std::is_same_v<T, glm::vec3>)
                    {
                        glm::vec4 vec4Value(val, 0.0f); // Promote to vec4 for alignment
                        memcpy(&data[offset], &vec4Value, sizeof(glm::vec4));
                    }
                    else throw std::runtime_error("Type mismatch for VEC3");
                    break;

                case ShaderInput::DataType::VEC4:
                    if constexpr (std::is_same_v<T, glm::vec4>) memcpy(&data[offset], &val, sizeof(glm::vec4));
                    else throw std::runtime_error("Type mismatch for VEC4");
                    break;

                case ShaderInput::DataType::COLOR:
                    if constexpr (std::is_same_v<T, Color>) memcpy(&data[offset], &val, sizeof(Color));
                    else throw std::runtime_error("Type mismatch for COLOR");
                    break;

                case ShaderInput::DataType::MAT2:
                    if constexpr (std::is_same_v<T, glm::mat2>) memcpy(&data[offset], &val, sizeof(glm::mat2));
                    else throw std::runtime_error("Type mismatch for MAT2");
                    break;

                case ShaderInput::DataType::MAT3:
                    if constexpr (std::is_same_v<T, glm::mat3>)
                    {
                        glm::mat4 mat4Value(1.0f); // Promote to mat4 for alignment
                        memcpy(&mat4Value, &val, sizeof(glm::mat3));
                        memcpy(&data[offset], &mat4Value, sizeof(glm::mat4));
                    }
                    else throw std::runtime_error("Type mismatch for MAT3");
                    break;

                case ShaderInput::DataType::MAT4:
                    if constexpr (std::is_same_v<T, glm::mat4>) memcpy(&data[offset], &val, sizeof(glm::mat4));
                    else throw std::runtime_error("Type mismatch for MAT4");
                    break;

                case ShaderInput::DataType::INT:
                    if constexpr (std::is_same_v<T, int>) memcpy(&data[offset], &val, sizeof(int));
                    else throw std::runtime_error("Type mismatch for INT");
                    break;

                case ShaderInput::DataType::BOOL:
                    if constexpr (std::is_same_v<T, bool>)
                    {
                        int intValue = val ? 1 : 0; // Convert bool to 4-byte int
                        memcpy(&data[offset], &intValue, sizeof(int));
                    }
                    else throw std::runtime_error("Type mismatch for BOOL");
                    break;

                default:
                    throw std::runtime_error("Unknown ShaderInput::DataType");
                }
            }, value);

            // Update the offset
            offset += typeInfo.size;
        }
        // if(!initialized)
        // {
            // Initialize buffers
            buffer = std::make_unique<Buffer>(
                *Shared::device,
                data.size(), // Use data.size() instead of sizeof(data)
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                Shared::device->properties.limits.minUniformBufferOffsetAlignment
            );
            buffer->map();
        // }
        
        // updateDescriptorSet();
        buffer->writeToBuffer(data.data());
        initialized = true;
    }

    void Material::setValue(std::string name, MaterialValue value)
    {
        for (size_t i = 0; i < shaderInputs.size(); ++i)
        {
            if (shaderInputs[i].name == name)
            {
                inputValues[i] = value;
                return;
            }
        }
        throw std::runtime_error("Shader input not found");
    }

    template <class T>
    T Material::getValue(std::string name)
    {
        for (size_t i = 0; i < shaderInputs.size(); ++i)
        {
            if (shaderInputs[i].name == name)
            {
                if(shaderInputs[i].type == ShaderInput::getTypeID<T>())
                {
                    return std::get<T>(inputValues[i]);
                }
                else
                {
                    std::cerr << "Incorrect Datatype" << std::endl;
                    return T{};
                }
            }
        }
        return T{};
    }

    void Material::setTexture(uint32_t binding, std::shared_ptr<Texture> texture) 
    {
        if(!initialized)
        {
            createShaderInputBuffer();
        }
        if (binding >= textures.size()) 
        {
            textures.resize(binding + 1); // ???
        }
        textures[binding] = texture;
        // updateDescriptorSet();
    }

    void Material::createDescriptorSet()
    {
        std::vector<VkDescriptorSet> descriptorSets = {descriptorSet};
        if (descriptorSet != VK_NULL_HANDLE)
        {
            shader->getDescriptorPool()->freeDescriptors(descriptorSets);
            descriptorSet = VK_NULL_HANDLE;
        }
        VkDescriptorBufferInfo bufferInfo = buffer->descriptorInfo();

        DescriptorWriter writer = DescriptorWriter(*(shader->getDescriptorSetLayout()), *(shader->getDescriptorPool()));
        writer.writeBuffer(0, &bufferInfo);
        for(int i = 0; i < textures.size(); i++)
        {
            // std::cout << "Writing image to binding " << i + 1 << std::endl;
            writer.writeImage(i + 1, textures[i]->getDescriptorInfo());
        }
        writer.build(descriptorSet);
    }

    void Material::updateValues()
    {
        vkDeviceWaitIdle(Shared::device->device());
        createShaderInputBuffer();
        createDescriptorSet(); // TODO: Update individual buffers
    }

    void Material::drawImGui()
    {
        // glm::vec3 color = getVec3("color");
        // float roughness = getFloat("roughness");
        // float metallic = getFloat("metallic");
        // bool changed = ImGui::ColorPicker3("Albedo", &color.r);
        // changed |= ImGui::SliderFloat("Roughness", &roughness, 0.0f, 1.0f);
        // changed |= ImGui::SliderFloat("Metallic", &metallic, 0.0f, 1.0f);
        ImGui::PushID(this);
        if(ImGui::CollapsingHeader(("Material " + std::to_string(id)).c_str()))
        {
            bool changed = false;
            for(const ShaderInput &input : shaderInputs)
            {
                switch(input.type)
                {
                    case ShaderInput::DataType::FLOAT:
                        {float fValue = getValue<float>(input.name.c_str());
                        changed |= ImGui::DragFloat(input.name.c_str(), &fValue, 0.01f);
                        if(changed) setValue(input.name.c_str(), fValue);}
                        break;
                    case ShaderInput::DataType::VEC2:
                        {glm::vec2 v2Value = getValue<glm::vec2>(input.name.c_str());
                        changed |= ImGui::DragFloat2(input.name.c_str(), &v2Value.x, 0.01f);
                        if(changed) setValue(input.name.c_str(), v2Value);}
                        break;
                    case ShaderInput::DataType::VEC3:
                        {glm::vec3 v3Value = getValue<glm::vec3>(input.name.c_str());
                        changed |= ImGui::DragFloat3(input.name.c_str(), &v3Value.x, 0.01f);
                        if(changed) setValue(input.name.c_str(), v3Value);}
                        break;
                    case ShaderInput::DataType::VEC4:
                        {glm::vec4 v4Value = getValue<glm::vec4>(input.name.c_str());
                        changed |= ImGui::DragFloat4(input.name.c_str(), &v4Value.x, 0.01f);
                        if(changed) setValue(input.name.c_str(), v4Value);}
                        break;
                    case ShaderInput::DataType::COLOR:
                        {Color colorValue = getValue<Color>(input.name.c_str());
                        float color[4] = { colorValue.r, colorValue.g, colorValue.b, colorValue.a };
                        changed |= ImGui::ColorEdit4(input.name.c_str(), color);
                        if(changed) setValue(input.name.c_str(), Color(color[0], color[1], color[2], color[3]));}
                        break;
                    case ShaderInput::DataType::MAT2:
                        // Not implemented
                        break;
                    case ShaderInput::DataType::MAT3:
                        // Not implemented
                        break;
                    case ShaderInput::DataType::MAT4:
                        // Not implemented
                        break;
                    case ShaderInput::DataType::INT:
                        {int iValue = getValue<int>(input.name.c_str());
                        changed |= ImGui::DragInt(input.name.c_str(), &iValue, 1.0f);
                        if(changed) setValue(input.name.c_str(), iValue);}
                        break;
                    case ShaderInput::DataType::BOOL:
                        {bool bValue = getValue<bool>(input.name.c_str());
                        changed |= ImGui::Checkbox(input.name.c_str(), &bValue);
                        if(changed) setValue(input.name.c_str(), bValue);}
                        break;
                    default:
                        break;
                }
            }
            if(changed)
            {
                updateValues();
            }
        }
        ImGui::PopID();
    }
} // namespace graphics