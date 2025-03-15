#include "material.hpp"
#include "containers.hpp"

namespace graphics
{
    // Align a given offset to the specified alignment
    size_t alignTo(size_t offset, size_t alignment)
    {
        // std::cout << "Offset: " <<  offset << std::endl;
        // std::cout << "Alignment: " <<  alignment << std::endl;
        return (offset + alignment - 1) & ~(alignment - 1);
    }

    // Determine the size and alignment for a given ShaderInput::DataType
    struct TypeInfo
    {
        size_t size;
        size_t alignment;
    };

    TypeInfo getTypeInfo(ShaderInput::DataType type)
    {
        switch (type)
        {
        case ShaderInput::DataType::FLOAT: return {sizeof(float), 4};
        case ShaderInput::DataType::VEC2:  return {sizeof(glm::vec2), 8};
        case ShaderInput::DataType::VEC3:  return {sizeof(glm::vec3), 16}; // Vec3 uses Vec4 alignment
        case ShaderInput::DataType::VEC4:  return {sizeof(glm::vec4), 16};
        case ShaderInput::DataType::MAT2:  return {sizeof(glm::mat2), 16}; // Matrices align to vec4
        case ShaderInput::DataType::MAT3:  return {sizeof(glm::mat3), 16}; // Matrices align to vec4
        case ShaderInput::DataType::MAT4:  return {sizeof(glm::mat4), 16};
        case ShaderInput::DataType::INT:   return {sizeof(int), 4};
        case ShaderInput::DataType::BOOL:  return {sizeof(int), 4}; // bools are treated as 4 bytes in std140
        default: throw std::runtime_error("Unknown ShaderInput::DataType");
        }
    }

    Material::Material(id_t mat_id, const Shader *_shader) : id(mat_id), shader(_shader)
    {
        const std::vector<ShaderInput> &inputs = shader->getInputs();
        inputValues.resize(inputs.size());
        
    }

    void Material::createShaderInputBuffer()
    {
        const std::vector<ShaderInput> &shaderInputs = shader->getInputs();
        assert(inputValues.size() == shader->getInputs().size() && "Input values size must match shader input size");

        size_t offset = 0;
        size_t bufferSize = 0;
        for (size_t i = 0; i < shaderInputs.size(); ++i)
        {
            const ShaderInput& input = shaderInputs[i];
            const Value& value = inputValues[i];

            // Get type info for alignment and size
            TypeInfo typeInfo = getTypeInfo(input.type);

            // Align the offset
            offset = alignTo(offset, typeInfo.alignment);

            // Resize the buffer to accommodate the new data
            bufferSize += typeInfo.size;
        }
        data.resize(bufferSize);
        offset = 0;
        for (size_t i = 0; i < shaderInputs.size(); ++i)
        {
            const ShaderInput& input = shaderInputs[i];
            const Value& value = inputValues[i];

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

        if(!initialized)
        {
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
            
            VkDescriptorBufferInfo bufferInfo = buffer->descriptorInfo();
            DescriptorWriter(*Descriptors::materialSetLayout, *Descriptors::materialPool)
            .writeBuffer(0, &bufferInfo)
                .build(descriptorSet);
            initialized = true;
        }
        
        buffer->writeToBuffer(data.data());
    }

    void Material::setValue(std::string name, ShaderResource::Value value)
    {
        const std::vector<ShaderInput> &shaderInputs = shader->getInputs();
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
} // namespace graphics