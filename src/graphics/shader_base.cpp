#include "shader_base.hpp"
#include "containers.hpp"

namespace graphics
{
    void ShaderBase::createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule)
    {
        Console::log("Creating shader module", "ShaderBase");
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        assert(code.size() % 4 == 0 && "Shader code size must be a multiple of 4");
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        if(vkCreateShaderModule(Shared::device->device(), &createInfo, nullptr, shaderModule) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create shader module!");
        }
    }

    
    std::vector<uint32_t> ShaderBase::SlangToSpirv(
        const std::vector<char>& shaderData,
        const char* moduleName,
        const char* entryPointName,
        SlangStage slangStage)
    {
        std::string source(shaderData.begin(), shaderData.end());

        Slang::ComPtr<slang::IGlobalSession> globalSession;
        SlangGlobalSessionDesc globalDesc = {};
        if (SLANG_FAILED(createGlobalSession(&globalDesc, globalSession.writeRef())))
            throw std::runtime_error("Slang: failed to create global session");

        slang::SessionDesc sessionDesc = {};
        // const char* paths[] = { "C:/VulkanSDK/1.4.309.0/Include/slang" }; // adjust as needed
        const char* paths[] = { "./internal/shaders", "./assets/shaders" }; // Set search paths
        sessionDesc.searchPaths = paths;
        sessionDesc.searchPathCount = 2;

        Slang::ComPtr<slang::ISession> session;
        if (SLANG_FAILED(globalSession->createSession(sessionDesc, session.writeRef())))
            throw std::runtime_error("Slang: failed to create session");

        Slang::ComPtr<slang::ICompileRequest> request;
        if (SLANG_FAILED(session->createCompileRequest(request.writeRef())))
            throw std::runtime_error("Slang: failed to create compile request");

        request->addCodeGenTarget(SLANG_SPIRV);

        SlangProfileID spirvProfile = globalSession->findProfile("sm_6_8");
        if (!spirvProfile)
            throw std::runtime_error("Slang: sm_6_8 profile not available");
        request->setTargetProfile(0, spirvProfile);

        int tuIndex = request->addTranslationUnit(SLANG_SOURCE_LANGUAGE_SLANG, moduleName);
        if (tuIndex < 0) throw std::runtime_error("Slang: addTranslationUnit failed");
        request->addTranslationUnitSourceString(tuIndex, moduleName, source.c_str());

        int entryPointIndex = request->addEntryPoint(tuIndex, entryPointName, slangStage);
        if (entryPointIndex < 0) throw std::runtime_error("Slang: addEntryPoint failed");

        if (SLANG_FAILED(request->compile()))
        {
            const char* diag = request->getDiagnosticOutput();
            // getDiagnosticOutput may provide (const char**, size_t*) or a single-string API depending on build
            if(diag != nullptr)
                Console::error("Shader compile failed: " + std::string(diag), "Slang");
            else
                Console::error("Shader compile failed (no info)", "Slang");
            return {};
        }

        slang::IBlob* rawBlob = nullptr;
        if (SLANG_FAILED(request->getEntryPointCodeBlob(entryPointIndex, 0, &rawBlob)))
            throw std::runtime_error("Slang: failed to get SPIR-V code blob");

        Slang::ComPtr<slang::IBlob> spirvBlob;
        spirvBlob.attach(rawBlob); // take ownership

        const void* data = spirvBlob->getBufferPointer();
        size_t sizeBytes = spirvBlob->getBufferSize();
        if (!data || sizeBytes == 0)
            throw std::runtime_error("Slang: empty SPIR-V output");

        if (sizeBytes % sizeof(uint32_t) != 0)
            throw std::runtime_error("Slang: SPIR-V size not a multiple of 4");

        const uint32_t* words = reinterpret_cast<const uint32_t*>(data);
        size_t wordCount = sizeBytes / sizeof(uint32_t);
        return std::vector<uint32_t>(words, words + wordCount);
    }
} // namespace graphics