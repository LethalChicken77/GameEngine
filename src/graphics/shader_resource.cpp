#include "shader_resource.hpp"
#include "containers.hpp"

namespace graphics
{
    // // Align a given offset to the specified alignment
    // size_t ShaderResource::alignTo(size_t offset, size_t alignment)
    // {
    //     // std::cout << "Offset: " <<  offset << std::endl;
    //     // std::cout << "Alignment: " <<  alignment << std::endl;
    //     return (offset + alignment - 1) & ~(alignment - 1);
    // }

    // TypeInfo ShaderResource::getTypeInfo(ShaderInput::DataType type)
    // {
    //     switch (type)
    //     {
    //     case ShaderInput::DataType::FLOAT: return {sizeof(float), 4};
    //     case ShaderInput::DataType::VEC2:  return {sizeof(glm::vec2), 8};
    //     case ShaderInput::DataType::VEC3:  return {sizeof(glm::vec3), 16}; // Vec3 uses Vec4 alignment
    //     case ShaderInput::DataType::VEC4:  return {sizeof(glm::vec4), 16};
    //     case ShaderInput::DataType::MAT2:  return {sizeof(glm::mat2), 16}; // Matrices align to vec4
    //     case ShaderInput::DataType::MAT3:  return {sizeof(glm::mat3), 16}; // Matrices align to vec4
    //     case ShaderInput::DataType::MAT4:  return {sizeof(glm::mat4), 16};
    //     case ShaderInput::DataType::INT:   return {sizeof(int), 4};
    //     case ShaderInput::DataType::BOOL:  return {sizeof(int), 4}; // bools are treated as 4 bytes in std140
    //     default: throw std::runtime_error("Unknown ShaderInput::DataType");
    //     }
    // }
} // namespace graphics