#include <catch2/catch_test_macros.hpp>
#include "util/paths.h"

#include <cstdlib>
#include <string>

namespace {

// Guarda y restaura AVATAR_RESOURCES_DIR para no ensuciar el proceso entre
// test cases (paths.cpp guarda el resultado en una variable estática global).
struct EnvGuard {
    bool teniaValor;
    std::string valorAnterior;

    EnvGuard() {
        const char* v = std::getenv("AVATAR_RESOURCES_DIR");
        teniaValor = v != nullptr;
        if (teniaValor) valorAnterior = v;
    }

    ~EnvGuard() {
        if (teniaValor) {
            setenv("AVATAR_RESOURCES_DIR", valorAnterior.c_str(), 1);
        } else {
            unsetenv("AVATAR_RESOURCES_DIR");
        }
    }
};

} // namespace

TEST_CASE("resourcePath usa AVATAR_RESOURCES_DIR cuando esta definida", "[paths]") {
    EnvGuard guard;
    setenv("AVATAR_RESOURCES_DIR", "/tmp/mis-recursos", 1);

    paths_init("/cualquier/ruta/avatar_app");

    REQUIRE(resourcePath("models/ggml-base.bin") == "/tmp/mis-recursos/models/ggml-base.bin");
}

TEST_CASE("resourcePath cae al directorio del ejecutable sin la env var", "[paths]") {
    EnvGuard guard;
    unsetenv("AVATAR_RESOURCES_DIR");

    // En Linux, paths_init() resuelve el directorio real vía /proc/self/exe
    // (el binario de test), ignorando este argv0 falso -- por eso el test
    // sólo valida el sufijo relativo, no el prefijo absoluto.
    paths_init("/opt/avatar/bin/avatar_app");

    std::string ruta = resourcePath("dialog/preguntas.json");
    std::string sufijoEsperado = "/resources/dialog/preguntas.json";
    bool terminaConSufijo =
        ruta.size() >= sufijoEsperado.size() &&
        ruta.compare(ruta.size() - sufijoEsperado.size(), sufijoEsperado.size(), sufijoEsperado) == 0;
    REQUIRE(terminaConSufijo);
}
