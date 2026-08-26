cmake_minimum_required(VERSION 3.16)
project(AvatarVirtual LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ---------------------------------------------------------------------------
# Dependencias del sistema (instaladas vía apt, ver README.md)
# En Ubuntu, SDL2/SDL2_image/PortAudio NO exponen paquetes cmake en modo
# CONFIG -- exponen archivos .pc de pkg-config. Por eso usamos PkgConfig
# en vez de find_package(... CONFIG REQUIRED), que sólo funciona con vcpkg.
# ---------------------------------------------------------------------------
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)
pkg_check_modules(SDL2_IMAGE REQUIRED IMPORTED_TARGET SDL2_image)
pkg_check_modules(PORTAUDIO REQUIRED IMPORTED_TARGET portaudio-2.0)

# nlohmann-json3-dev sí instala un config cmake real, esto funciona tal cual.
find_package(nlohmann_json REQUIRED)

# ---------------------------------------------------------------------------
# whisper.cpp no tiene paquete apt: se compila como subproyecto a partir
# de un git submodule (ver README.md -> "git submodule update --init").
# ---------------------------------------------------------------------------
set(WHISPER_DIR ${CMAKE_SOURCE_DIR}/external/whisper.cpp)
if(NOT EXISTS ${WHISPER_DIR}/CMakeLists.txt)
    message(FATAL_ERROR
        "No se encontró whisper.cpp en ${WHISPER_DIR}.\n"
        "Ejecutá: git submodule update --init --recursive\n"
        "(ver README.md, sección 'Dependencias incluidas / externas')")
endif()
set(WHISPER_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(WHISPER_BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(${WHISPER_DIR} whisper_build)

# ---------------------------------------------------------------------------
# Librería con la lógica pura del proyecto (sin SDL2/PortAudio/whisper), para
# poder linkearla tanto en avatar_app como en el binario de tests unitarios
# sin arrastrar dependencias de hardware.
# ---------------------------------------------------------------------------
add_library(avatar_core STATIC
    src/util/paths.cpp
    src/dialog/dialog.cpp
)

target_include_directories(avatar_core PUBLIC src)
target_link_libraries(avatar_core PUBLIC nlohmann_json::nlohmann_json)

# ---------------------------------------------------------------------------
# Ejecutable principal
# ---------------------------------------------------------------------------
add_executable(avatar_app
    src/main.cpp
    src/audio/audio.cpp
    src/avatar/avatar.cpp
    src/draw/draw.cpp
)

target_include_directories(avatar_app PRIVATE src)

target_link_libraries(avatar_app PRIVATE
    avatar_core
    PkgConfig::SDL2
    PkgConfig::SDL2_IMAGE
    PkgConfig::PORTAUDIO
    whisper
)

# Ruta de recursos usada como último fallback si no hay resources/ junto al
# binario ni la variable de entorno AVATAR_RESOURCES_DIR (ver src/util/paths.*)
target_compile_definitions(avatar_app PRIVATE
    RESOURCES_DIR="${CMAKE_INSTALL_PREFIX}/share/avatar/resources"
)

# Para desarrollo local: copia resources/ junto al binario recién compilado,
# así "./bin/avatar_app" funciona sin necesidad de instalar nada.
add_custom_command(TARGET avatar_app POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/resources
        $<TARGET_FILE_DIR:avatar_app>/resources
    COMMENT "Copiando resources/ junto al ejecutable para desarrollo local"
)

# ---------------------------------------------------------------------------
# Instalación (opcional): deja el proyecto portable a nivel de sistema
# ---------------------------------------------------------------------------
install(TARGETS avatar_app RUNTIME DESTINATION bin)
install(DIRECTORY resources/ DESTINATION share/avatar/resources)

# ---------------------------------------------------------------------------
# Tests unitarios (opcional, ver README.md -> "Tests")
# Off por defecto para no romper el build nativo default ni la imagen Docker,
# que no tienen el paquete apt "catch2" instalado.
# ---------------------------------------------------------------------------
option(AVATAR_BUILD_TESTS "Compilar tests unitarios (requiere apt install catch2)" OFF)
if(AVATAR_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
