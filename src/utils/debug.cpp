#include "debug.hpp"
#include <iostream>
#include <sstream>
#include <string.h>

namespace debug
{
    void log(const std::string& message)
    {
        std::cout << message << std::endl;
    }

    void logerr(const std::string& message)
    {
        std::cerr << message << std::endl;
    }

    std::string vec2ToString(const glm::vec2& v)
    {
        std::ostringstream oss;
        oss << "vec2(" << v.x << ", " << v.y << ")";
        return oss.str();
    }

    std::string vec3ToString(const glm::vec3& v)
    {
        std::ostringstream oss;
        oss << "vec3(" << v.x << ", " << v.y << ", " << v.z << ")";
        return oss.str();
    }

    std::string vec4ToString(const glm::vec4& v)
    {
        std::ostringstream oss;
        oss << "vec4(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
        return oss.str();
    }

    std::string mat2ToString(const glm::mat2& m)
    {
        std::ostringstream oss;
        oss << "mat2(\n";
        for (int i = 0; i < 2; ++i)
        {
            for (int j = 0; j < 2; ++j)
            {
                oss << m[i][j];
                if (i != 1 || j != 1)
                    oss << ", \t";
            }
            oss << "\n";
        }
        oss << ")\n";
        return oss.str();
    }

    std::string mat3ToString(const glm::mat3& m)
    {
        std::ostringstream oss;
        oss << "mat3(\n";
        for (int i = 0; i < 3; ++i)
        {
            for (int j = 0; j < 3; ++j)
            {
                oss << m[i][j];
                if (i != 1 || j != 1)
                    oss << ", \t";
            }
            oss << "\n";
        }
        oss << ")\n";
        return oss.str();
    }

    std::string mat4ToString(const glm::mat4& m)
    {
        std::ostringstream oss;
        oss << "mat4(\n";
        for (int i = 0; i < 4; ++i)
        {
            for (int j = 0; j < 4; ++j)
            {
                oss << m[i][j];
                if (i != 1 || j != 1)
                    oss << ", \t";
            }
            oss << "\n";
        }
        oss << ")\n";
        return oss.str();
    }
}