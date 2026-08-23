#pragma once
#include <string>

// Debe llamarse una única vez al inicio de main(argv[0]) antes de usar
// resourcePath(). Determina el directorio base de recursos con esta
// prioridad:
//   1) Variable de entorno AVATAR_RESOURCES_DIR (si está definida)
//   2) ./resources junto al ejecutable (útil para desarrollo local,
//      el build ya copia resources/ ahí como POST_BUILD)
//   3) RESOURCES_DIR definido en tiempo de compilación (ruta de instalación,
//      por ejemplo /usr/local/share/avatar/resources)
void paths_init(const char* argv0);

// Devuelve la ruta absoluta a un recurso dado su path relativo dentro de
// resources/, por ejemplo: resourcePath("models/ggml-base.bin")
std::string resourcePath(const std::string& relative);
