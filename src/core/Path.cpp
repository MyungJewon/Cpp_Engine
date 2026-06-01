#include "core/Path.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <mach-o/dyld.h>
#include <libgen.h>
#include <limits.h>
#include <vector>
#endif

namespace {
bool IsAbsolutePath(const std::string& path) {
    if (path.empty()) return false;
#ifdef _WIN32
    return path[0] == '/' || path[0] == '\\' || (path.size() > 1 && path[1] == ':');
#else
    return path[0] == '/';
#endif
}
}

std::string Path::GetExecutableDir() {
#ifdef _WIN32
    char buffer[MAX_PATH];
    DWORD length = GetModuleFileNameA(nullptr, buffer, MAX_PATH);
    std::string path(buffer, length);
    const size_t slash = path.find_last_of("\\/");
    return slash == std::string::npos ? std::string(".") : path.substr(0, slash);
#else
    char buffer[PATH_MAX];
    uint32_t size = sizeof(buffer);
    if (_NSGetExecutablePath(buffer, &size) != 0) {
        std::vector<char> dynamicBuffer(size);
        _NSGetExecutablePath(dynamicBuffer.data(), &size);
        return std::string(dirname(dynamicBuffer.data()));
    }

    // dirname은 입력 버퍼를 수정하므로 지역 버퍼를 그대로 넘긴다.
    return std::string(dirname(buffer));
#endif
}

std::string Path::Resolve(const std::string& path) {
    if (path.empty() || IsAbsolutePath(path)) return path;

    const std::string base = GetExecutableDir();
#ifdef _WIN32
    const char separator = '\\';
#else
    const char separator = '/';
#endif
    if (!base.empty() && (base.back() == '/' || base.back() == '\\')) return base + path;
    return base + separator + path;
}
