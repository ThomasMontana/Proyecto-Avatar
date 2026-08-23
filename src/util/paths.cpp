#include "paths.h"

#include <cstdlib>
#include <climits>
#include <string>

#if defined(__linux__)
#include <unistd.h>
#endif

#ifndef RESOURCES_DIR
#define RESOURCES_DIR ""
#endif

static std::string g_baseResourcesDir;

// Devuelve el directorio donde vive el ejecutable en Linux (/proc/self/exe).
// argv0Fallback se usa como último recurso si /proc/self/exe no existe
// (por ejemplo en otros sistemas operativos).
static std::string directorioDelEjecutable(const char* argv0Fallback) {
#if defined(__linux__)
    char buffer[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (len != -1) {
        buffer[len] = '\0';
        std::string ruta(buffer);
        size_t pos = ruta.find_last_of('/');
        if (pos != std::string::npos) {
            return ruta.substr(0, pos);
        }
    }
#endif
    // Fallback genérico: usar el directorio de argv[0]
    if (argv0Fallback) {
        std::string ruta(argv0Fallback);
        size_t pos = ruta.find_last_of("/\\");
        if (pos != std::string::npos) {
            return ruta.substr(0, pos);
        }
    }
    return ".";
}

void paths_init(const char* argv0) {
    // 1) Variable de entorno explícita: máxima prioridad, útil para
    //    contenedores, paquetes .deb, AppImage, etc.
    const char* env = std::getenv("AVATAR_RESOURCES_DIR");
    if (env != nullptr && env[0] != '\0') {
        g_baseResourcesDir = env;
        return;
    }

    // 2) resources/ junto al binario (modo desarrollo / build local)
    std::string dirEjecutable = directorioDelEjecutable(argv0);
    g_baseResourcesDir = dirEjecutable + "/resources";
}

std::string resourcePath(const std::string& relative) {
    if (!g_baseResourcesDir.empty()) {
        return g_baseResourcesDir + "/" + relative;
    }
    // 3) Último recurso: ruta de instalación fijada en tiempo de compilación
    return std::string(RESOURCES_DIR) + "/" + relative;
}
