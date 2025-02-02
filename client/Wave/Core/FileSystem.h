#pragma once

#include <filesystem>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace core
{
    using Path = std::filesystem::path;

    class FileSystem
    {
    public:
        /// @brief Wrapper to retrieve current root directory
        /// @hack CMake is currently configured to drop the solution deep in the 'build' directory 
        static inline Path GetRootDirectory() 
        { 
#if defined(WIN32)
            char buffer[MAX_PATH];
            GetModuleFileNameA(NULL, buffer, MAX_PATH);
            return std::filesystem::path(buffer).parent_path();
#elif defined(__linux__)
            return std::filesystem::canonical("/proc/self/exe").parent_path();
#endif
        }

        /// @brief Wrapper to retrieve path to the shader directory
        static inline Path GetShaderDirectory() { return GetRootDirectory() / "Shaders"; }

        /// @brief Wrapper to retrieve a shader in the shader directory
        /// @param filename Filename of the shader to retrieve
        /// @return Standard path object to the retrieved file
        static inline Path GetShaderPath(const char* filename) { return GetShaderDirectory() / filename; }

        /// @brief Wrapper to retrieve path to the assets directory
        static inline Path GetAssetsDirectory() { return GetRootDirectory() / "Assets"; }

        /// @brief Wrapper to retrieve an asset in the assets directory
        /// @param filename Filename of the asset to retrieve
        /// @return Standard path object to the retrieved file
        static inline Path GetAssetPath(const char* filename) { return GetAssetsDirectory() / filename; }

        /// @brief Wrapper to retrieve path to the surf scripts directory
        static inline Path GetSurfScriptsDirectory() { return GetRootDirectory() / "surf"; }

        /// @brief Wrapper to retrieve a script in the surf directory
        /// @param filename Filename of the surf script to retrieve
        /// @return Standard path object to the retrieved file
        static inline Path GetSurfScriptPath(const char* filename) { return GetSurfScriptsDirectory() / filename; }
    };
}